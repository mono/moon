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
#include "mms-downloader.h"
#include "http-downloader.h"
#include "npstream-downloader.h"

#define d(x)

bool downloader_shutdown = false;

uint32_t
browser_downloader_started (BrowserResponse *response, gpointer state)
{
	BrowserDownloader *downloader = (BrowserDownloader *)state;

	if (downloader != NULL) {
		downloader->SetResponse (response);
		downloader->Started ();
	}

	return DOWNLOADER_OK;
}

uint32_t
browser_downloader_available (BrowserResponse *response, gpointer state, char *buffer, uint32_t length)
{
	BrowserDownloader *downloader = (BrowserDownloader *)state;

	if (downloader != NULL) {
		return downloader->Read (buffer, length);
	}

	return DOWNLOADER_ERR;
}

uint32_t
browser_downloader_finished (BrowserResponse *response, gpointer state)
{
	BrowserDownloader *downloader = (BrowserDownloader *)state;

	if (downloader != NULL) {
		downloader->Finished ();
	}

	return DOWNLOADER_OK;
}

static gpointer
p_downloader_create_state (Downloader *dl)
{
	d (printf ("p_downloader_create_state (%p)\n", dl));

	return new PluginDownloader (dl);
}

static void
p_downloader_destroy_state (gpointer data)
{
	d (printf ("p_downloader_destroy_state (%p)\n", data));
	
	delete (PluginDownloader *) data;
}

static void
p_downloader_open (const char *verb, const char *uri, gpointer state)
{
	d (printf ("p_downloader_open (%s, %s, %p)\n", verb, uri, state));
	
	PluginDownloader *pd = (PluginDownloader *) state;

	pd->Open (verb, uri);
}

static void
p_downloader_send (gpointer state)
{
	d (printf ("p_downloader_send (%p)\n", state));
	
	PluginDownloader *pd = (PluginDownloader *) state;

	pd->Send ();
}

static void
p_downloader_abort (gpointer state)
{
	d (printf ("p_downloader_abort (%p)\n", state));
	
	if (downloader_shutdown)
		return;

	PluginDownloader *pd = (PluginDownloader *) state;

	pd->Abort ();
}

void
downloader_initialize (void)
{
	downloader_shutdown = false;
	downloader_set_functions (
		p_downloader_create_state,
		p_downloader_destroy_state,
		p_downloader_open,
		p_downloader_send,
		p_downloader_abort);
}

void
downloader_destroy (void)
{
	downloader_shutdown = true;
}

void
PluginDownloader::Open (const char *verb, const char *uri)
{
	this->verb = g_strdup (verb);
	if (strncmp (uri, "mms://", 6) == 0) {
		this->uri = g_strdup_printf ("http://%s", uri+6);
		this->bdl = new MmsDownloader (this);
	} else {
#if HTTP_DOWNLOADER
		if (strstr (uri, "://"))
			this->uri = g_strdup (uri);
		else
			this->uri = g_strdup_printf ("%s%s", GetPlugin ()->getSourceLocation (), uri);
		this->bdl = new HttpDownloader (this);
#else
		this->uri = g_strdup (uri);
		this->bdl = new NPStreamDownloader (this);
#endif
	}
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
}

// BrowserDownloader

PluginInstance *
BrowserDownloader::GetPlugin ()
{
        PluginInstance *instance = NULL;

        if (pd && pd->dl && pd->dl->GetContext ()) {
                // Get the context from the downloader.
                instance = (PluginInstance *) pd->dl->GetContext ();
        } else if (plugin_instances && plugin_instances->data) {
                // TODO: Review if we really should allowing download with the first plugin.
                NPP_t *plugin = (NPP_t *) plugin_instances->data;
                if (plugin == NULL || plugin->pdata == NULL)
                        return NULL;
                instance = (PluginInstance*)plugin->pdata;
        }

	return instance;
}
