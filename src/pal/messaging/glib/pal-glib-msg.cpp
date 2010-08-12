/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * pal-glib-msg.cpp: giochannel + unix domain socket based local messaging
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include "config.h"

#include "pal-glib-msg.h"

#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/fcntl.h>

#include <errno.h>

#include <unistd.h>
#include <glib.h>

#include "deployment.h"
#include "runtime.h"

#define LOCAL_MESSAGING_DIRECTORY_NAME "lm"

#define COND_HAS(condition,f) (((condition) & (f)) == (f))

#define IOCHANNEL_READY_FOR_READ(condition) (COND_HAS(condition,G_IO_IN) || COND_HAS(condition,G_IO_PRI))
#define IOCHANNEL_READY_FOR_WRITE(condition) (COND_HAS(condition,G_IO_OUT))
#define IOCHANNEL_ERROR(condition) (COND_HAS(condition,G_IO_ERR) || COND_HAS(condition,G_IO_NVAL) || COND_HAS(condition,G_IO_HUP))

using namespace Moonlight;

static char*
create_listener_path (const char *domain,
		      const char *receiverName)
{
	char *dir = g_build_filename (g_get_user_data_dir (), "moonlight", LOCAL_MESSAGING_DIRECTORY_NAME, domain, NULL);

	if (-1 == g_mkdir_with_parents (dir, 0777))
		perror ("g_mkdir_with_parents");
	
	char *path = g_build_filename (dir, receiverName, NULL);

	g_free (dir);

	return path;
}


class StateMachine;

class StateMachine {
public:
	char* GetError () { return error; }

protected:
	StateMachine (GFunc finishedCallback,
		      GFunc errorCallback,
		      gpointer callbackData)

	{
		error = NULL;
		read_offset = 0;
		write_offset = 0;
		this->finishedCallback = finishedCallback;
		this->errorCallback = errorCallback;
		this->callbackData = callbackData;
	}

	virtual ~StateMachine ()
	{
		g_free (error);
	}

	GFunc finishedCallback;
	GFunc errorCallback;
	gpointer callbackData;

	int read_offset;
	int write_offset;

	char *error;
	
	typedef gboolean (StateMachine::*SuccessFunc) (void);

	void SetError (char *error)
	{
		if (this->error)
			g_free (this->error);
		this->error = error;
	}

	gboolean do_read (GIOCondition condition, int fd,
			  char *buffer, int buffer_length,
			  StateMachine::SuccessFunc success)
	{
		if (IOCHANNEL_ERROR (condition)) {
			SetError (g_strdup_printf (COND_HAS (condition, G_IO_HUP) ? "hang up while reading, state = %d" :
						   COND_HAS (condition, G_IO_NVAL) ? "invalid fd while reading, state = %d" :
						   /*COND_HAS (condition, G_IO_ERR) */ "poll errorw while reading, state = %d", current_state));
			current_state = READ_ERROR;

			errorCallback (this, callbackData);

			// remove the callback
			return FALSE;
		}
		else /*if (IOCHANNEL_READY_FOR_READ (condition))*/ {
			if (buffer_length == 0)
				return (this->*success)();

			int num_read = read (fd, buffer + read_offset, buffer_length - read_offset);
			if (num_read == -1) {
				current_state = READ_ERROR;
				SetError (g_strdup (strerror (errno)));

				errorCallback (this, callbackData);

				return FALSE;
			}
			else if (num_read + read_offset < buffer_length) {
				read_offset += num_read;
				// more to read
				return TRUE;
			}
			else if (num_read + read_offset == buffer_length) {
				return (this->*success) ();
			}
			else {
				current_state = READ_ERROR;
				SetError (g_strdup_printf ("error doing partial read, read returned > %d bytes: %d",
							   buffer_length - read_offset, num_read));

				errorCallback (this, callbackData);

				return FALSE;
			}
		}
	}

