/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * plugin-downloader.cpp: Moonlight plugin download routines.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "plugin-downloader.h"
#include "browser-bridge.h"
#include "npstream-request.h"

#define d(x)

bool downloader_shutdown = false;

guint32
plugin_downloader_started (DownloaderResponse *response, gpointer state)
{
	d(printf ("plugin_downloader_started (%p, %p).\n", response, state));
	PluginDownloader *pd = (PluginDownloader *)state;

	if (pd != NULL) {
		pd->setResponse (response);
		pd->Started ();
	}

	return DOWNLOADER_OK;
}

guint32
plugin_downloader_available (DownloaderResponse *response, gpointer state, char *buffer, guint32 length)
{
	d(printf ("plugin_downloader_available (%p, %p, %p, %u).\n", response, state, buffer, length));
	PluginDownloader *pd = (PluginDownloader *)state;

	if (pd != NULL) {
		return pd->Read (buffer, length);
	}

	return DOWNLOADER_ERR;
}

guint32
plugin_downloader_finished (DownloaderResponse *response, gpointer state, bool success, gpointer data, const char *uri)
{
	d(printf ("plugin_downloader_finished (%p, %p).\n", response, state));
	PluginDownloader *pd = (PluginDownloader *)state;

	if (pd != NULL) {
		pd->Finished (success, data, uri);
	}

	return DOWNLOADER_OK;
}

static gpointer
plugin_downloader_create_state (Downloader *dl)
{
	PluginDownloader *state = new PluginDownloader (dl);

	d (printf ("plugin_downloader_create_state (%p)\n", state));

	return state;
}

static void
plugin_downloader_destroy_state (gpointer data)
{
	d (printf ("plugin_downloader_destroy_state (%p)\n", data));
	
	delete (PluginDownloader *) data;
}

static void
plugin_downloader_open (const char *verb, const char *uri, bool streaming, gpointer state)
{
	d (printf ("plugin_downloader_open (%s, %s, %p)\n", verb, uri, state));
	
	PluginDownloader *pd = (PluginDownloader *) state;

	pd->Open (verb, uri, streaming);
}

static void
plugin_downloader_send (gpointer state)
{
	d (printf ("plugin_downloader_send (%p)\n", state));
	
	PluginDownloader *pd = (PluginDownloader *) state;

	pd->Send ();
}

static void
plugin_downloader_set_body (gpointer state, void *body, guint32 length)
{
	d (printf ("plugin_downloader_set_body (%p)\n", state));
	
	PluginDownloader *pd = (PluginDownloader *) state;

	pd->SetBody (body, length);
}

static void
plugin_downloader_set_header (gpointer state, const char *header, const char *value)
{
	d (printf ("plugin_downloader_set_header (%p)\n", state));
	
	PluginDownloader *pd = (PluginDownloader *) state;

	pd->SetHttpHeader (header, value);
}

static void
plugin_downloader_abort (gpointer state)
{
	d (printf ("plugin_downloader_abort (%p)\n", state));
	
	if (downloader_shutdown)
		return;

	PluginDownloader *pd = (PluginDownloader *) state;

	pd->Abort ();
}

static void *
plugin_downloader_create_webrequest (const char *method, const char *uri, gpointer context)
{
	if (!context)
		return NULL;
	
	PluginInstance *instance = (PluginInstance *) context;
	BrowserBridge *bridge = instance->GetBridge ();
	
	return bridge ? bridge->CreateDownloaderRequest (method, uri) : NULL;
}

PluginDownloader::PluginDownloader (Downloader *dl)
{
	d (printf ("PluginDownloader::PluginDownloader (), this: %p, dl: %p\n", this, dl));
	this->dl = dl;
	this->uri = NULL;
	this->verb = NULL;
	this->response = NULL;
	this->request = NULL;
	this->finished = false;
}

PluginDownloader::~PluginDownloader ()
{
	d (printf ("PluginDownloader::~PluginDownloader (), this: %p, dl: %p\n", this, dl));

	Abort ();

	g_free (verb);
	g_free (uri);
	dl = NULL;
}

