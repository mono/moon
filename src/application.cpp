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

namespace Moonlight {

Application::Application ()
{
	SetObjectType (Type::APPLICATION);
	
	install_state = InstallStateUnknown;
	resource_root = NULL;
	
	get_default_style_cb = NULL;
	convert_setter_values_cb = NULL;
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
Application::RegisterCallbacks (GetDefaultStyleCallback get_default_style_cb,
				ConvertSetterValuesCallback convert_setter_values_cb,
				GetResourceCallback get_resource_cb,
				ConvertKeyframeValueCallback convert_keyframe_callback)
{
	this->get_default_style_cb = get_default_style_cb;
	this->convert_setter_values_cb = convert_setter_values_cb;
	this->convert_keyframe_callback = convert_keyframe_callback;
	this->get_resource_cb = get_resource_cb;
}

Style *
Application::GetDefaultStyle (ManagedTypeInfo *key)
{
	if (get_default_style_cb)
		return get_default_style_cb (key);
	return NULL;
}

void
Application::ConvertSetterValues (Style *style)
{
	if (convert_setter_values_cb)
		convert_setter_values_cb (style);
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
	EventHandler write_cb;
	HttpRequest *request;
};

static void application_downloader_abort (HttpRequest *request, void *ctx);
static void application_downloader_progress_changed (EventObject *sender, EventArgs *calldata, gpointer closure);
static void application_downloader_stopped (EventObject *sender, EventArgs *calldata, gpointer closure);
static void application_downloader_write (EventObject *sender, EventArgs *calldata, gpointer closure);

bool
Application::GetResource (const char *resourceBase, const Uri *uri,
			  NotifyFunc notify_cb, EventHandler write_cb,
			  DownloaderAccessPolicy policy, HttpRequest::Options options,
			  Cancellable *cancellable, gpointer user_data)
{
	if (!uri) {
		g_warning ("Passing a null uri to Application::GetResource");
		if (notify_cb)
			notify_cb (NotifyFailed, NULL, user_data);
		
		return false;
	}

	if (get_resource_cb && uri && !uri->isAbsolute) {
		char *url = uri->ToString ();
		ManagedStreamCallbacks stream;
		if (url != NULL && url [0] != 0) {
			stream = get_resource_cb (resourceBase, url);
		} else {
			memset (&stream, 0, sizeof (stream));
		}
		g_free (url);
		
		if (stream.handle) {
			if (notify_cb)
				notify_cb (NotifyStarted, NULL, user_data);
			
			if (write_cb) {
				char buf[4096];
				int offset = 0;
				int nread;
				
				if (stream.CanSeek (stream.handle))
					stream.Seek (stream.handle, 0, 0);
				
				HttpRequestWriteEventArgs *args = new HttpRequestWriteEventArgs (NULL, 0, 0);
				do {
					if ((nread = stream.Read (stream.handle, buf, 0, sizeof (buf))) <= 0)
						break;
					
					args->SetData (buf);
					args->SetOffset (offset);
					args->SetCount (nread);
					write_cb (this, args, user_data);
					offset += nread;
				} while (true);
				args->unref ();
			}
			
			if (notify_cb)
				notify_cb (NotifyCompleted, NULL, user_data);
			
			stream.Close (stream.handle);
			
			return true;
		}
	}	
	
#if 0
	// FIXME: drt 171 and 173 expect this to fail simply because the uri
	// begins with a '/', but other drts (like 238) depend on this
	// working. I give up.
	if (!uri->isAbsolute && uri->path && uri->path[0] == '/') {
		if (notify_cb)
			notify_cb (NotifyFailed, NULL, user_data);
		
		return false;
	}
#endif
	
	//no get_resource_cb or empty stream
	HttpRequest *request;
	if (!(request = GetDeployment ()->CreateHttpRequest (options))) {
		if (notify_cb)
			notify_cb (NotifyFailed, NULL, user_data);
		
		return false;
	}
	
	NotifyCtx *ctx = g_new (NotifyCtx, 1);
	ctx->user_data = user_data;
	ctx->notify_cb = notify_cb;
	ctx->write_cb = write_cb;
	ctx->request = request;

	if (notify_cb)
		request->AddHandler (HttpRequest::ProgressChangedEvent, application_downloader_progress_changed, ctx);

	if (cancellable) {
		cancellable->SetCancelFuncAndData (application_downloader_abort, request, ctx);
	}

	request->AddHandler (HttpRequest::WriteEvent, application_downloader_write, ctx);
	request->AddHandler (HttpRequest::StoppedEvent, application_downloader_stopped, ctx);
	request->Open ("GET", (Uri *) uri, policy);
	request->Send ();
	
	return true;
}

static void
application_downloader_write (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	NotifyCtx *ctx = (NotifyCtx *) closure;
	ctx->write_cb (sender, calldata, ctx->user_data);
}

static void
application_downloader_abort (HttpRequest *request, void *ctx)
{
	request->Abort ();
	request->RemoveAllHandlers (ctx);
}

static void
application_downloader_progress_changed (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	NotifyCtx *ctx = (NotifyCtx *) closure;
	HttpRequestProgressChangedEventArgs *args = (HttpRequestProgressChangedEventArgs *) calldata;
	if (ctx->notify_cb)
		ctx->notify_cb (NotifyProgressChanged, (gint64) (100 * args->GetProgress ()), ctx->user_data);
}

static void
application_downloader_stopped (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	HttpRequestStoppedEventArgs *args = (HttpRequestStoppedEventArgs *) calldata;
	NotifyCtx *ctx = (NotifyCtx *) closure;
	if (ctx->notify_cb)
		ctx->notify_cb (args->IsSuccess () ? NotifyCompleted : NotifyFailed, NULL, ctx->user_data);
	ctx->request->RemoveAllHandlers (ctx);
	ctx->request->unref ();
	g_free (ctx);
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
	filename = uri->ToString ();
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
	
	url = uri->ToString ();
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

void
Application::UpdateComplete (bool updated, const char *error)
{
	CheckAndDownloadUpdateCompletedEventArgs *args;
	
	args = new CheckAndDownloadUpdateCompletedEventArgs (updated, error);
	
	Emit (Application::CheckAndDownloadUpdateCompletedEvent, args);
}

void
Application::update_complete (bool updated, const char *error, gpointer user_data)
{
	((Application *) user_data)->UpdateComplete (updated, error);
}

void
Application::CheckAndDownloadUpdateAsync ()
{
	MoonInstallerService *installer = runtime_get_installer_service ();
	Deployment *deployment = Deployment::GetCurrent ();
	
	installer->CheckAndDownloadUpdateAsync (deployment, Application::update_complete, this);
}

bool
Application::IsRunningOutOfBrowser ()
{
	MoonInstallerService *installer = runtime_get_installer_service ();
	Deployment *deployment = Deployment::GetCurrent ();
	
	return installer->IsRunningOutOfBrowser (deployment);
}

void
Application::SetInstallState (InstallState state)
{
	if (install_state == state)
		return;
	
	install_state = state;
	
	Emit (Application::InstallStateChangedEvent, NULL);
}

InstallState
Application::GetInstallState ()
{
	if (install_state == InstallStateUnknown) {
		MoonInstallerService *installer = runtime_get_installer_service ();
		
		if (installer->CheckInstalled (Deployment::GetCurrent ()))
			install_state = InstallStateInstalled;
		else
			install_state = InstallStateNotInstalled;
	}
	
	return install_state;
}

bool
Application::IsInstallable ()
{
	Deployment *deployment = Deployment::GetCurrent ();
	const char *location = deployment->GetXapLocation ();
	
	return location && (!g_ascii_strncasecmp (location, "file:", 5) ||
			    !g_ascii_strncasecmp (location, "http:", 5) ||
			    !g_ascii_strncasecmp (location, "https:", 6));
}

bool
Application::InstallWithError (MoonError *error, bool unattended)
{
	MoonInstallerService *installer = runtime_get_installer_service ();
	Deployment *deployment = Deployment::GetCurrent ();

	// application manifest must allow out-of-browser support
	OutOfBrowserSettings *settings = deployment->GetOutOfBrowserSettings ();
	if (!settings) {
		// FIXME - this fix a crasher but is not complete wrt MSDN
		return false;
	}

	if (!IsInstallable ()) {
		MoonError::FillIn (error, MoonError::INVALID_OPERATION, "Applications may only be installed from http, https or file URLs");
		return false;
	}
	
	if (GetInstallState () == InstallStateInstalled || IsRunningOutOfBrowser ()) {
		MoonError::FillIn (error, MoonError::INVALID_OPERATION, "Application is already installed");
		return false;
	}

	// the dialog is displayed only if the action leading to this call was initiated directly from the user
	if (!unattended && !deployment->GetSurface ()->IsUserInitiatedEvent ())
		return false;
	
	SetInstallState (InstallStateInstalling);
	
	if (installer->Install (deployment, unattended)) {
		SetInstallState (InstallStateInstalled);
		return true;
	}
	
	SetInstallState (InstallStateInstallFailed);
	
	return false;
}

bool
Application::Install ()
{
	MoonError err;
	
	return InstallWithError (&err, false);
}

void
Application::Uninstall ()
{
	MoonInstallerService *installer = runtime_get_installer_service ();
	Deployment *deployment = Deployment::GetCurrent ();
	
	installer->Uninstall (deployment);
	
	SetInstallState (InstallStateNotInstalled);
}

};