	gboolean do_write (GIOCondition condition, int fd,
			   char *buffer, int buffer_length,
			   StateMachine::SuccessFunc success)
	{
		if (IOCHANNEL_ERROR (condition)) {
			current_state = READ_ERROR;
			SetError (g_strdup ("GIOChannel error"));

			errorCallback (this, callbackData);

			return FALSE;
		}
		else if (IOCHANNEL_READY_FOR_WRITE (condition)) {
			if (buffer_length == 0)
				return (this->*success)();
				
			int num_written = write (fd, buffer + write_offset, buffer_length - write_offset);
			if (num_written == -1) {
				current_state = WRITE_ERROR;
				SetError (g_strdup (COND_HAS (condition, G_IO_HUP) ? "hang up while writing" :
						    COND_HAS (condition, G_IO_NVAL) ? "invalid fd while writing" :
						    /*COND_HAS (condition, G_IO_ERR) */ "poll error while writing"));

				errorCallback (this, callbackData);

				return FALSE;
			}
			else if (num_written + write_offset < buffer_length) {
				write_offset += num_written;
				// more to write, somehow we got less than sizeof (int) bytes in that write...
				return TRUE;
			}
			else if (num_written + write_offset == buffer_length) {
				return (this->*success)();
			}
			else {
				current_state = WRITE_ERROR;
				SetError (g_strdup_printf ("error, write returned > %d bytes: %d",
							   buffer_length - write_offset, num_written));
				errorCallback (this, callbackData);
				return FALSE;
			}
		}
		
		return FALSE;
	}

	int current_state;

	enum {
		INITIAL,
		READ_ERROR,
		WRITE_ERROR,
		FINISHED,
		NUM_STATES
	};
};

class ReceiverMachine : public StateMachine {
public:
	ReceiverMachine (int fd,
			 MessageReceivedCallback messageReceivedCallback,
			 gpointer messageReceivedCallbackData,
			 GFunc finishedCallback,
			 GFunc errorCallback,
			 gpointer callbackData)
		: StateMachine (finishedCallback, errorCallback, callbackData)
	{
		deployment = Deployment::GetCurrent ();

		current_state = READING_HEADER;

		this->fd = fd;
		this->messageReceivedCallback = messageReceivedCallback;
		this->messageReceivedCallbackData = messageReceivedCallbackData;

		message_length = 0;
		message_contents = NULL;

		response_length = 0;
		response_contents = NULL;

		iochannel = g_io_channel_unix_new (fd);
		g_io_channel_set_encoding (iochannel, NULL, NULL);
		g_io_channel_set_buffered (iochannel, FALSE);
		g_io_channel_set_close_on_unref (iochannel, TRUE);
		g_io_channel_set_flags (iochannel, (GIOFlags)(G_IO_FLAG_NONBLOCK | G_IO_FLAG_IS_READABLE), NULL);
		source_id = g_io_add_watch (iochannel, 
					    (GIOCondition)(G_IO_IN | G_IO_PRI | G_IO_ERR | G_IO_HUP | G_IO_NVAL),
 					    ReceiverMachine::iochannel_callback,
					    this);
	}

	virtual ~ReceiverMachine ()
	{
		if (source_id != 0)
			g_source_remove (source_id);

		g_io_channel_unref (iochannel);
		g_free (response_contents);
		g_free (message_contents);
	}
	
private:

	gboolean done_reading_header ()
	{
		message_contents = (char*)g_malloc0 (message_length + 1);

		current_state = READING_MESSAGE;
		read_offset = 0;
		return TRUE;
	}
	
	gboolean done_reading_message ()
	{
		// we're done reading the message, call the callback now and then switch to WRITING_RESPONSE
		response_contents = messageReceivedCallback (message_contents, messageReceivedCallbackData);

		response_length = response_contents ? strlen (response_contents) : 0;

		current_state = WRITING_HEADER;

		// since we're switching to writing, change the GIOConditions we're interested in
		g_source_remove (source_id);
		source_id = g_io_add_watch (iochannel, 
					    (GIOCondition)(G_IO_OUT | G_IO_ERR | G_IO_HUP | G_IO_NVAL),
					    ReceiverMachine::iochannel_callback,
					    this);

		write_offset = 0;

		return TRUE;
	}

