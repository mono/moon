/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * application.cpp:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include <glib.h>
#include <glib/gstdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "application.h"
#include "runtime.h"
#include "deployment.h"
#include "utils.h"
#include "uri.h"


Application::Application ()
{
	SetObjectType (Type::APPLICATION);
	
	resource_root = NULL;
	
	apply_default_style_cb = NULL;
	apply_style_cb = NULL;
	convert_keyframe_callback = NULL;
	get_resource_cb = NULL;
}

Application::~Application ()
{
	if (resource_root) {
		RemoveDir (resource_root);
		g_free (resource_root);
	}
}

Application*
Application::GetCurrent ()
{
	return Deployment::GetCurrent()->GetCurrentApplication();
}

void
Application::SetCurrent (Application *application)
{
	Deployment::GetCurrent()->SetCurrentApplication (application);
}

void
Application::RegisterCallbacks (ApplyDefaultStyleCallback apply_default_style_cb,
				ApplyStyleCallback apply_style_cb,
				GetResourceCallback get_resource_cb,
				ConvertKeyframeValueCallback convert_keyframe_callback)
{
	this->apply_default_style_cb = apply_default_style_cb;
	this->apply_style_cb = apply_style_cb;
	this->convert_keyframe_callback = convert_keyframe_callback;
	this->get_resource_cb = get_resource_cb;
}

void
Application::ApplyDefaultStyle (FrameworkElement *fwe, ManagedTypeInfo *key)
{
	if (apply_default_style_cb)
		apply_default_style_cb (fwe, key);
}

void
Application::ApplyStyle (FrameworkElement *fwe, Style *style)
{
	if (apply_style_cb)
		apply_style_cb (fwe, style);
}

void
Application::ConvertKeyframeValue (Type::Kind kind, DependencyProperty *property, Value *original, Value *converted)
{
	if (convert_keyframe_callback) {
		convert_keyframe_callback (kind, property, original, converted);
	} else {
		converted = new Value (*original);
	}
}

struct NotifyCtx {
	gpointer user_data;
	NotifyFunc notify_cb;
};

void downloader_abort (gpointer data);
void downloader_progress_changed (EventObject *sender, EventArgs *calldata, gpointer closure);
void downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure);
void downloader_failed (EventObject *sender, EventArgs *calldata, gpointer closure);

void
Application::GetResource (const Uri *uri, NotifyFunc notify_cb, WriteFunc write_cb, DownloaderAccessPolicy policy, Cancellable *cancellable, gpointer user_data)
{
	g_warning ("GetResource");
	if (!uri) {
		g_warning ("Passing a null uri to Application::GetResource");
		return;
	}

	if (get_resource_cb && uri && !uri->isAbsolute) {
		char *url = uri->ToString ();
		ManagedStreamCallbacks stream = get_resource_cb (url);
		g_free (url);
		if (stream.handle) {
			g_warning ("wo oh oh, got a stream");
			if (notify_cb) {
				notify_cb (NotifyStarted, NULL, user_data);
				notify_cb (NotifySize, stream.Length (stream.handle), user_data);
			}

			char buffer [1024];
			if (stream.CanSeek (stream.handle) && stream.Position (stream.handle) != 0)
				stream.Seek (stream.handle, 0, 0);
	
			gint32 nread;
			gint32 offset = 0;
			do {
				nread = stream.Read (stream.handle, buffer, 0, 1024);
				if (write_cb)
					write_cb (buffer, offset, nread, user_data);
				offset += nread;
			} while (nread);

			if (notify_cb)
				notify_cb (NotifyCompleted, NULL, user_data);

			return;
		}
	}	

	//no get_resource_cb or empty stream
	Downloader *downloader;
	Surface *surface = Deployment::GetCurrent ()->GetSurface ();
	if (!(downloader = surface->CreateDownloader ()))
		return;
	
	NotifyCtx *ctx = g_new (NotifyCtx, 1);
	ctx->user_data = user_data;
	ctx->notify_cb = notify_cb;

	if (notify_cb) {
		downloader->AddHandler (Downloader::DownloadProgressChangedEvent, downloader_progress_changed, ctx);
		downloader->AddHandler (Downloader::DownloadFailedEvent, downloader_failed, ctx);
		downloader->AddHandler (Downloader::CompletedEvent, downloader_complete, ctx);
	}

	if (cancellable) {
		cancellable->SetCancelFuncAndData (downloader_abort, downloader);
	}

	if (downloader->Completed ()) {
		if (notify_cb)
			notify_cb (NotifyCompleted, NULL, user_data);
	} else {
		if (!downloader->Started ()) {
			downloader->Open ("GET", (Uri*)uri, policy);
			downloader->SetStreamFunctions (write_cb, NULL, user_data);
			downloader->Send ();
		}
	}
}

