/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * pal.cpp
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include "config.h"

#include <glib.h>
#include <glib/gstdio.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "deployment.h"
#include "downloader.h"
#include "runtime.h"
#include "pal.h"

void
MoonWindowingSystem::SetWindowlessCtor (MoonWindowlessCtor ctor)
{
	windowless_ctor = ctor;
}

MoonWindow *
MoonWindowingSystem::CreateWindowless (int width, int height, PluginInstance *forPlugin)
{
	// call into the plugin to create the windowless
	if (windowless_ctor) {
		MoonWindow *window = windowless_ctor (width, height, forPlugin);
		return window;
	}
	else {
		g_warning ("Windowless mode only works in the plugin");
		return NULL;
	}
}


MoonInstallerService::MoonInstallerService ()
{
	downloader = NULL;
	completed = NULL;
	user_data = NULL;
}

MoonInstallerService::~MoonInstallerService ()
{
	CloseDownloader (true);
}

void
MoonInstallerService::CloseDownloader (bool abort)
{
	if (downloader) {
		g_byte_array_free (xap, true);
		
		if (abort)
			downloader->Abort ();
		
		downloader->unref ();
		g_free (path);
		g_free (tmp);
		
		downloader = NULL;
		completed = NULL;
		user_data = NULL;
		path = NULL;
		tmp = NULL;
		xap = NULL;
	}
}

void
MoonInstallerService::UpdaterNotifySize (gint64 size)
{
	g_byte_array_set_size (xap, (guint) size);
}

void
MoonInstallerService::downloader_notify_size (gint64 size, gpointer user_data)
{
	((MoonInstallerService *) user_data)->UpdaterNotifySize (size);
}

void
MoonInstallerService::UpdaterWrite (void *buf, gint32 offset, gint32 n)
{
	memcpy (((char *) xap->data) + offset, buf, n);
}

void
MoonInstallerService::downloader_write (void *buf, gint32 offset, gint32 n, gpointer user_data)
{
	((MoonInstallerService *) user_data)->UpdaterWrite (buf, offset, n);
}

void
MoonInstallerService::UpdaterCompleted ()
{
	int err = 0;
	FILE *fp;
	
	if ((fp = fopen (tmp, "wb"))) {
		// write to the temporary file
		if (fwrite (xap->data, 1, xap->len, fp) == xap->len)
			err = ferror (fp);
		fclose (fp);
		
		if (err == 0) {
			// rename the temp file to the actual file
			if (g_rename (tmp, path) == -1)
				err = errno;
		}
	} else {
		err = errno;
	}
	
	completed (err == 0, err ? g_strerror (err) : NULL, user_data);
	
	CloseDownloader (false);
}

void
MoonInstallerService::downloader_completed (EventObject *sender, EventArgs *args, gpointer user_data)
{
	((MoonInstallerService *) user_data)->UpdaterCompleted ();
}

void
MoonInstallerService::UpdaterFailed ()
{
	completed (false, downloader->GetFailedMessage (), user_data);
	
	CloseDownloader (false);
}

void
MoonInstallerService::downloader_failed (EventObject *sender, EventArgs *args, gpointer user_data)
{
	((MoonInstallerService *) user_data)->UpdaterFailed ();
}

void
MoonInstallerService::CheckAndDownloadUpdateAsync (Deployment *deployment, UpdateCompletedCallback completed, gpointer user_data)
{
	char *uri;
	
	if (downloader)
		return;
	
	uri = GetUpdateUri (deployment);
	path = GetXapFilename (deployment);
	tmp = GetTmpFilename (deployment);
	
	downloader = deployment->GetSurface ()->CreateDownloader ();
	downloader->AddHandler (Downloader::DownloadFailedEvent, downloader_failed, this);
	downloader->AddHandler (Downloader::CompletedEvent, downloader_completed, this);
	downloader->SetStreamFunctions (downloader_write, downloader_notify_size, this);
	downloader->Open ("GET", uri, XamlPolicy);
	xap = g_byte_array_new ();
	downloader->Send ();
	g_free (uri);
	
	this->completed = completed;
	this->user_data = user_data;
}
