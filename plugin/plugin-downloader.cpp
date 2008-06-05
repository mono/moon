/*
 * plugin-downloader.cpp: Moonlight plugin download routines.
 *
 * Author:
 *   Everaldo Canuto (everaldo@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include "plugin-downloader.h"
#include "browser-bridge.h"
#include "npstream-request.h"

#define d(x)

bool downloader_shutdown = false;

uint32_t
plugin_downloader_started (BrowserResponse *response, gpointer state)
{
	d(printf ("plugin_downloader_started (%p, %p).\n", response, state));
	PluginDownloader *pd = (PluginDownloader *)state;

	if (pd != NULL) {
		pd->setResponse (response);
		pd->Started ();
	}

	return DOWNLOADER_OK;
}

uint32_t
plugin_downloader_available (BrowserResponse *response, gpointer state, char *buffer, uint32_t length)
{
	d(printf ("plugin_downloader_available (%p, %p, %p, %u).\n", response, state, buffer, length));
	PluginDownloader *pd = (PluginDownloader *)state;

	if (pd != NULL) {
		return pd->Read (buffer, length);
	}

	return DOWNLOADER_ERR;
}

uint32_t
plugin_downloader_finished (BrowserResponse *response, gpointer state, gpointer data)
{
	d(printf ("plugin_downloader_finished (%p, %p).\n", response, state));
	PluginDownloader *pd = (PluginDownloader *)state;

	if (pd != NULL) {
		pd->Finished (data);
	}

	return DOWNLOADER_OK;
}

static gpointer
plugin_downloader_create_state (Downloader *dl)
{
	d (printf ("plugin_downloader_create_state (%p)\n", dl));

	return new PluginDownloader (dl);
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
plugin_downloader_set_body (gpointer state, void *body, uint32_t length)
{
	d (printf ("plugin_downloader_send (%p)\n", state));
	
	PluginDownloader *pd = (PluginDownloader *) state;

	pd->SetBody (body, length);
}

static void
plugin_downloader_set_header (gpointer state, const char *header, const char *value)
{
	d (printf ("plugin_downloader_send (%p)\n", state));
	
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

PluginDownloader::PluginDownloader (Downloader *dl)
{
	d (printf ("PluginDownloader::PluginDownloader (), dl: %p\n", dl));
	this->dl = dl;
	this->uri = NULL;
	this->verb = NULL;
	this->aborted = false;
	this->response = NULL;
	this->request = NULL;
}

PluginDownloader::~PluginDownloader ()
{
	d (printf ("PluginDownloader::~PluginDownloader (), dl: %p, bdl: %p\n", dl, bdl));
	g_free (verb);
	g_free (uri);
	dl = NULL;
}

void
PluginDownloader::Abort ()
{
	if (aborted)
		return;

	aborted = true;

	if (this->request)
		this->request->Abort ();
	if (this->response)
		this->response->Abort ();
}

void
PluginDownloader::Open (const char *verb, const char *uri, bool streaming)
{
	//delete this->bdl;
	g_free (this->uri);
	g_free (this->verb);
	
	this->verb = g_strdup (verb);
	this->uri = g_strdup (uri);
	
	if (streaming) {
		this->request = GetPlugin ()->GetBridge ()->CreateBrowserRequest ("GET", this->uri);
	} else {
		this->request = new NPStreamRequest ("GET", this->uri, GetPlugin ());
	}
}

void
PluginDownloader::Send ()
{
	this->offset = 0;
	this->request->GetResponse (plugin_downloader_started, plugin_downloader_available, plugin_downloader_finished, this);
}

void
PluginDownloader::Started ()
{
}

uint32_t
PluginDownloader::Read (char *buffer, uint32_t length)
{
	dl->Write (buffer, this->offset, length);
	this->offset += length;
	return DOWNLOADER_OK;
}

void
PluginDownloader::Finished (gpointer data)
{
	if (!aborted) {
		dl->NotifySize (this->offset);
		dl->NotifyFinished ((const char *)data);
	}
}
	
void
PluginDownloader::SetHttpHeader (const char *header, const char *value)
{
	if (request != NULL)
		request->SetHttpHeader (header, value);
}

void
PluginDownloader::SetBody (void *body, uint32_t length)
{
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
	downloader_set_functions (
		plugin_downloader_create_state,
		plugin_downloader_destroy_state,
		plugin_downloader_open,
		plugin_downloader_send,
		plugin_downloader_abort,
		plugin_downloader_set_header,
		plugin_downloader_set_body);
}

void
downloader_destroy (void)
{
	downloader_shutdown = true;
}
