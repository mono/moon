/*
 * browser-http.cpp: Moonlight plugin routines for http requests/responses.
 *
 * Author:
 *   Jb Evain (jbevain@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include "browser-http.h"

// SyncBrowserHttpResponse

void *
SyncBrowserHttpResponse::Read (int *size)
{
	PRUint32 read = 0;
	char *data = NULL;

	while (true) {
		PRUint32 available, len;
		response_stream->Available (&available);

		if (data == NULL) {
			data = (char *) NS_Alloc (available);
		} else if (available == 0) {
			break;
		} else {
			data = (char *)NS_Realloc (data, read + available);
		}

		response_stream->Read (data + read, available, &len);

		if (len == 0)
			break;

		read += len;
	}

	*size = read;

	return data;
}

// BrowserHttpRequest

void
BrowserHttpRequest::CreateChannel ()
{
	nsresult rv = NS_OK;
	nsCOMPtr<nsIServiceManager> mgr;
	rv = NS_GetServiceManager (getter_AddRefs (mgr));
	if (NS_FAILED (rv)) {
		printf ("failed to ge a ServiceManager \n");
		return;
	}

	nsCOMPtr<nsIIOService> ioservice;
	rv = mgr->GetServiceByContractID ("@mozilla.org/network/io-service;1",
			NS_GET_IID (nsIIOService), getter_AddRefs (ioservice));

	if (NS_FAILED (rv)) {
		printf ("failed to get a IOService \n");
		return;
	}

	nsEmbedCString url;
	url = this->uri;

	nsCOMPtr<nsIURI> uri;
	rv = ioservice->NewURI (url, nsnull, nsnull, getter_AddRefs (uri));

	ioservice->NewChannelFromURI (uri, getter_AddRefs (this->channel));

	nsCOMPtr<nsIHttpChannel> httpchannel = do_QueryInterface (channel);
	if (!httpchannel)
		return;

	nsEmbedCString meth;
	meth = this->method;
	httpchannel->SetRequestMethod (meth);
}

void
BrowserHttpRequest::Abort ()
{
	channel->Cancel (NS_BINDING_ABORTED);
}

SyncBrowserHttpResponse *
BrowserHttpRequest::GetResponse ()
{
	nsresult rs = NS_OK;
	nsCOMPtr<nsIInputStream> input;
	rs = channel->Open (getter_AddRefs (input));
	if (NS_FAILED (rs))
		return NULL;

	return new SyncBrowserHttpResponse (channel, input);
}

AsyncBrowserHttpResponse *
BrowserHttpRequest::GetAsyncResponse ()
{
	printf ("GetAsyncResponse is not implemented \n");
	AsyncBrowserHttpResponse *response = new AsyncBrowserHttpResponse (channel);
	// TODO: make AsyncBrowserHttpResponse implement nsIStreamListener
	return response;
}

void
BrowserHttpRequest::SetHttpHeader (const char *name, const char *value)
{
	nsCOMPtr<nsIHttpChannel> httpchannel = do_QueryInterface (channel);
	if (!httpchannel)
		return;

	nsEmbedCString nsname, nsvalue;
	nsname = name;
	nsvalue = value;

	httpchannel->SetRequestHeader (nsname, nsvalue, false);
}

void
BrowserHttpRequest::SetBody (const char *body, int size)
{
	nsCOMPtr<nsIUploadChannel> upload = do_QueryInterface (channel);
	if (!upload)
		return;

	nsEmbedCString type;

	nsCOMPtr<nsIStorageStream> storage;
	nsresult rv = NS_NewStorageStream (2048, PR_UINT32_MAX, getter_AddRefs (storage));


	nsCOMPtr<nsIOutputStream> output;
	storage->GetOutputStream (0, getter_AddRefs (output));

	PRUint32 written;
	output->Write (body, size, &written);

    nsCOMPtr<nsIInputStream> input;
    rv = storage->NewInputStream (0, getter_AddRefs (input));

	upload->SetUploadStream (input, type, -1);
}

BrowserHttpRequest *
browser_http_request_new (const char *method, const char *uri)
{
	return new BrowserHttpRequest (method, uri);
}

void
browser_http_request_abort (BrowserHttpRequest *request)
{
	request->Abort ();
}

void
browser_http_request_set_header (BrowserHttpRequest *request, const char *name, const char *value)
{
	request->SetHttpHeader (name, value);
}

void
browser_http_request_set_body (BrowserHttpRequest *request, const char *body, int size)
{
	request->SetBody (body, size);
}

void
browser_http_request_destroy (BrowserHttpRequest *request)
{
	delete request;
}

SyncBrowserHttpResponse *
browser_http_request_get_response (BrowserHttpRequest *request)
{
	return request->GetResponse ();
}

void
browser_http_response_destroy (BrowserHttpResponse *response)
{
	delete response;
}

void *
browser_http_sync_response_read (SyncBrowserHttpResponse *response, int *size)
{
	return response->Read (size);
}

void
browser_http_test ()
{
	BrowserHttpRequest *req = new BrowserHttpRequest ("GET", "http://evain.net/gdb.txt");
	SyncBrowserHttpResponse *response = req->GetResponse ();

	int len;
	void *data = response->Read (&len);

	char *text = g_strndup ((char *) data, len);

	printf ("response: \n%s", text);

	g_free (data);
	g_free (text);

	delete response;
	delete req;
}
