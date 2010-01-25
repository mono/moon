/*
 * npstream-downloader.cpp: NPStream Browser downloader
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <config.h>
#include "plugin-downloader.h"
#include "browser-bridge.h"
#include "npstream-request.h"

void
NPStreamRequest::SetStreamData (Downloader *downloader, NPP npp, NPStream *stream)
{
	PluginDownloader *pd = (PluginDownloader *) downloader->GetDownloaderState ();

	if (pd != NULL) {
		NPStreamRequest *req = (NPStreamRequest *) pd->getRequest ();
		
		if (pd->IsAborted ()) {
			MOON_NPN_DestroyStream (npp, stream, NPRES_USER_BREAK);
		} else if (req != NULL) {
			req->stream = stream;
		}
	}
	stream->pdata = pd;
}

const bool
NPStreamRequest::IsAborted ()
{
	return this->aborted;
}

void
NPStreamRequest::StreamDestroyed ()
{
	stream = NULL;
}

void
NPStreamRequest::Abort ()
{
	if (instance != NULL && stream != NULL) {
		MOON_NPN_DestroyStream (instance->GetInstance (), stream, NPRES_USER_BREAK);
		stream = NULL;
	}
}

bool
NPStreamRequest::GetResponse (DownloaderResponseStartedHandler started, DownloaderResponseDataAvailableHandler available, DownloaderResponseFinishedHandler finished, gpointer context)
{
	PluginDownloader *pd = (PluginDownloader *) context;

	if (instance != NULL) {
		StreamNotify *notify = new StreamNotify (StreamNotify::DOWNLOADER, pd->dl);
		NPError err = MOON_NPN_GetURLNotify (instance->GetInstance (), uri, NULL, notify);

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
	g_warning ("NPStream does not support SetHttpHeader");
}

void
NPStreamRequest::SetBody (void *body, int length)
{
	g_warning ("NPStream does not support SetBody");
}
