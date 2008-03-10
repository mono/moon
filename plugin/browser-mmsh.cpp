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
#include <asf/asf.h>

#define LE_16(val) (GINT16_FROM_LE (*((u_int16_t*)(val))))
#define LE_64(val) (GINT64_FROM_LE (*((u_int64_t*)(val))))

#define MMS_DATA     0x44
#define MMS_HEADER   0x48
#define MMS_METADATA 0x4D
#define ASF_DEFAULT_PACKET_SIZE 2888

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
	this->finisher (this, this->context);
	return NS_OK;
}

static void
asf_header_parse (char *asf_header, int size, int64_t *file_size, uint16_t *asf_packet_size)
{
	ASFBufferSource *asf_src = new ASFBufferSource (NULL, asf_header, size);
	ASFParser *parser = new ASFParser (asf_src, NULL);
	asf_src->parser = parser;
	if (!parser->ReadHeader ()) {
		g_print ("Error reading header\n");
		*file_size = 0;
		*asf_packet_size = ASF_DEFAULT_PACKET_SIZE;
		return;
	}
	
        asf_file_properties *properties = parser->GetFileProperties ();

	delete asf_src;

	//g_print ("FILE_SIZE: %d\n", properties->file_size);
	//g_print ("MAX: %d\n", properties->max_packet_size);
	//g_print ("MIN: %d\n", properties->min_packet_size);
	 
	if (properties->max_packet_size == properties->min_packet_size)
		*asf_packet_size = properties->max_packet_size;
	else
		*asf_packet_size = ASF_DEFAULT_PACKET_SIZE;

	*file_size = properties->file_size;
}


void
BrowserMmshResponse::Abort ()
{
	this->channel->Cancel (NS_BINDING_ABORTED);
}


void
AsyncBrowserMmshResponse::MmsMetadataParse (int packet_size, const char *data)
{
	int offset = 0;
	int i = 0;
	char **metadata = g_strsplit ((char*) data, ",", 0);
	if (metadata[0]) {
		g_print ("playlist-gen-id:%s\n", metadata[0]);
		offset += strlen (metadata[0]);
	}
	if (metadata[1]) {
		g_print ("broadcast-id:%s\n", metadata[1]);
		offset += strlen (metadata[1]) + 1;
	}
	if (metadata[2]) {
		g_print ("features:%s", metadata[2]);
		offset += strlen (metadata[2]) + 1;
	}
	for (i = 3; metadata[i]; i++) {
		g_print (",%s", metadata[i]);
		offset += strlen (metadata[i]) + 1;
	}
	g_print ("\n");
	g_strfreev (metadata);

	if (packet_size > offset) {
		int cdl_index;
		char **cdls = g_strsplit (data + offset + 1, "\r\n", 0);

		for (cdl_index = 0; cdls[cdl_index]; cdl_index++) {
			if (*cdls[cdl_index] == '$') {
				/* It's not a CDL but the real packet */
				break;
			}
			char **items = g_strsplit (cdls[cdl_index], ",", 0);
			int i;
			for (i = 0; items[i]; i += 5) {
				g_print ("Item %d:%s", i, items[i + 1]);
				g_print (" value:%s\n", items[i + 4]);
				if (strcmp (items[i+1], "WMS_CONTENT_DESCRIPTION_PLAYLIST_ENTRY_DURATION") == 0) {
					notify_size = atoll (items[i + 4]);
				}
				if (strcmp (items[i+1], "WMS_CONTENT_DESCRIPTION_PLAYLIST_ENTRY_URL") == 0) {
					notify_name = g_strdup (items[i + 4]);
				}
			}
			g_strfreev (items);
			/*if (notifier && notify_size)
				notifier (this, this->context, notify_name, notify_size);*/
			
		}
		g_strfreev (cdls);
	}
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
			MmsMetadataParse (packet_size, mms_packet+8);
		} else if (type == MMS_HEADER) {
				int64_t file_size;
				asf_header_parse (mms_packet+8, packet_size - 8, &file_size, &asf_packet_size);
				if (file_size && notifier)
					notifier (this, this->context, NULL, file_size);
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
				      AsyncMmshResponseNotifierHandler notifier,
				      AsyncMmshResponseFinishedHandler finisher, gpointer context)
{
	nsresult rs = NS_OK;
	AsyncBrowserMmshResponse *response;

	response = new AsyncBrowserMmshResponse (channel, reader, notifier, finisher, context);
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