	gboolean done_writing_header ()
	{
		// we're done writing the response length, let's write the response now.
		current_state = WRITING_RESPONSE;
		write_offset = 0;
		return TRUE;
	}

	gboolean done_writing_response ()
	{
		// we're done writing the response
		current_state = WAITING_FOR_SHUTDOWN;
		write_offset = 0;

		// since we're waiting for shutdown, change the GIOConditions to what we're interested in
		g_source_remove (source_id);
		source_id = g_io_add_watch (iochannel, 
					    (GIOCondition)(G_IO_HUP | G_IO_NVAL | G_IO_ERR),
					    ReceiverMachine::iochannel_callback,
					    this);

		return TRUE;
	}

	static gboolean iochannel_callback (GIOChannel *source,
					    GIOCondition condition,
					    gpointer data)
	{
		return ((ReceiverMachine*)data)->IOChannelCallback (condition);
	}

	gboolean IOChannelCallback (GIOCondition condition)
	{
		Deployment::SetCurrent (deployment);

		switch (current_state) {
		case READING_HEADER:
			return do_read (condition, fd,
					(char*)&message_length, sizeof (message_length),
					(StateMachine::SuccessFunc)&ReceiverMachine::done_reading_header);

		case READING_MESSAGE:
			return do_read (condition, fd,
					message_contents, message_length,
					(StateMachine::SuccessFunc)&ReceiverMachine::done_reading_message);

		case WRITING_HEADER:
			return do_write (condition, fd,
					 (char*)&response_length, sizeof (response_length),
					 (StateMachine::SuccessFunc)&ReceiverMachine::done_writing_header);

		case WRITING_RESPONSE:
			return do_write (condition, fd,
					 response_contents, response_length,
					 (StateMachine::SuccessFunc)&ReceiverMachine::done_writing_response);

		case WAITING_FOR_SHUTDOWN:
			current_state = FINISHED;

			finishedCallback (this, callbackData);

			return FALSE;

		case READ_ERROR:
		case WRITE_ERROR:
		case FINISHED:
		default:
			// shouldn't reach this...
			SetError (g_strdup ("invalid state in ReceiverMachine::IOChanelCallback"));

			errorCallback (this, callbackData);

			return FALSE;
		}
	}

	enum {
		READING_HEADER = StateMachine::NUM_STATES,
		READING_MESSAGE,
		WRITING_HEADER,
		WRITING_RESPONSE,
		WAITING_FOR_SHUTDOWN
	};
	
	int fd;
	GIOChannel *iochannel;
	guint source_id;

	int message_length;
	char *message_contents;

	int response_length;
	char *response_contents;

	MessageReceivedCallback messageReceivedCallback;
	gpointer messageReceivedCallbackData;

	Deployment *deployment;
};

class MoonMessageListenerGlib : public MoonMessageListener {
public:
	MoonMessageListenerGlib (const char *path, int fd)
	{
		deployment = Deployment::GetCurrent();

	  	this->path = g_strdup (path);
		this->fd = fd;
		source_id = 0;
		iochannel = g_io_channel_unix_new (fd);
		g_io_channel_set_encoding (iochannel, NULL, NULL);
		g_io_channel_set_buffered (iochannel, FALSE);
		g_io_channel_set_close_on_unref (iochannel, TRUE);
		g_io_channel_set_flags (iochannel, (GIOFlags)(G_IO_FLAG_NONBLOCK | G_IO_FLAG_IS_WRITEABLE | G_IO_FLAG_IS_READABLE), NULL);
	}

	virtual ~MoonMessageListenerGlib ()
	{
		unlink (path);
		g_free (path);

		if (source_id != 0)
			g_source_remove (source_id);

		g_io_channel_unref (iochannel);
	}
	
	virtual void AddMessageReceivedCallback (MessageReceivedCallback messageReceivedCallback,
						 gpointer data)
	{
		this->messageReceivedCallback = messageReceivedCallback;
		this->data = data;

		source_id = g_io_add_watch (iochannel, 
					    (GIOCondition)(G_IO_IN | G_IO_PRI | G_IO_ERR | G_IO_HUP | G_IO_NVAL),
					    MoonMessageListenerGlib::iochannel_callback,
					    this);
	}

