/*
 * browser-http.cpp: Moonlight plugin routines for mms over http requests/responses.
 *
 * Author:
 *   Fernando Herrera (fherrera@novell.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include "browser-mmsh.h"

#define LE_16(val) (GINT16_FROM_LE (*((u_int16_t*)(val))))
#define LE_64(val) (GINT64_FROM_LE (*((u_int64_t*)(val))))

#define MMS_DATA     0x44
#define MMS_HEADER   0x48
#define MMS_METADATA 0x4D

// BrowserMmshResponse

NS_IMPL_ISUPPORTS1 (BrowserMmshResponse, nsIHttpHeaderVisitor)

NS_IMETHODIMP
BrowserMmshResponse::VisitHeader (const nsACString &header, const nsACString &value)
{
        if (handler == NULL)
                return NS_OK;

        const char *name, *val;
        PRUint32 nl, vl;

        nl = NS_CStringGetData (header, &name);
        vl = NS_CStringGetData (value, &val);

        name = g_strndup (name, nl);
        val = g_strndup (val, vl);

        handler (name, val);

        // g_free ((gpointer) name);
        // g_free ((gpointer) val);

    return NS_OK;
}

void
BrowserMmshResponse::VisitHeaders (HttpHeaderHandler handler)
{
        nsCOMPtr<nsIHttpChannel> httpchannel = do_QueryInterface (channel);
        if (!httpchannel)
                return;

        this->handler = handler;

        httpchannel->VisitResponseHeaders (this);

        this->handler = NULL;
}


char *
BrowserMmshResponse::GetStatus (int *code)
{
	nsCOMPtr<nsIHttpChannel> httpchannel = do_QueryInterface (channel);
	if (!httpchannel) {
		*code = 0;
		return NULL;
	}

	nsEmbedCString status_desc;
	PRUint32 status;
	const char *desc;
	int dl;

	httpchannel->GetResponseStatusText (status_desc);

	dl = NS_CStringGetData (status_desc, &desc);

	httpchannel->GetResponseStatus (&status);

	*code = status;
	return g_strndup (desc, dl);
}


// AsyncBrowserHttpResponse

NS_IMPL_ISUPPORTS1 (AsyncBrowserMmshResponse, nsIStreamListener)

NS_IMETHODIMP
AsyncBrowserMmshResponse::OnStartRequest (nsIRequest *request, nsISupports *context)
{
	return NS_OK;
}

NS_IMETHODIMP
AsyncBrowserMmshResponse::OnStopRequest (nsIRequest *request, nsISupports *ctx, nsresult status)
{
	handler (this, this->context);
	return NS_OK;
}

static int
get_asf_packet_size (char *asf_header)
{
	//FIXME: do the real parsing!
	return 2888;
}

void
BrowserMmshResponse::Abort ()
{
	this->channel->Cancel (NS_BINDING_ABORTED);
}



NS_IMETHODIMP
AsyncBrowserMmshResponse::OnDataAvailable (nsIRequest *request, nsISupports *context, nsIInputStream *input, PRUint32 offset, PRUint32 count)
{
	PRUint32 length;
	char *read_buffer;

	if (tmp_buffer) {
		read_buffer = (char *) NS_Realloc (tmp_buffer, tmp_size + count);
		tmp_buffer = NULL;
	} else {
		read_buffer = (char *) NS_Alloc (count);
	}

	input->Read (read_buffer + tmp_size, count, &length);
	length += tmp_size;
	tmp_size = 0;

	while (length > 0) {
		uint8_t type;
	        uint32_t size;
		char *mms_packet;
		uint16_t packet_size;

		if (length < 3) { // Incomplete packet
			tmp_buffer = (char*) NS_Alloc (length);
			memcpy (tmp_buffer, read_buffer, length);
			tmp_size = length;
			return NS_OK;
		}
		
		type = (uint8_t) read_buffer[1];
		size = LE_16 (&read_buffer[2]);

		if (length < size + 4) { // Incomplete Data packet
			tmp_buffer = (char*) NS_Alloc (length);
			memcpy (tmp_buffer, read_buffer, length);
			tmp_size = length;
			return NS_OK;
		}

		mms_packet = read_buffer + 4;
		packet_size = LE_16 (&mms_packet[6]);

		if (type == MMS_METADATA) {
			
			/* Get Metadata and figure out file size and so */
		} else if (type == MMS_HEADER) {
				asf_packet_size = get_asf_packet_size (mms_packet+8);
				reader (this, this->context, mms_packet+8, this->size, packet_size - 8);
				this->size += packet_size - 8;
		} else if (type == MMS_DATA) {
				char *new_data = (char*)g_malloc0 (asf_packet_size);
				memcpy (new_data, mms_packet+8, packet_size - 8);
				reader (this, this->context, new_data, this->size, asf_packet_size);
				this->size += asf_packet_size;
				g_free (new_data);
		}
		length -= size + 4;
		read_buffer = read_buffer + size + 4;

	}

	// FIXME: Free the buffer
	return NS_OK;
}

// BrowserMmshRequest

void
BrowserMmshRequest::CreateChannel ()
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

	printf ("BrowserHttpRequest: %s\n", uri);

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


bool
BrowserMmshRequest::GetAsyncResponse (AsyncMmshResponseDataAvailableHandler reader,
				      AsyncMmshResponseFinishedHandler handler, gpointer context)
{
	nsresult rs = NS_OK;
	AsyncBrowserMmshResponse *response;

	response = new AsyncBrowserMmshResponse (channel, reader, handler, context);
	rs = channel->AsyncOpen (response, (BrowserMmshResponse *) response);
	return !NS_FAILED (rs);
}

void
BrowserMmshRequest::SetHttpHeader (const char *name, const char *value)
{
	nsCOMPtr<nsIHttpChannel> httpchannel = do_QueryInterface (channel);
	if (!httpchannel)
		return;

	nsEmbedCString nsname, nsvalue;
	nsname = name;
	nsvalue = value;

	httpchannel->SetRequestHeader (nsname, nsvalue, true);
}

void
BrowserMmshRequest::SetBody (const char *body, int size)
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
