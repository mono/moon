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
#include "plugin-downloader.h"
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
	MemorySource *asf_src = new MemorySource (NULL, asf_header, size, 0);
	ASFParser *parser = new ASFParser (asf_src, NULL);
	asf_src->SetOwner (false);
	asf_src->unref ();
	if (!parser->ReadHeader ()) {
		g_print ("Error reading header\n");
		*file_size = 0;
		*asf_packet_size = ASF_DEFAULT_PACKET_SIZE;
		delete parser;
		return;
	}
	
	asf_file_properties *properties = parser->GetFileProperties ();

	g_print ("FILE_SIZE: %d\n", properties->file_size);
	//g_print ("MAX: %d\n", properties->max_packet_size);
	//g_print ("MIN: %d\n", properties->min_packet_size);

	*asf_packet_size = parser->GetPacketSize ();
	*file_size = properties->file_size;

	delete parser;
}


void
BrowserMmshResponse::Abort ()
{
	this->channel->Cancel (NS_BINDING_ABORTED);
	this->aborted = true;
}

typedef struct {
	const char *key;
	void (*handle_key) (AsyncBrowserMmshResponse *obj, const char *val);
} MetadataParseTable;

static void
parse_features (AsyncBrowserMmshResponse *abmr, const char *val)
{
	g_print ("features: %s\n", val);
}

static MetadataParseTable pragma_table [] = {
	//{ "playlist-gen-id=", parse_playlist_gen_id },
	//{ "broadcast-id=", parse_broadcast_id },
	{ "features=", parse_features },
	{ NULL, NULL }
};

//
// Parses a N,STR from P and returns the position after the STR, and
// the value of STR is copied to RETVAL
//
static const char *
get_sized_item (const char *p, char **retval)
{
	*retval = NULL;
	int n = atoi (p);

	while (*p && *p != ',')
		p++;
	if (*p == 0)
		return p;

	p++;
	*retval = (char *) malloc (n+1);
	if (*retval == NULL)
		return "";

	strncpy (*retval, p, n);
	(*retval) [n] = 0;

	p += n + 1;

	return p;
}

static const char *
get_number (const char *p, int *ret)
{
	*ret = 0;
	*ret = atoi (p);
	
	while (*p && *p != ',')
		p++;
	if (*p == 0)
		return p;

	p++;
	return p;
}

static const char *
get_string (const char *data, const char *end, char **res)
{
	*res = NULL;

	// skip over white space
	while (data < end && *data == ' ')
		data++;

	// if we have quoted text.
	if (*data == '"'){
		const char *p = ++data;
		
		while (data < end && *data && *data != '"')
			data++;

		if (*data == '"'){
			// the quote ended properly, data-1 is the last character
			*res = (char *) g_malloc (data - p + 1);
			(*res) [data-p] = 0;
			strncpy (*res, p, data-p);

			// consume the comma
			while (data < end && *data && *data != ',')
				data++;
			if (*data == ',')
				data++;
		} else {
			// we found a null before the end of the quote
			*res = (char *) g_malloc (data - p + 1);
			(*res) [data-p] = 0;
			strncpy (*res, p, data-p);
		}

		return data;
	}

	const char *q = data;
	
	while (data < end && *data && *data != ',')
		data++;
	*res = (char *) g_malloc (data - q + 1);
	(*res) [data-q] = 0;
	strncpy (*res, q, data-q);

	if (*data == ',')
		data++;
	
	return data;
}

void
AsyncBrowserMmshResponse::MmsMetadataParse (int packet_size, const char *data)
{
	const char *end = data + packet_size;

	while (data < end && *data != 0){
		int j = 0;
		while (data < end && *data == ' ')
			data++;
			
		for (; pragma_table [j].key; j++){
			int l = strlen (pragma_table [j].key);

			if (strncmp (data, pragma_table [j].key, l) == 0){
				char *str;
				
				data = get_string (data + l, end, &str);
				(*pragma_table [j].handle_key) (this, str);
				g_free (str);
				break;
			} 
		}

		// If we did not find a match, just display it
		if (pragma_table [j].key == NULL){
			const char *p = data;
			// skip over key and = 
			while (data < end && *data && *data != '=')
				data++;
			char *res = (char *) g_malloc (data - p + 1);
			char *str;
			res [data-p] = 0;
			strncpy (res, p, data-p);
			
			// get value, and ignore it.
			data = get_string (data, end, &str);

			printf ("KEY: %s=%s\n", res, data);
			g_free (str);
			g_free (res);
		}
	}
	g_print ("\n");

	if (data < end) {
		int cdl_index;
		char **cdls = g_strsplit (data + 1, "\r\n", 0);
		
		for (cdl_index = 0; cdls[cdl_index]; cdl_index++) {
			const char *p = cdls [cdl_index];

			if (*p == '$') {
				/* It's not a CDL but the real packet */
				break;
			}

			while (*p){
				char *item, *strval;
				int type;
				
				p = get_sized_item (p, &item);

				// If we reach the end. 
				if (item == NULL)
					break;
				
				p = get_number (p, &type);
				p = get_sized_item (p, &strval);

				printf ("%s = %s\n", item, strval);
				if (strcmp (item, "WMS_CONTENT_DESCRIPTION_PLAYLIST_ENTRY_DURATION") == 0)
					notify_size = atoll (strval);
				else if (strcmp (item, "WMS_CONTENT_DESCRIPTION_PLAYLIST_ENTRY_URL") == 0)
					notify_size = atoll (strval);
				
				g_free (item);
				g_free (strval);
			}

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
	PluginDownloader *pd = (PluginDownloader*) this->context;

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
				if (file_size == packet_size -8) // file is only header so it's live stream
					pd->seekable = false;
				else
					pd->seekable = true;
				g_print ("Header size %d\n", packet_size - 8);
				pd->header_size = packet_size - 8;
				if (pd->seekable)
					notifier (this, this->context, NULL, file_size);
				if (!pd->ignore_non_data) {
					reader (this, this->context, mms_packet+8, 0, packet_size - 8);
					this->size += packet_size - 8;
				}
		} else if (type == MMS_DATA) {
				char *new_data = (char*)g_malloc0 (asf_packet_size);
				int packet_index = (int)LE_64 (&read_buffer[4]);
				memcpy (new_data, mms_packet+8, packet_size - 8);
				g_print ("Packet id %d\n", packet_index);
				if (pd->seekable)
					reader (this, this->context, new_data, pd->header_size + packet_index* asf_packet_size,
						asf_packet_size);
				else
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

	response = new AsyncBrowserMmshResponse (channel, this->uri, reader, notifier, finisher, context);
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
