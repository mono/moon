/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * downloader.h: Downloader class.
 *
 * Contact:
 *   Moonlight List (moonligt-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __DOWNLOADER_H__
#define __DOWNLOADER_H__

#include <glib.h>

#include "dependencyobject.h"
#include "internal-downloader.h"

class FileDownloader;
class Downloader;
class DownloaderResponse;

/* @CBindingRequisite */
typedef void     (* DownloaderResponseHeaderCallback) (gpointer context, const char *header, const char *value);

/* @CBindingRequisite */
typedef void     (* DownloaderWriteFunc) (void *buf, gint32 offset, gint32 n, gpointer cb_data);
/* @CBindingRequisite */
typedef void     (* DownloaderNotifySizeFunc) (gint64 size, gpointer cb_data);

/* @CBindingRequisite */
typedef gpointer (* DownloaderCreateStateFunc) (Downloader *dl);
/* @CBindingRequisite */
typedef void     (* DownloaderDestroyStateFunc) (gpointer state);
/* @CBindingRequisite */
/*
 * custom_header_support:
 *    must be set to true if HeaderFunc or BodyFunc is called later
 * disable_cache:
 *    must be set to true if there are multiple simultaneous requests to the same uri
 *    when a browser is the
 */
typedef void     (* DownloaderOpenFunc) (gpointer state, const char *verb, const char *uri, bool custom_header_support, bool disable_cache);
/* @CBindingRequisite */
typedef void     (* DownloaderSendFunc) (gpointer state);
/* @CBindingRequisite */
typedef void     (* DownloaderAbortFunc) (gpointer state);
/* @CBindingRequisite */
typedef void     (* DownloaderHeaderFunc) (gpointer state, const char *header, const char *value);
/* @CBindingRequisite */
typedef void     (* DownloaderBodyFunc) (gpointer state, void *body, guint32 length);
/* @CBindingRequisite */
typedef gpointer (* DownloaderCreateWebRequestFunc) (const char *method, const char *uri, gpointer context);
/* @CBindingRequisite */
typedef void     (* DownloaderSetResponseHeaderCallbackFunc) (gpointer state, DownloaderResponseHeaderCallback callback, gpointer context);
/* @CBindingRequisite */
typedef DownloaderResponse * (* DownloaderGetResponseFunc) (gpointer state);

// Reference:	URL Access Restrictions in Silverlight 2
//		http://msdn.microsoft.com/en-us/library/cc189008(VS.95).aspx
enum DownloaderAccessPolicy {
	DownloaderPolicy,
	MediaPolicy,
	XamlPolicy,
	FontPolicy,
	StreamingPolicy,
	MsiPolicy,
	NoPolicy
};

/* @Namespace=None */
/* @ManagedDependencyProperties=None */
/* @ManagedEvents=Manual */
class MOON_API Downloader : public DependencyObject {
	static DownloaderCreateStateFunc create_state;
	static DownloaderDestroyStateFunc destroy_state;
	static DownloaderOpenFunc open_func;
	static DownloaderSendFunc send_func;
	static DownloaderAbortFunc abort_func;
	static DownloaderHeaderFunc header_func;
	static DownloaderBodyFunc body_func;
	static DownloaderCreateWebRequestFunc request_func;
	static DownloaderSetResponseHeaderCallbackFunc set_response_header_callback_func;
	static DownloaderGetResponseFunc get_response_func;

	// Set by the consumer
	DownloaderNotifySizeFunc notify_size;
	DownloaderWriteFunc writer;
	gpointer user_data;
	
	// Set by the supplier.
	gpointer downloader_state;
	
	gpointer context;
	
	gint64 file_size;
	gint64 total;
	
	char *filename;
	char *buffer;
	
	char *failed_msg;
	
	int send_queued:1;
	int completed:1;
	int started:1;
	int aborted:1;
	int custom_header_support:1;
	int disable_cache:1;
	
	InternalDownloader *internal_dl;

	DownloaderAccessPolicy access_policy;
	
 protected:
	virtual ~Downloader ();
	
	void SetStatusText (const char *text);
	void SetStatus (int status);
	
 public:
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int DownloadProgressProperty;
	/* @PropertyType=string */
	const static int ResponseTextProperty;
	/* @PropertyType=gint32,DefaultValue=0,GenerateAccessors */
	const static int StatusProperty;
	/* @PropertyType=string,DefaultValue=\"\",GenerateAccessors */
	const static int StatusTextProperty;
	/* @PropertyType=Uri,GenerateAccessors */
	const static int UriProperty;
	
	// Events you can AddHandler to
	const static int CompletedEvent;
	const static int DownloadProgressChangedEvent;
	const static int DownloadFailedEvent;
	
	/* @GenerateCBinding,GeneratePInvoke */
	Downloader ();
	
	static bool ValidateDownloadPolicy (const char *location, Uri *uri, DownloaderAccessPolicy policy);
	
	void Abort ();
	char *GetResponseText (const char *Partname, gint64 *size);
	char *GetDownloadedFilename (const char *partname);
	void Open (const char *verb, const char *uri, DownloaderAccessPolicy policy);
	void Open (const char *verb, Uri *uri, DownloaderAccessPolicy policy);
	void SendInternal ();
	void Send ();
	void SendNow ();
	
	// the following is stuff not exposed by C#/js, but is useful
	// when writing unmanaged code for downloader implementations
	// or data sinks.
	
	void OpenInitialize ();
	void InternalAbort ();
	void InternalWrite (void *buf, gint32 offset, gint32 n);
	void InternalOpen (const char *verb, const char *uri);
	void InternalSetHeader (const char *header, const char *value);
	void InternalSetHeaderFormatted (const char *header, char *value); // calls g_free on the value
	void InternalSetBody (void *body, guint32 length);
	
	/* @GenerateCBinding,GeneratePInvoke */
	void Write (void *buf, gint32 offset, gint32 n);
	
	/* @GenerateCBinding,GeneratePInvoke */
	void NotifyFinished (const char *final_uri);
	
	/* @GenerateCBinding,GeneratePInvoke */
	void NotifyFailed (const char *msg);
	
	/* @GenerateCBinding,GeneratePInvoke */
	void NotifySize (gint64 size);
	
	bool CheckRedirectionPolicy (const char *url);

	void SetFilename (const char *fname);
	char *GetBuffer () { return buffer; }
	gint64 GetSize () { return total; }
	
	InternalDownloader *GetInternalDownloader () { return internal_dl; }
	
	// This is called by the consumer of the downloaded data (the
	// Image class for instance)
	void SetStreamFunctions (DownloaderWriteFunc writer,
				 DownloaderNotifySizeFunc notify_size,
				 gpointer user_data);
	
	// This is called by the supplier of the downloaded data (the
	// managed framework, the browser plugin, the demo test)
	
	/* @GenerateCBinding,GeneratePInvoke */
	static void SetFunctions (DownloaderCreateStateFunc create_state,
				  DownloaderDestroyStateFunc destroy_state,
				  DownloaderOpenFunc open,
				  DownloaderSendFunc send,
				  DownloaderAbortFunc abort,
				  DownloaderHeaderFunc header,
				  DownloaderBodyFunc body,
			      DownloaderCreateWebRequestFunc request,
			      DownloaderSetResponseHeaderCallbackFunc response_header_callback,
			      DownloaderGetResponseFunc get_response
				);
		
	bool Started ();
	bool Completed ();
	bool IsAborted () { return aborted; }
	const char *GetFailedMessage () { return failed_msg; }
	
	void SetRequireCustomHeaderSupport (bool value) { custom_header_support = value; }
	bool GetRequireCustomHeaderSupport () { return custom_header_support; }
	void SetDisableCache (bool value) { disable_cache = value; }
	bool GetDisableCache () { return disable_cache; }

	void     SetContext (gpointer context) { this->context = context;}
	gpointer GetContext () { return context; }
	gpointer GetDownloaderState () { return downloader_state; }

	DownloaderCreateWebRequestFunc GetRequestFunc () {return request_func; }
	/* @GenerateCBinding,GeneratePInvoke */
	void *CreateWebRequest (const char *method, const char *uri);
	void SetResponseHeaderCallback (DownloaderResponseHeaderCallback callback, gpointer context);

	DownloaderResponse * GetResponse ();
	//
	// Property Accessors
	//
	void SetDownloadProgress (double progress);
	double GetDownloadProgress ();
	
	const char *GetStatusText ();
	int GetStatus ();
	
	void SetUri (Uri *uri);
	Uri *GetUri ();
};

class DownloaderResponse;
class DownloaderRequest;

/* @CBindingRequisite */
typedef guint32 (* DownloaderResponseStartedHandler) (DownloaderResponse *response, gpointer context);
/* @CBindingRequisite */
typedef guint32 (* DownloaderResponseDataAvailableHandler) (DownloaderResponse *response, gpointer context, char *buffer, guint32 length);
/* @CBindingRequisite */
typedef guint32 (* DownloaderResponseFinishedHandler) (DownloaderResponse *response, gpointer context, bool success, gpointer data, const char *uri);

class IDownloader {
 private:
	Deployment *deployment;

 public:
	virtual ~IDownloader () {};

	virtual void Abort () = 0;
	virtual const bool IsAborted () = 0;
	Deployment *GetDeployment () { return deployment; }
	void SetDeployment (Deployment *deployment) { this->deployment = deployment; }
};

class MOON_API DownloaderResponse : public IDownloader {
 protected:
	DownloaderResponseStartedHandler started;
	DownloaderResponseDataAvailableHandler available;
	DownloaderResponseFinishedHandler finished;
	gpointer context;
	DownloaderRequest *request;
	bool aborted;

 public:
	DownloaderResponse ();
	DownloaderResponse (DownloaderResponseStartedHandler started, DownloaderResponseDataAvailableHandler available, DownloaderResponseFinishedHandler finished, gpointer context);
	/* @GenerateCBinding,GeneratePInvoke */
	virtual ~DownloaderResponse ();

	/* @GenerateCBinding,GeneratePInvoke */
	virtual void Abort () = 0;
	virtual const bool IsAborted () { return this->aborted; }
	/* @GenerateCBinding,GeneratePInvoke */
	virtual void SetHeaderVisitor (DownloaderResponseHeaderCallback visitor, gpointer context) = 0;
	/* @GenerateCBinding,GeneratePInvoke */
	virtual int GetResponseStatus () = 0;
	/* @GenerateCBinding,GeneratePInvoke */
	virtual const char * GetResponseStatusText () = 0;
	DownloaderRequest *GetDownloaderRequest () { return request; }
	void SetDownloaderRequest (DownloaderRequest *value) { request = value; }
	
	virtual void ref () = 0;
	virtual void unref () = 0;
};

class MOON_API DownloaderRequest : public IDownloader {
 protected:
 	DownloaderResponse *response;
	char *uri;
	char *method;

	bool aborted;

 public:
	DownloaderRequest (const char *method, const char *uri);
	/* @GenerateCBinding,GeneratePInvoke */
	virtual ~DownloaderRequest ();

	/* @GenerateCBinding,GeneratePInvoke */
	virtual void Abort () = 0;
	/* @GenerateCBinding,GeneratePInvoke */
	virtual bool GetResponse (DownloaderResponseStartedHandler started, DownloaderResponseDataAvailableHandler available, DownloaderResponseFinishedHandler finished, gpointer context) = 0;
	/* @GenerateCBinding,GeneratePInvoke */
	virtual const bool IsAborted () { return this->aborted; }
	/* @GenerateCBinding,GeneratePInvoke */
	virtual void SetHttpHeader (const char *name, const char *value) = 0;
	/* @GenerateCBinding,GeneratePInvoke */
	virtual void SetBody (/* @MarshalAs=byte[] */ void *body, int size) = 0;
	/* @GenerateCBinding,GeneratePInvoke */
	DownloaderResponse *GetDownloaderResponse () { return response; }
	void SetDownloaderResponse (DownloaderResponse *value) { response = value; }
};

G_BEGIN_DECLS

void downloader_init (void);

G_END_DECLS

#endif
