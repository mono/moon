/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * lunar-downloader.cpp:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <config.h>

#include "lunar-downloader.h"
#include "npstream-request.h"

#define d(x)

BrowserBridge *LunarDownloader::bridge = NULL;

LunarDownloader::LunarDownloader (Downloader *dl)
{
	response_header_callback = NULL;
	response_header_context = NULL;
	finished = false;
	aborted = false;
	downloader = dl;
	response = NULL;
	request = NULL;
	verb = NULL;
	uri = NULL;
	offset = 0;
}

LunarDownloader::~LunarDownloader ()
{
	Abort ();
	
	g_free (verb);
	g_free (uri);
}

void
LunarDownloader::Open (const char *verb, const char *uri, bool streaming, bool disable_cache)
{
	this->verb = g_strdup (verb);
	this->uri = g_strdup (uri);
	
	if (LunarDownloader::bridge)
		request = LunarDownloader::bridge->CreateDownloaderRequest (verb, uri, disable_cache);
}

void
LunarDownloader::Abort ()
{
	if (finished || aborted)
		return;
	
	response_header_callback = NULL;
	response_header_context = NULL;
	aborted = true;
	
	if (request) {
		request->Abort ();
		delete request;
		request = NULL;
	}
	
	if (response) {
		response->Abort ();
		response->unref ();
		response = NULL;
	}
}

static guint32
downloader_started (DownloaderResponse *response, gpointer state)
{
	LunarDownloader *ld = (LunarDownloader *) state;
	
	if (ld != NULL) {
		ld->SetResponse (response);
		ld->Started ();
	}
	
	return DOWNLOADER_OK;
}

static guint32
downloader_available (DownloaderResponse *response, gpointer state, char *buffer, guint32 length)
{
	LunarDownloader *ld = (LunarDownloader *) state;
	
	if (ld != NULL)
		return ld->Read (buffer, length);
	
	return DOWNLOADER_ERR;
}

static guint32
downloader_finished (DownloaderResponse *response, gpointer state, bool success, gpointer data, const char *uri)
{
	LunarDownloader *ld = (LunarDownloader *) state;
	
	if (ld != NULL)
		ld->Finished (success, data, uri);
	
	return DOWNLOADER_OK;
}

void
LunarDownloader::Send ()
{
	finished = false;
	offset = 0;
	
	request->GetResponse (downloader_started, downloader_available, downloader_finished, this);
}

guint32
LunarDownloader::Read (char *buffer, guint32 length)
{
	if (!downloader)
		return DOWNLOADER_ERR;
	
	downloader->Write (buffer, offset, length);
	offset += length;
	
	return DOWNLOADER_OK;
}

void
LunarDownloader::Started ()
{
	// no-op
}

void
LunarDownloader::Finished (bool success, gpointer data, const char *uri)
{
	finished = true;
	
	if (!downloader)
		return;
	
	if (success) {
		downloader->NotifySize (offset);
		downloader->SetFilename ((const char *) data);
		downloader->NotifyFinished (uri);
	} else {
		downloader->NotifyFailed ("download failed");
	}
}

void
LunarDownloader::SetHttpHeader (const char *header, const char *value, bool disable_folding)
{
	if (request != NULL)
		request->SetHttpHeader (header, value, disable_folding);
}

void
LunarDownloader::SetBody (void *body, guint32 length)
{
	if (request != NULL)
		request->SetBody (body, length);
}

void
LunarDownloader::SetResponseHeaderCallback (DownloaderResponseHeaderCallback callback, gpointer context)
{
	response_header_callback = NULL;
	
	if (response) {
		response->SetHeaderVisitor (callback, context);
	} else {
		response_header_callback = callback;
		response_header_context = context;
	}
}

void
LunarDownloader::SetResponse (DownloaderResponse *response)
{
	if (this->response == response)
		return;
	
	if (this->response != NULL)
		this->response->unref ();
	
	this->response = response;
	
	if (this->response != NULL) {
		this->response->ref ();
		
		if (response_header_callback != NULL)
			response->SetHeaderVisitor (response_header_callback, response_header_context);
	}
}


static gpointer
downloader_create_state (Downloader *dl)
{
	return new LunarDownloader (dl);
}

static void
downloader_destroy_state (gpointer state)
{
	delete ((LunarDownloader *) state);
}

static void
downloader_open (gpointer state, const char *verb, const char *uri, bool streaming, bool disable_cache)
{
	((LunarDownloader *) state)->Open (verb, uri, streaming, disable_cache);
}

static void
downloader_send (gpointer state)
{
	((LunarDownloader *) state)->Send ();
}

static void
downloader_abort (gpointer state)
{
	((LunarDownloader *) state)->Abort ();
}

static void
downloader_set_header (gpointer state, const char *header, const char *value, bool disable_folding)
{
	((LunarDownloader *) state)->SetHttpHeader (header, value, disable_folding);
}

static void
downloader_set_body (gpointer state, void *body, guint32 length)
{
	((LunarDownloader *) state)->SetBody (body, length);
}

static void *
downloader_create_webrequest (const char *method, const char *uri, gpointer context)
{
	BrowserBridge *bridge = LunarDownloader::GetBridge ();
	
	return bridge ? bridge->CreateDownloaderRequest (method, uri, false) : NULL;
}

static void
downloader_set_response_header_callback (gpointer state, DownloaderResponseHeaderCallback callback, gpointer context)
{
	((LunarDownloader *) state)->SetResponseHeaderCallback (callback, context);
}

static DownloaderResponse *
downloader_get_response (gpointer state)
{
	return ((LunarDownloader *) state)->GetResponse ();
}

void
lunar_downloader_init (void)
{
	Downloader::SetFunctions (downloader_create_state,
				  downloader_destroy_state,
				  downloader_open,
				  downloader_send,
				  downloader_abort,
				  downloader_set_header,
				  downloader_set_body,
				  downloader_create_webrequest,
				  downloader_set_response_header_callback,
				  downloader_get_response);
}

void
lunar_downloader_shutdown (void)
{
	LunarDownloader::SetBridge (NULL);
}
