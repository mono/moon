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

#include <glib/gstdio.h>
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
	g_free (resource_root);
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
	WriteFunc write_cb;
};

static void downloader_abort (gpointer data, void *ctx);
static void downloader_progress_changed (EventObject *sender, EventArgs *calldata, gpointer closure);
static void downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure);
static void downloader_failed (EventObject *sender, EventArgs *calldata, gpointer closure);
static void downloader_write (void *data, gint32 offset, gint32 n, void *closure);
static void downloader_notify_size (gint64 size, gpointer closure);

void
Application::GetResource (const char *resourceBase, const Uri *uri,
			  NotifyFunc notify_cb, WriteFunc write_cb,
			  DownloaderAccessPolicy policy,
			  Cancellable *cancellable, gpointer user_data)
{
	if (!uri) {
		g_warning ("Passing a null uri to Application::GetResource");
		return;
	}

	if (get_resource_cb && uri && !uri->isAbsolute) {
		int uriflags = 0;
		if (GetSurface () && GetSurface ()->GetRelaxedMediaMode ())
			uriflags |= UriShowFileScheme;
		char *url = uri->ToString ((UriToStringFlags)uriflags);
		ManagedStreamCallbacks stream = get_resource_cb (resourceBase, url);
		g_free (url);
		
		if (stream.handle) {
			if (notify_cb) {
				notify_cb (NotifyStarted, NULL, user_data);
				notify_cb (NotifySize, stream.Length (stream.handle), user_data);
			}
			
			if (write_cb) {
				char buf[4096];
				int offset = 0;
				int nread;
				
				if (stream.CanSeek (stream.handle))
					stream.Seek (stream.handle, 0, 0);
				
				do {
					if ((nread = stream.Read (stream.handle, buf, 0, sizeof (buf))) <= 0)
						break;
					
					write_cb (buf, offset, nread, user_data);
					offset += nread;
				} while (true);
			}
			
			if (notify_cb)
				notify_cb (NotifyCompleted, NULL, user_data);
			
			stream.Close (stream.handle);
			
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
	ctx->write_cb = write_cb;

	if (notify_cb) {
		downloader->AddHandler (Downloader::DownloadProgressChangedEvent, downloader_progress_changed, ctx);
		downloader->AddHandler (Downloader::DownloadFailedEvent, downloader_failed, ctx);
		downloader->AddHandler (Downloader::CompletedEvent, downloader_complete, ctx);
	}

	if (cancellable) {
		cancellable->SetCancelFuncAndData (downloader_abort, downloader, ctx);
	}

	if (downloader->Completed ()) {
		if (notify_cb)
			notify_cb (NotifyCompleted, NULL, user_data);
	} else {
		if (!downloader->Started ()) {
			downloader->Open ("GET", (Uri*)uri, policy);
			downloader->SetStreamFunctions (downloader_write, downloader_notify_size, ctx);
			downloader->Send ();
		}
	}
}

static void
downloader_notify_size (gint64 size, gpointer closure)
{
	NotifyCtx *ctx = (NotifyCtx *) closure;
	ctx->notify_cb (NotifySize, size, ctx->user_data);
}

static void
downloader_write (void *data, gint32 offset, gint32 n, void *closure)
{
	NotifyCtx *ctx = (NotifyCtx *) closure;
	ctx->write_cb (data, offset, n, ctx->user_data);
}

static void
downloader_abort (gpointer data, void *ctx)
{
	Downloader *dl = (Downloader *) data;
	NotifyCtx *nc = (NotifyCtx *)ctx;
	dl->RemoveHandler (Downloader::DownloadProgressChangedEvent, downloader_progress_changed, nc);
	dl->RemoveHandler (Downloader::DownloadFailedEvent, downloader_failed, nc);
	dl->RemoveHandler (Downloader::CompletedEvent, downloader_complete, nc);
	dl->Abort ();
}

static void
downloader_progress_changed (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	NotifyCtx *ctx = (NotifyCtx *) closure;
	Downloader *dl = (Downloader *) sender;
	ctx->notify_cb (NotifyProgressChanged, (gint64) (100 * dl->GetDownloadProgress ()), ctx->user_data);
}

static void
downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	NotifyCtx *ctx = (NotifyCtx *) closure;
	ctx->notify_cb (NotifyCompleted, NULL, ctx->user_data);
	g_free (ctx);
	((Downloader *) sender)->unref_delayed ();
}

static void
downloader_failed (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	NotifyCtx *ctx = (NotifyCtx *) closure;
	ctx->notify_cb (NotifyFailed, NULL, ctx->user_data);
	g_free (ctx);
	((Downloader *) sender)->unref_delayed ();
}

//FIXME: nuke this!
char *
Application::GetResourceAsPath (const char *resourceBase, const Uri *uri)
{
	char *dirname, *path, *filename, *url;
	ManagedStreamCallbacks stream;
	unzFile zipfile;
	struct stat st;
	char buf[4096];
	int nread;
	int fd;
	
	if (!get_resource_cb || !uri || uri->isAbsolute)
		return NULL;
	
	// construct the path name for this resource
	int uriflags = 0;
	if (GetSurface () && GetSurface ()->GetRelaxedMediaMode ())
		uriflags |= UriShowFileScheme;
	filename = uri->ToString ((UriToStringFlags)uriflags);
	CanonicalizeFilename (filename, -1, CanonModeResource);
	if (uri->GetQuery () != NULL) {
		char *sc = strchr (filename, ';');
		if (sc)
			*sc = '/';
	}
	
	path = g_build_filename (GetResourceRoot(), filename, NULL);
	g_free (filename);
	
	if (g_stat (path, &st) != -1) {
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
	
	url = uri->ToString ((UriToStringFlags)uriflags);
	stream = get_resource_cb (resourceBase, url);
	g_free (url);
	
	if (!stream.handle) {
		g_free (path);
		return NULL;
	}
	
	// reset the stream to 0
	if (stream.CanSeek (stream.handle))
		stream.Seek (stream.handle, 0, SEEK_SET);
	
	// create and save the buffer to disk
	if ((fd = g_open (path, O_WRONLY | O_CREAT | O_EXCL, 0600)) == -1) {
		stream.Close (stream.handle);
		g_free (path);
		return NULL;
	}
	
	// write the stream to disk
	do {
		if ((nread = stream.Read (stream.handle, buf, 0, sizeof (buf))) <= 0)
			break;
		
		if (write_all (fd, buf, (size_t) nread) == -1) {
			stream.Close (stream.handle);
			g_unlink (path);
			g_free (path);
			close (fd);
			
			return NULL;
		}
	} while (true);
	
	stream.Close (stream.handle);
	close (fd);
	
	// check to see if the resource is zipped
	if (!(zipfile = unzOpen (path))) {
		// nope, not zipped...
		return path;
	}
	
	// create a directory to contain our unzipped content
	if (!(dirname = CreateTempDir (path))) {
		unzClose (zipfile);
		g_free (dirname);
		g_unlink (path);
		g_free (path);
		return NULL;
	}
	
	// unzip the contents
	if (!ExtractAll (zipfile, dirname, CanonModeResource)) {
		RemoveDir (dirname);
		unzClose (zipfile);
		g_free (dirname);
		g_unlink (path);
		g_free (path);
		return NULL;
	}
	
	unzClose (zipfile);
	g_unlink (path);
	
	if (g_rename (dirname, path) == -1) {
		RemoveDir (dirname);
		g_free (dirname);
		g_free (path);
		return NULL;
	}
	
	g_free (dirname);
	
	return path;
}

const char*
Application::GetResourceRoot ()
{
	if (!resource_root) {
		char *buf = g_build_filename (g_get_tmp_dir (), "moonlight-app.XXXXXX", NULL);
		// create a root temp directory for all files
		if (!(resource_root = MakeTempDir (buf)))
			g_free (buf);
		Deployment::GetCurrent()->TrackPath (resource_root);
	}
	return resource_root;
}