	static gboolean iochannel_callback (GIOChannel *source,
					    GIOCondition condition,
					    gpointer data)
	{
		return ((MoonMessageListenerGlib*)data)->IOChannelCallback (condition);
	}

	gboolean IOChannelCallback (GIOCondition condition)
	{
		Deployment::SetCurrent (deployment);

		switch (condition) {
		case G_IO_IN:
		case G_IO_PRI: {
			// a peer has made a connection
			struct sockaddr peer_addr;
			socklen_t peer_addrlen = sizeof (peer_addr);

			int sender_fd = accept (fd, &peer_addr, &peer_addrlen);

			if (sender_fd == -1) {
				perror ("accept");
				// XXX we need to figure out what to do here...
				return TRUE;
			}

			new ReceiverMachine (sender_fd,
					     messageReceivedCallback,
					     data,
					     MoonMessageListenerGlib::machine_finished_callback,
					     MoonMessageListenerGlib::machine_error_callback,
					     this);

			return TRUE;
		}
		case G_IO_HUP:
		case G_IO_ERR:
		case G_IO_NVAL:
			// error condition, let's remove the callback
			return FALSE;
		default:
			return TRUE;
		}
	}

	static void machine_finished_callback (gpointer data, gpointer user_data)
	{
	    ReceiverMachine *machine = (ReceiverMachine*)data;

	    delete machine;
	}

	static void machine_error_callback (gpointer data, gpointer user_data)
	{
	    ReceiverMachine *machine = (ReceiverMachine*)data;
	    delete machine;
	}

	virtual void RemoveMessageReceivedCallback ()
	{
		g_source_remove (source_id);
		messageReceivedCallback = NULL;
		data = NULL;
	}

private:
	char *path;
	int fd;
	GIOChannel *iochannel;
	guint source_id;

	MessageReceivedCallback messageReceivedCallback;
	gpointer data;

	Deployment *deployment;
};


class SenderMachine : public StateMachine {
public:
	SenderMachine (const char *filename,
		       const char *domain_filename,
		       MessageSentCallback messageSentCallback,
		       gpointer messageSentCallbackData,
		       GFunc finishedCallback,
		       GFunc errorCallback,
		       gpointer callbackData)
		: StateMachine (finishedCallback, errorCallback, callbackData)
	{
		deployment = Deployment::GetCurrent ();

		this->messageSentCallback = messageSentCallback;
		this->messageSentCallbackData = messageSentCallbackData;

		message_length = 0;
		message_contents = NULL;

		response_length = 0;
		response_contents = NULL;

		fd = socket (AF_LOCAL, SOCK_STREAM, 0);

		iochannel = g_io_channel_unix_new (fd);
		g_io_channel_set_encoding (iochannel, NULL, NULL);
		g_io_channel_set_buffered (iochannel, FALSE);
		g_io_channel_set_close_on_unref (iochannel, TRUE);
		g_io_channel_set_flags (iochannel, (GIOFlags)(G_IO_FLAG_NONBLOCK | G_IO_FLAG_IS_WRITEABLE | G_IO_FLAG_IS_READABLE), NULL);

		source_id = 0;

		this->filename = g_strdup (filename);
		this->domain_filename = g_strdup (domain_filename);

		current_state = INITIAL;
	}

