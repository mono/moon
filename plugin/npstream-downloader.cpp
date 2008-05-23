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
#include "npstream-downloader.h"

void
npstream_downloader_set_stream_data (Downloader *downloader, NPP npp, NPStream *stream)
{
	PluginDownloader *pd = (PluginDownloader *) downloader->GetDownloaderState ();

	if (pd != NULL) {
		NPStreamDownloader *dl = (NPStreamDownloader *) pd->getBrowserDownloader ();
		
		if (dl != NULL) {
			dl->SetNPP (npp);
			dl->SetStream (stream);
		}
	}
	stream->pdata = pd;
}

void
NPStreamDownloader::Abort ()
{
	if (npp != NULL && stream != NULL) {
		NPN_DestroyStream (npp, stream, NPRES_USER_BREAK);
		stream = NULL;
	}
}

uint32_t
NPStreamDownloader::Read (char *buffer, uint32_t length)
{
	return DOWNLOADER_OK;
}

void
NPStreamDownloader::Started ()
{
}

void
NPStreamDownloader::Finished ()
{
}

void
NPStreamDownloader::Send ()
{
	PluginInstance *instance = GetPlugin ();

	if (instance != NULL) {
		StreamNotify *notify = new StreamNotify (StreamNotify::DOWNLOADER, pd->dl);
		NPError err = NPN_GetURLNotify (instance->getInstance (), pd->GetUri (), NULL, notify);

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
		}
	}
}