void
downloader_abort (gpointer data)
{
	Downloader *dl = (Downloader *) data;
	dl->Abort ();
}

void
downloader_progress_changed (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	NotifyCtx *ctx = (NotifyCtx *) closure;
	Downloader *dl = (Downloader *) sender;
	ctx->notify_cb (NotifyProgressChanged, (gint64) (100 * dl->GetDownloadProgress ()), ctx->user_data);
}

void
downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	NotifyCtx *ctx = (NotifyCtx *) closure;
	ctx->notify_cb (NotifyCompleted, NULL, ctx->user_data);
	g_free (ctx);
}

void
downloader_failed (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	NotifyCtx *ctx = (NotifyCtx *) closure;
	ctx->notify_cb (NotifyFailed, NULL, ctx->user_data);
	g_free (ctx);
}

//compatibility function, act like the old get_resource_cb
gpointer
Application::GetResourceAsBuffer (const Uri *uri, int *size)
{
	gpointer buffer = NULL;

	if (!uri) {
		g_warning ("Passing a null uri to Application::GetResource");
		return NULL;
	}

	if (get_resource_cb && uri && !uri->isAbsolute) {
		char *url = uri->ToString ();
		ManagedStreamCallbacks stream = get_resource_cb (url);
		g_free (url);

		if (stream.handle) {

			*size = stream.Length (stream.handle);
			if (!size)
				return NULL;

			buffer = g_new (char, *size);

			if (stream.CanSeek (stream.handle) && stream.Position (stream.handle) != 0)
				stream.Seek (stream.handle, 0, 0);
	
			stream.Read (stream.handle, buffer, 0, *size);
		}
	}	
	
	return buffer;
}

//FIXME: nuke this!
char *
Application::GetResourceAsPath (const Uri *uri)
{
	char *dirname, *path, *filename, *url;
	unzFile zipfile;
	struct stat st;
	gpointer buf;
	int size;
	int fd;
	
	if (!get_resource_cb || !uri)
		return NULL;
	
	if (!resource_root) {
		// create a root temp directory for our resource files
		if (!(resource_root = CreateTempDir ("moonlight-app")))
			return NULL;
	}
	
	// construct the path name for this resource
	filename = uri->ToString ();
	CanonicalizeFilename (filename, -1, true);
	if (uri->GetQuery () != NULL) {
		char *sc = strchr (filename, ';');
		if (sc)
			*sc = '/';
	}
	
	path = g_build_filename (resource_root, filename, NULL);
	g_free (filename);
	
	if (stat (path, &st) != -1) {
		// path exists, we're done
		return path;
	}
	
	// create the directory for our resource (keeping the relative path intact)
	dirname = g_path_get_dirname (path);
	if (g_mkdir_with_parents (dirname, 0700) == -1 && errno != EEXIST) {
		g_free (dirname);
		g_free (path);
		return NULL;
	}
	
	g_free (dirname);
	
	// now we need to get the resource buffer and dump it to disk
	if (!(buf = GetResourceAsBuffer (uri, &size))) {
		g_free (path);
		return NULL;
	}
	
	
	// create and save the buffer to disk
	if ((fd = open (path, O_WRONLY | O_CREAT | O_EXCL, 0600)) == -1) {
		g_free (path);
		g_free (buf);
		return NULL;
	}
	
	if (write_all (fd, (char *) buf, (size_t) size) == -1) {
		close (fd);
		g_unlink (path);
		g_free (path);
		g_free (buf);
	}
	
	g_free (buf);
	close (fd);
	
	// check to see if the resource is zipped
	if (!(zipfile = unzOpen (path))) {
		// nope, not zipped...
		return path;
	}
	
	// create a directory to contain our unzipped content
	dirname = g_strdup_printf ("%s.XXXXXX", path);
	if (!CreateTempDir (dirname)) {
		unzClose (zipfile);
		g_free (dirname);
		g_unlink (path);
		g_free (path);
		return NULL;
	}
	
	// unzip the contents
	if (!ExtractAll (zipfile, dirname, true)) {
		RemoveDir (dirname);
		unzClose (zipfile);
		g_free (dirname);
		g_unlink (path);
		g_free (path);
		return NULL;
	}
	
	unzClose (zipfile);
	g_unlink (path);
	
	if (rename (dirname, path) == -1) {
		RemoveDir (dirname);
		g_free (dirname);
		g_free (path);
		return NULL;
	}
	
	g_free (dirname);
	
	return path;
}
