/*
 * npstream-downloader.cpp: NPStream Browser downloader
 *
 * Author:
 *   Geoff Norton  (gnorton@novell.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include "plugin-downloader.h"
#include "browser-bridge.h"
#include "npstream-request.h"

void
npstream_request_set_stream_data (Downloader *downloader, NPP npp, NPStream *stream)
{
	PluginDownloader *pd = (PluginDownloader *) downloader->GetDownloaderState ();

	if (pd != NULL) {
		NPStreamRequest *req = (NPStreamRequest *) pd->getRequest ();
		
		if (req != NULL) {
			req->SetNPP (npp);
			req->SetStream (stream);
		}
	}
	stream->pdata = pd;
}

void
NPStreamRequest::Abort ()
{
	if (npp != NULL && stream != NULL) {
		NPN_DestroyStream (npp, stream, NPRES_USER_BREAK);
		stream = NULL;
	}
}

bool
NPStreamRequest::GetResponse (BrowserResponseStartedHandler started, BrowserResponseDataAvailableHandler available, BrowserResponseFinishedHandler finished, gpointer context)
{
	PluginDownloader *pd = (PluginDownloader *) context;

	if (instance != NULL) {
		StreamNotify *notify = new StreamNotify (StreamNotify::DOWNLOADER, pd->dl);
		NPError err = NPN_GetURLNotify (instance->getInstance (), uri, NULL, notify);

		if (err != NPERR_NO_ERROR) {
			const char *msg;
			
			switch (err) {
			case NPERR_GENERIC_ERROR:
				msg = "generic error";
				break;
			case NPERR_OUT_OF_MEMORY_ERROR:
				msg = "out of memory";
				break;
			case NPERR_INVALID_URL:
				msg = "invalid url requested";
				break;
			case NPERR_FILE_NOT_FOUND:
				msg = "file not found";
				break;
			default:
				msg = "unknown error";
				break;
			}
			
			pd->dl->NotifyFailed (msg);
			return false;
		}
		return true;
	}
	return false;
}

void
NPStreamRequest::SetHttpHeader (const char *name, const char *value)
{
	g_warning ("NPStream does not suppoert SetHttpHeader");
}

void
NPStreamRequest::SetBody (void *body, int length)
{
	g_warning ("NPStream does not suppoert SetBody");
}
