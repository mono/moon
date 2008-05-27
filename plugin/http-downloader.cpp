/*
 * http-downloader.cpp: Browser Http downloader
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
#include "http-downloader.h"

void
HttpDownloader::Abort ()
{
	if (response != NULL)
		response->Abort ();
}

uint32_t
HttpDownloader::Read (char *buffer, uint32_t length)
{
	pd->dl->Write (buffer, this->offset, length);
	this->offset += length;
	return DOWNLOADER_OK;
}

void
HttpDownloader::Started ()
{
}

void
HttpDownloader::Finished ()
{
	pd->dl->NotifySize (this->offset);
	pd->dl->NotifyFinished (NULL);
}

void
HttpDownloader::Send ()
{
	PluginInstance *instance = GetPlugin ();

	if (instance == NULL || instance->GetBridge () == NULL)
		return;

	BrowserRequest *request = instance->GetBridge ()->CreateBrowserRequest ("GET", pd->GetUri ());

	request->GetResponse (browser_downloader_started, browser_downloader_available, browser_downloader_finished, this);
}
