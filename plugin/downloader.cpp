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
	}

	~PluginDownloader ()
	{
		g_free (verb);
		g_free (uri);
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
	// This is a hack: we need the p_downloader_create_state to provide us
	// with the pointer to this plugin.    Currently we do not track this
	// information, but we will
	//
	// At this point, we merely steal the first plugin to do this,
	// but that is wrong (if that instance is killed, its download
	// will not complete
	//

	if (plugin_instances->data) {
		StreamNotify *notify = new StreamNotify (StreamNotify::DOWNLOADER, pd->dl);
		NPN_GetURLNotify ((NPP_t *) plugin_instances->data, pd->uri, NULL, notify);
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