	void Send (const char *domain, const char *msg, gpointer managedUserState)
	{
		if (current_state != FINISHED &&
		    current_state != INITIAL &&
		    current_state != READ_ERROR &&
		    current_state != WRITE_ERROR) {
			g_warning ("invalid state in SenderMachine::Send");
		}

		this->domain = g_strdup (domain);
		domain_length = strlen (domain) + 1; // note the + 1

		this->message_contents = g_strdup (msg);
		message_length = strlen (msg);
		
		total_length = domain_length + message_length;

		this->managedUserState = managedUserState;

		bool add_source = false;

		char *receiver_filename = filename;

	try_again:
		memset (&addr, 0, sizeof (addr));
		addr.sun_family = AF_LOCAL;
		g_strlcpy (addr.sun_path, receiver_filename, sizeof (addr.sun_path));

		if (-1 == connect (fd, (struct sockaddr*) &addr, sizeof (addr))) {
			if (errno == EINPROGRESS) {
				current_state = CONNECTING;
				add_source = true;
			}
			else {
				if (domain_filename && receiver_filename != domain_filename) {
					receiver_filename = domain_filename;
					goto try_again;
				}
					
				error = g_strdup_printf ("SenderMachine had an error connecting: %s", strerror (errno));
				current_state = WRITE_ERROR;

				errorCallback (this, callbackData);
				return;
			}
		}
		else {
			current_state = WRITING_HEADER;
			add_source = true;
		}

		if (add_source)
			source_id = g_io_add_watch (iochannel, 
						    (GIOCondition)(G_IO_OUT | G_IO_ERR | G_IO_HUP | G_IO_NVAL),
						    SenderMachine::iochannel_callback,
						    this);

	}

	virtual ~SenderMachine ()
	{
		if (source_id != 0)
			g_source_remove (source_id);

		g_io_channel_unref (iochannel);
		g_free (response_contents);
		g_free (message_contents);
		g_free (filename);
		g_free (domain_filename);
	}

	const char *GetMessageContents () { return message_contents; }
	Deployment *GetDeployment () { return deployment; }

private:
	void on_finished ()
	{
		messageSentCallback (NULL, message_contents, response_contents, managedUserState, messageSentCallbackData);

		finishedCallback (this, callbackData);
	}

	gboolean done_reading_header ()
	{
		// we're done reading the response length, let's read the content now.

		// XXX should this not be null if response_length == 0?
		response_contents = (char*)g_malloc0 (response_length + 1);

		if (response_length == 0) {
			current_state = FINISHED;
			on_finished ();
			return FALSE;
		}
		else {
			current_state = READING_RESPONSE;
			read_offset = 0;
			return TRUE;
		}
	}

	gboolean done_reading_response ()
	{
		// we're done reading the response, call the message sent callback now
		current_state = FINISHED;

		on_finished ();

		return FALSE;
	}

	gboolean done_writing_header ()
	{
		// we're done writing the total length, let's write the domain now.
		current_state = WRITING_DOMAIN;
		write_offset = 0;
		return TRUE;
	}

	gboolean done_writing_domain ()
	{
		// we're done writing the domain, switch to writing the message contents
		current_state = WRITING_MESSAGE;
		write_offset = 0;
		return TRUE;
	}

	gboolean done_writing_message ()
	{
		// we're done writing the message, switch to reading the response
		current_state = READING_HEADER;

		g_source_remove (source_id);

		// since we're switching to writing, change the GIOConditions we're interested in
		source_id = g_io_add_watch (iochannel, 
					    (GIOCondition)(G_IO_IN | G_IO_PRI | G_IO_ERR | G_IO_HUP | G_IO_NVAL),
					    SenderMachine::iochannel_callback,
					    this);

		write_offset = 0;
		return TRUE;
	}

	gboolean connecting (GIOCondition condition)
	{
		if (IOCHANNEL_READY_FOR_WRITE (condition)) {
			
			int connect_error = 0;
			socklen_t len = sizeof (connect_error);

			if (-1 == getsockopt (fd, SOL_SOCKET, SO_ERROR, &connect_error, &len)) {
				SetError (g_strdup_printf ("unable to determine connection state of socket: %s\n", strerror(errno)));
				errorCallback (this, callbackData);
				return FALSE;
			}

			if (connect_error == 0) {
				// successful connect
				current_state = WRITING_HEADER;
				return TRUE;
			}
			else {
				SetError (g_strdup_printf ("SenderMachine had an error connecting: %s", strerror (connect_error)));
				errorCallback (this, callbackData);
				return FALSE;
			}
		}
		else /*if (IOCHANNEL_ERROR (condition))*/ {
			current_state = READ_ERROR;
			SetError (g_strdup ("GIOChannel error 1"));

			errorCallback (this, callbackData);

			return FALSE;
		}
	}