void
PluginDownloader::Abort ()
{
	d (printf ("PluginDownloader::Abort (), this: %p, dl: %p, finished: %i, request: %p, response: %p\n", this, dl, finished, request, response));

	if (finished)
		return;

	if (this->request) {
		this->request->Abort ();
		delete request;
		this->request = NULL;
	}
	if (this->response) {
		this->response->Abort ();
		this->response->unref ();
		this->response = NULL;
	}
}

void
PluginDownloader::Open (const char *verb, const char *uri, bool streaming)
{
	d (printf ("PluginDownloader::Open (), this: %p, dl: %p\n", this, dl));
	
	//delete this->bdl;
	g_free (this->uri);
	g_free (this->verb);
	
	this->verb = g_strdup (verb);
	this->uri = g_strdup (uri);
	
	if (streaming) {
		BrowserBridge *bridge = GetPlugin ()->GetBridge ();
		if (bridge)
			this->request = bridge->CreateDownloaderRequest (this->verb, this->uri);
	} else {
		this->request = new NPStreamRequest (this->verb, this->uri, GetPlugin ());
	}
}

void
PluginDownloader::Send ()
{
	d (printf ("PluginDownloader::Send (), this: %p, dl: %p\n", this, dl));
	
	this->offset = 0;
	this->request->GetResponse (plugin_downloader_started, plugin_downloader_available, plugin_downloader_finished, this);
}

void
PluginDownloader::Started ()
{
	d (printf ("PluginDownloader::Started (), this: %p, dl: %p\n", this, dl));
}

void 
PluginDownloader::setResponse (DownloaderResponse *response)
{
	d (printf ("PluginDownloader::setResponse (%p)\n", response));
	
	if (this->response == response)
		return;
		
	if (this->response != NULL)
		this->response->unref ();
	this->response = response;
	if (this->response != NULL)
		this->response->ref ();
}

DownloaderRequest *
PluginDownloader::getRequest ()
{
	return this->request;
}

guint32
PluginDownloader::Read (char *buffer, guint32 length)
{
	d (printf ("PluginDownloader::Read (), this: %p, dl: %p\n", this, dl));
	
	if (dl != NULL) {
		dl->Write (buffer, this->offset, length);
		this->offset += length;
		return DOWNLOADER_OK;
	}

	return DOWNLOADER_ERR;
}

void
PluginDownloader::Finished (bool success, gpointer data, const char *uri)
{
	d (printf ("PluginDownloader::Finished (), this: %p, dl: %p\n", this, dl));

	finished = true;

	if (dl != NULL) {
		if (success) {
			dl->NotifySize (this->offset);
			dl->SetFilename ((const char *)data);
			dl->NotifyFinished (uri);
		} else {
			dl->NotifyFailed ("download failed");
		}
	}
}
	
void
PluginDownloader::SetHttpHeader (const char *header, const char *value)
{
	d (printf ("PluginDownloader::SetHttpHeader (), this: %p, dl: %p\n", this, dl));
	
	if (request != NULL)
		request->SetHttpHeader (header, value);
}

void
PluginDownloader::SetBody (void *body, guint32 length)
{
	d (printf ("PluginDownloader::SetBody (), this: %p, dl: %p\n", this, dl));
	
	if (request != NULL)
		request->SetBody (body, length);
}

PluginInstance *
PluginDownloader::GetPlugin ()
{
	PluginInstance *instance = NULL;

	if (dl && dl->GetContext ()) {
		// Get the context from the downloader.
		instance = (PluginInstance *) dl->GetContext ();
	} else if (plugin_instances && plugin_instances->data) {
		// TODO: Review if we really should allowing download with the first plugin.
		NPP_t *plugin = (NPP_t *) plugin_instances->data;
		if (plugin == NULL || plugin->pdata == NULL)
			return NULL;
		instance = (PluginInstance*)plugin->pdata;
	}
        
	return instance;
}

void
downloader_initialize (void)
{
	downloader_shutdown = false;
	
	Downloader::SetFunctions (
		&plugin_downloader_create_state,
		&plugin_downloader_destroy_state,
		&plugin_downloader_open,
		&plugin_downloader_send,
		&plugin_downloader_abort,
		&plugin_downloader_set_header,
		&plugin_downloader_set_body,
		&plugin_downloader_create_webrequest,
		false);
}

void
downloader_destroy (void)
{
	downloader_shutdown = true;
}
