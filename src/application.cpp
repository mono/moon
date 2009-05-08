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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

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
				GetResourceCallback get_resource_cb)
{
	this->apply_default_style_cb = apply_default_style_cb;
	this->apply_style_cb = apply_style_cb;
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

gpointer
Application::GetResource (const Uri *uri, int *size)
{
	if (get_resource_cb && uri) {
		char *url = uri->ToString ();
		gpointer ptr = get_resource_cb (url, size);
		g_free (url);
		
		return ptr;
	}
	
	*size = 0;
	
	return NULL;
}

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
		path = g_build_filename (g_get_tmp_dir (), "moonlight-app.XXXXXX", NULL);
		if (!(resource_root = CreateTempDir (path))) {
			g_free (path);
			return NULL;
		}
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
	url = uri->ToString ();
	if (!(buf = get_resource_cb (url, &size))) {
		g_free (path);
		g_free (url);
		return NULL;
	}
	
	g_free (url);
	
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