	static gboolean iochannel_callback (GIOChannel *source,
					    GIOCondition condition,
					    gpointer data)
	{
		return ((SenderMachine*)data)->IOChannelCallback (condition);
	}

	gboolean IOChannelCallback (GIOCondition condition)
	{
		Deployment::SetCurrent (deployment);

		switch (current_state) {
		case CONNECTING:
			return connecting (condition);

		case READING_HEADER:
			return do_read (condition, fd,
					(char*)&response_length, sizeof (response_length),
					(StateMachine::SuccessFunc)&SenderMachine::done_reading_header);

		case READING_RESPONSE:
			return do_read (condition, fd,
					response_contents, response_length,
					(StateMachine::SuccessFunc)&SenderMachine::done_reading_response);

		case WRITING_HEADER:
			return do_write (condition, fd,
					 (char*)&total_length, sizeof (total_length),
					 (StateMachine::SuccessFunc)&SenderMachine::done_writing_header);

		case WRITING_DOMAIN:
			return do_write (condition, fd,
					 domain, domain_length,
					 (StateMachine::SuccessFunc)&SenderMachine::done_writing_domain);

		case WRITING_MESSAGE:
			return do_write (condition, fd,
					 message_contents, message_length,
					 (StateMachine::SuccessFunc)&SenderMachine::done_writing_message);

		case INITIAL:
		case READ_ERROR:
		case WRITE_ERROR:
		case FINISHED:
		default:
			// shouldn't reach this...
			g_warning ("invalid state in SenderMachine::IOChanelCallback");
			return FALSE;
		}
	}

	enum {
		CONNECTING = StateMachine::NUM_STATES,
		WRITING_HEADER,
		WRITING_DOMAIN,
		WRITING_MESSAGE,
		READING_HEADER,
		READING_RESPONSE,
	};

	struct sockaddr_un addr;

	int fd;
	GIOChannel *iochannel;
	guint source_id;

	char *filename;
	char *domain_filename;

	int total_length;

	int message_length;
	char *message_contents;

	int domain_length;
	char *domain;

	int response_length;
	char *response_contents;

	MessageSentCallback messageSentCallback;
	gpointer messageSentCallbackData;

	gpointer managedUserState;

	Deployment *deployment;
};

class MoonMessageSenderGlib : public MoonMessageSender {
public:
	MoonMessageSenderGlib (const char *listener_path, const char *domain_listener_path, const char *domain)
	{
		this->listener_path = g_strdup (listener_path);
		this->domain_listener_path = g_strdup (domain_listener_path);
		this->domain = g_strdup (domain);
	}

	virtual ~MoonMessageSenderGlib ()
	{
		g_free (listener_path);
		g_free (domain_listener_path);
		g_free (domain);
	}
	
	virtual void AddMessageSentCallback (MessageSentCallback messageSentCallback, gpointer data)
	{
		this->messageSentCallback = messageSentCallback;
		this->messageSentCallbackData = data;
	}

	virtual void RemoveMessageSentCallback ()
	{
		this->messageSentCallback = NULL;
		this->messageSentCallbackData = NULL;
	}

	virtual void SendMessageAsync (const char *msg, gpointer managedUserState, MoonError *error)
	{
		SenderMachine *machine = new SenderMachine (listener_path,
							    domain_listener_path,
							    messageSentCallback,
							    messageSentCallbackData,
							    MoonMessageSenderGlib::machine_finished_callback,
							    MoonMessageSenderGlib::machine_error_callback,
							    this);

		machine->Send (domain, msg, managedUserState);
	}

private:
	static void machine_finished_callback (gpointer data, gpointer user_data)
	{
		SenderMachine *machine = (SenderMachine *)data;
		delete machine;
	}

	void MachineError (char *error, char *msg)
	{
		if (messageSentCallback) {
			MoonError err;
			MoonError::FillIn (&err, MoonError::SEND_FAILED, error);
			messageSentCallback (&err, msg, NULL, NULL, messageSentCallbackData);
		}
		g_free (error);
		g_free (msg);
	}

	struct ErrorClosure {
		char *error;
		char *msg;
		MoonMessageSenderGlib *sender;
		Deployment *deployment;
	};

