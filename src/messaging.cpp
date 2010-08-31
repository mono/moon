/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * messaging.cpp: client side messaging
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include "config.h"

#include <stdio.h>
#include <string.h>

#include "messaging.h"
#include "runtime.h"
#include "deployment.h"
#include "uri.h"

namespace Moonlight {

LocalMessageReceiver::LocalMessageReceiver (const char *receiverName,
					    ReceiverNameScope namescope)
{
	SetObjectType (Type::LOCALMESSAGERECEIVER);
	listener = NULL;
	this->receiverName = g_strdup (receiverName);
	this->namescope = namescope;
	if (namescope == ReceiverNameScopeGlobal)
		this->receiverDomain = g_strdup ("*");
	else {
		this->receiverDomain = NULL;

		Deployment *deployment = Deployment::GetCurrent();
		if (deployment && deployment->GetXapLocation ()) {
			this->receiverDomain = g_strdup (deployment->GetXapLocation ()->GetHost ());
		}
		else {
			g_warning ("LocalMessageReceiver.ctor no deployment or xap location");
		}

		if (this->receiverDomain == NULL)
			this->receiverDomain = g_strdup ("*");
	}
}

LocalMessageReceiver::~LocalMessageReceiver ()
{
	delete listener;
	listener = NULL;

	g_free (receiverName);
	g_free (receiverDomain);
}

void
LocalMessageReceiver::SetAllowedSenderDomains (char **allowedSenderDomains, int count)
{
	this->allowedSenderDomains = g_new (char*, count);
	this->allowedSenderDomainsCount = count;

	for (int i = 0; i < count; i ++) {
		this->allowedSenderDomains[i] = g_strdup (allowedSenderDomains[i]);
	}
}

void
LocalMessageReceiver::ListenWithError (MoonError *error)
{
	if (listener != NULL) {
		// FIXME: test that this is actually an error
		MoonError::FillIn (error, MoonError::EXCEPTION, "already listening with this LocalMessageReceiver.");
		return;
	}

	listener = runtime_get_messaging_service ()->CreateMessagingListener (receiverDomain, receiverName, error);

	if (listener == NULL) {
		// FIXME: there might be varying error conditions
		// (someone's already registered this
		// domain/receiverName, etc).  find out which need to
		// be mapped to error codes, and which codes.
		return;
	}

	listener->AddMessageReceivedCallback (MessageReceivedHandler, this);

	// we unref_delayed in ::Dispose.
	ref ();
}

char*
LocalMessageReceiver::MessageReceivedHandler (const char *msg, gpointer data)
{
	LocalMessageReceiver *receiver = (LocalMessageReceiver*)data;

	return receiver->MessageReceived(msg);
}

char*
LocalMessageReceiver::MessageReceived (const char *msg)
{
	const char *domain = msg;
	bool allowed = false;

	for (int i = 0; i < allowedSenderDomainsCount; i ++) {
		if (!g_ascii_strcasecmp (allowedSenderDomains[i], "*") || !g_ascii_strcasecmp (allowedSenderDomains[i], domain)) {
			allowed = true;
			break;
		}
	}

	if (!allowed) {
		// we ignore any message from a domain that the
		// receiver isn't configured to allow.

		// XXX this needs to return an *error*, not a valid value
		return NULL;
	}

	msg = msg + strlen (domain) + 1;

	MessageReceivedEventArgs *args = new MessageReceivedEventArgs (msg,
								       namescope,
								       receiverName,
								       domain);
	args->ref (); /* so that they survive past the Emit */

	Emit (MessageReceivedEvent, args);

	char *response;
	const char *args_response = args->GetResponse();
	if (args_response && *args_response)
		response = g_strdup (args_response);
	else
		response = g_strdup (receiverName);

	args->unref ();

	return response;
}

void
LocalMessageReceiver::DisposeWithError (MoonError *error)
{
	delete listener;
	listener = NULL;
	unref_delayed ();
}

//// Senders


LocalMessageSender::LocalMessageSender (const char *receiverName, const char *receiverDomain)
{
	SetObjectType (Type::LOCALMESSAGESENDER);

	this->receiverName = g_strdup (receiverName);

	this->senderDomain = NULL;

	Deployment *deployment = Deployment::GetCurrent();
	if (deployment && deployment->GetXapLocation ()) {
		this->senderDomain = g_strdup (deployment->GetXapLocation ()->GetHost ());
	}

	if (this->senderDomain == NULL)
		this->senderDomain = g_strdup ("");

	this->receiverDomain = g_strdup (receiverDomain && *receiverDomain ? receiverDomain : senderDomain);

	sender = NULL;
}

LocalMessageSender::~LocalMessageSender ()
{
	delete sender;
	sender = NULL;

	g_free (receiverName);
	g_free (receiverDomain);
	g_free (senderDomain);
}

void
LocalMessageSender::SendAsyncWithError (const char *msg, gpointer managedUserState, MoonError *error)
{
	if (sender == NULL) {
		sender = runtime_get_messaging_service ()->CreateMessagingSender (receiverName,
										  receiverDomain,
										  senderDomain,
										  error);
		if (sender == NULL)
			return;

		sender->AddMessageSentCallback (LocalMessageSender::MessageSentHandler, this);
	}

	sender->SendMessageAsync (msg, managedUserState, error);
}

void
LocalMessageSender::MessageSentHandler (MoonError *error, const char *message, const char *response, gpointer managedUserState, gpointer data)
{
	LocalMessageSender *sender = (LocalMessageSender*)data;

	sender->MessageSent (error, message, response, managedUserState);
}

void
LocalMessageSender::MessageSent (MoonError *error, const char *message, const char *response, gpointer managedUserState)
{
	if (HasHandlers (SendCompletedEvent)) {
		Emit (SendCompletedEvent, new SendCompletedEventArgs (error,
								      message,
								      receiverName,
								      receiverDomain,
								      response,
								      managedUserState));
	}
}

};
