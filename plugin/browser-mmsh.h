/*
 * browser-mmsh.h: Moonlight plugin routines for mms over http requests/responses.
 *
 * Author:
 *   Fernando Herrera (fherrera@novell.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include "moonlight.h"
#include "runtime.h"

#include <nsCOMPtr.h>
#include <nsXPCOM.h>
#include <nsIURI.h>
#include <nsIServiceManager.h>
#include <nsIComponentManager.h>
#include <nsIIOService.h>
#include <nsStringAPI.h>
#include <nsIInputStream.h>
#include <nsIOutputStream.h>
#include <nsIStreamListener.h>
#include <nsEmbedString.h>
#include <nsIChannel.h>
#include <nsIRequest.h>
#include <nsIRequestObserver.h>
#include <nsIHttpChannel.h>
#include <nsIHttpHeaderVisitor.h>
#include <nsEmbedString.h>
#include <nsIUploadChannel.h>

// unfrozen apis
#include <necko/nsNetError.h>
#include <xpcom/nsIStorageStream.h>

typedef void (* HttpHeaderHandler) (const char *name, const char *value);

class BrowserMmshResponse : public nsIHttpHeaderVisitor {
	nsCOMPtr<nsIChannel> channel;
	HttpHeaderHandler handler;

protected:
	NS_DECL_NSIHTTPHEADERVISITOR

public:
	NS_DECL_ISUPPORTS

	BrowserMmshResponse (nsCOMPtr<nsIChannel> channel) : handler (NULL)
	{
		this->channel = channel;
	}

	virtual ~BrowserMmshResponse ()
	{
	}

	void Abort ();
	void VisitHeaders (HttpHeaderHandler handler);
	char *GetStatus (int *code);

	//virtual void *Read (int *length) = 0;
};

class AsyncBrowserMmshResponse;

typedef void (* AsyncMmshResponseFinishedHandler) (BrowserMmshResponse *response, gpointer context);
typedef void (* AsyncMmshResponseNotifierHandler) (BrowserMmshResponse *response, gpointer context, char* name, int64_t size);
typedef void (* AsyncMmshResponseDataAvailableHandler) (BrowserMmshResponse *response, gpointer context, char* buffer, int offset, PRUint32 length);

class AsyncBrowserMmshResponse : public BrowserMmshResponse, public nsIStreamListener {
	AsyncMmshResponseFinishedHandler finisher;
	AsyncMmshResponseNotifierHandler notifier;
	AsyncMmshResponseDataAvailableHandler reader;
	gpointer context;
	char *tmp_buffer;
	uint32_t tmp_size;
	uint32_t size;
	uint16_t asf_packet_size;
	int64_t notify_size;
	char *notify_name;
	
protected:
	NS_DECL_NSIREQUESTOBSERVER
	NS_DECL_NSISTREAMLISTENER

public:
	NS_DECL_ISUPPORTS

	AsyncBrowserMmshResponse (nsCOMPtr<nsIChannel> channel,
				  AsyncMmshResponseDataAvailableHandler reader, 
				  AsyncMmshResponseNotifierHandler notifier, 
				  AsyncMmshResponseFinishedHandler finisher,
				  gpointer context)
		: BrowserMmshResponse (channel)
	{
		this->tmp_buffer = NULL;
		this->tmp_size = 0;
		this->asf_packet_size = 0;
		this->size = 0;
		this->notify_size = 0;
		this->notify_name = NULL;
		
		this->notifier = notifier;
		this->reader = reader;
		this->finisher = finisher;
		this->context = context;
	}

	void MmsMetadataParse (int, const char*);
	virtual ~AsyncBrowserMmshResponse ()
	{
	}

};

class BrowserMmshRequest {
	const char *uri;
	const char *method;

	nsCOMPtr<nsIChannel> channel;

	void CreateChannel ();
public:

	BrowserMmshRequest (const char *method, const char *uri)
	{
		this->method = g_strdup (method);
		this->uri = g_strdup (uri);

		CreateChannel ();
	}

	~BrowserMmshRequest ()
	{
		g_free ((gpointer) uri);
		g_free ((gpointer) method);
	}

	bool GetAsyncResponse (AsyncMmshResponseDataAvailableHandler reader, 
			       AsyncMmshResponseNotifierHandler notifier,
			       AsyncMmshResponseFinishedHandler finisher, gpointer context);
	void SetHttpHeader (const char *name, const char *value);
	void SetBody (const char *body, int size);
};