	static bool idle_machine_error (gpointer data)
	{
		ErrorClosure* error_closure = (ErrorClosure*)data;
		char *error = error_closure->error;
		char *msg = error_closure->msg;
		MoonMessageSenderGlib *sender = error_closure->sender;
		Deployment *deployment = error_closure->deployment;

		delete error_closure;

		Deployment::SetCurrent (deployment);
		
		sender->MachineError (error, msg);

		return false;
	}
	
	static void machine_error_callback (gpointer data, gpointer user_data)
	{
		SenderMachine *machine = (SenderMachine*)data;
		MoonMessageSenderGlib *sender = (MoonMessageSenderGlib*)user_data;
		
		g_warning ("SenderMachine had an error: %s\n", machine->GetError());

		delete machine;

		ErrorClosure *error_closure = new ErrorClosure();
		error_closure->error = g_strdup (machine->GetError());
		error_closure->msg = g_strdup (machine->GetMessageContents());
		error_closure->sender = sender;
		error_closure->deployment = machine->GetDeployment();

		runtime_get_windowing_system()->AddIdle (idle_machine_error, error_closure);
	}

	char *listener_path;
	char *domain_listener_path;
	char *domain;

	MessageSentCallback messageSentCallback;
	gpointer messageSentCallbackData;
};

MoonMessagingServiceGlib::MoonMessagingServiceGlib ()
{
}

MoonMessagingServiceGlib::~MoonMessagingServiceGlib ()
{
}

MoonMessageListener*
MoonMessagingServiceGlib::CreateMessagingListener (const char *domain, const char *listenerName, MoonError *error)
{
	char *listener_path = create_listener_path (domain, listenerName);

	printf ("listener path = %s\n", listener_path);

	struct sockaddr_un addr;

	memset (&addr, 0, sizeof (addr));
	addr.sun_family = AF_LOCAL;
	g_strlcpy (addr.sun_path, listener_path, sizeof (addr.sun_path));

 try_bind:
	int sock_fd = socket (AF_LOCAL, SOCK_STREAM, 0);

	if (-1 == bind (sock_fd, (struct sockaddr*) &addr, sizeof (addr))) {
		if (errno == EADDRINUSE) {
			// there's already a socket there.  don't
			// fret, though - it might be dead.  try to
			// connect to it.
			if (0 == connect (sock_fd, (struct sockaddr*) &addr, sizeof (addr))) {
				// nope, it's really there and alive. hang up and return the error.
				close (sock_fd);
				MoonError::FillIn (error, MoonError::LISTEN_FAILED, "LocalMessageReceiver already listening");
				return NULL;
			}
			else {
				// nobody is listening, unlink the socket and try again
				close (sock_fd);
				if (-1 == unlink (listener_path)) {
					perror ("unlink");
					MoonError::FillIn (error, MoonError::LISTEN_FAILED, "LocalMessageReceiver not listening, but failed to remove socket");
					return NULL;
				}
				goto try_bind;
			}
		}
		else {
			close (sock_fd);
			MoonError::FillIn (error, MoonError::LISTEN_FAILED, "bind system call failed");
			return NULL;
		}
	}

	if (-1 == listen (sock_fd, 128)) {
		perror ("listen");

		close (sock_fd);
		MoonError::FillIn (error, MoonError::LISTEN_FAILED, "listen system call failed");
		return NULL;
	}

	MoonMessageListener *listener = new MoonMessageListenerGlib (listener_path, sock_fd);

	g_free (listener_path);

	return listener;
}

MoonMessageSender*
MoonMessagingServiceGlib::CreateMessagingSender (const char *listenerName, const char *listenerDomain, const char *domain, MoonError *error)
{
	char *listener_path = create_listener_path (listenerDomain, listenerName);
	char *domain_listener_path = NULL;

	if (!strcmp (listenerDomain, "*"))
		domain_listener_path = create_listener_path (domain, listenerName);

	printf ("sender path = %s\n", listener_path);

	MoonMessageSender *sender = new MoonMessageSenderGlib (listener_path, domain_listener_path, domain);

	g_free (listener_path);

	return sender;
}
