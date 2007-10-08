/*
 * downloader.cpp: Moonlight plugin download routines.
 *
 * Author:
 *   Everaldo Canuto (everaldo@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include "moonlight.h"
#include "runtime.h"
#include "downloader.h"
#include "plugin.h"

class PluginDownloader {
public:
	PluginDownloader (Downloader *dl)
	{
		this->dl = dl;
		this->verb = NULL;
		this->uri = NULL;
		//base_ref (dl);
	}

	~PluginDownloader ()
	{
		g_free (verb);
		g_free (uri);
		//base_unref (dl);
	}

	Downloader *dl;
	char *uri;
	char *verb;
};

static gpointer
p_downloader_create_state (Downloader *dl)
{
  //	DEBUGMSG ("downloader_create_state");
	return new PluginDownloader (dl);
}

static void
p_downloader_destroy_state (gpointer data)
{
	delete (PluginDownloader *) data;
}

static void
p_downloader_open (const char *verb, const char *uri, gpointer state)
{
	PluginDownloader *pd = (PluginDownloader *) state;

	g_free (pd->verb);
	g_free (pd->uri);

	pd->verb = g_strdup (verb);
	pd->uri = g_strdup (uri);
}

static void
p_downloader_send (gpointer state)
{
	PluginDownloader *pd = (PluginDownloader *) state;

	//	fprintf (stderr, "PluginDownloaderSend: Starting downloader again for (%s %s)\n", pd->verb, pd->uri);
	//

	NPP_t* plugin = NULL;
	if (pd && pd->dl && pd->dl->GetContext ()) {
		// Get the context from the downloader.
		plugin = ((PluginInstance*) pd->dl->GetContext ())->getInstance ();
	} else if (plugin_instances && plugin_instances->data) {
		// TODO: Review if we really should allowing download with the first plugin.
		plugin = (NPP_t*) plugin_instances->data;
		printf ("DOWNLOADING WITH FIRST PLUGIN (%p), (Downloader->id: %i): %s\n", plugin, pd->dl->id, pd->uri);
	}
	if (plugin && pd) {
		StreamNotify *notify = new StreamNotify (StreamNotify::DOWNLOADER, pd->dl);
		NPN_GetURLNotify (plugin, pd->uri, NULL, notify);
	}

	//
	// TODO:
	//
	//   Write the data to the stream, we should reconsider the idea of
	//   keeping everything in memory as whole documents and zip files
	//   can be transfered (see the XPS reader for silverlight):
	//              http://tinyurl.com/2n55mp
	//
	//   
}

static void
p_downloader_abort (gpointer state)
{
	fprintf (stderr, "moonlight-plugin: implement downloader abort\n");
	DEBUGMSG ("downloader_abort");
}

void
downloader_initialize (void)
{
	downloader_set_functions (
			p_downloader_create_state,
			p_downloader_destroy_state,
			p_downloader_open,
			p_downloader_send,
			p_downloader_abort);
}

void 
downloader_destroy ()
{
}
