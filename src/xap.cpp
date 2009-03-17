/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * xap.cpp:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */
#include <config.h>
#include <string.h>
#include <malloc.h>
#include <glib.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "xaml.h"
#include "error.h"
#include "utils.h"
#include "type.h"
#include "zip/unzip.h"
#include "xap.h"

char *
Xap::Unpack (const char *fname)
{
	char *xap_dir;

	xap_dir = CreateTempDir (fname);
	if (xap_dir == NULL)
		return NULL;

	unzFile zipfile = unzOpen (fname);
	if (zipfile == NULL)
		goto exception0;

	if (unzGoToFirstFile (zipfile) != UNZ_OK)
		goto exception1;
	
	if (unzOpenCurrentFile (zipfile) != UNZ_OK)
		goto exception1;

	do {
		char *filename, *path, *dirname, *s;
		unz_file_info finfo;
		int fd;
		
		unzGetCurrentFileInfo (zipfile, &finfo, NULL, 0, NULL, 0, NULL, 0);
		filename = (char *) g_malloc (finfo.size_filename + 2);
		if (filename == 0)
			goto exception1;
		
		unzGetCurrentFileInfo (zipfile, NULL, filename, finfo.size_filename+1, NULL, 0, NULL, 0);
		
		if (finfo.external_fa & (1 << 4)) {
			g_free (filename);
			continue;
		}
		
		for (s = filename; *s; s++) {
			if (*s == '\\')
				*s = '/';
		}
		
		path = g_build_filename (xap_dir, filename, NULL);
		g_free (filename);
		
		dirname = g_path_get_dirname (path);
		g_mkdir_with_parents (dirname, 0700);
		g_free (dirname);
		
		fd = open (path, O_CREAT | O_WRONLY, 0644);
		g_free (path);
		
		if (fd == -1)
			goto exception1;
		
		if (unzOpenCurrentFile (zipfile) != UNZ_OK)
			goto exception1;
		
		bool exc = ExtractFile (zipfile, fd);
		unzCloseCurrentFile (zipfile);
		if (exc == false)
			goto exception1;
	} while (unzGoToNextFile (zipfile) == UNZ_OK);
	unzClose (zipfile);

	return xap_dir;

 exception1:
	unzClose (zipfile);

 exception0:
	RemoveDir (xap_dir);
	g_free (xap_dir);

	return NULL;
}

Xap::Xap (XamlLoader *loader, char *xap_dir, DependencyObject *root)
{
	this->loader = loader;
	this->xap_dir = xap_dir;
	this->root = root;
}

Xap::~Xap ()
{
	g_free (xap_dir);
	xap_dir = NULL;
}

Xap *
xap_create_from_file (XamlLoader *loader, const char *filename)
{
	char *xap_dir = Xap::Unpack (filename);
	Type::Kind element_type;
	DependencyObject *element;

	if (xap_dir == NULL)
		return NULL;

	// Load the AppManifest file
	char *manifest = g_build_filename (xap_dir, "AppManifest.xaml", NULL);
	element = loader->CreateFromFile (manifest, false, &element_type);
	g_free (manifest);

	if (element_type != Type::DEPLOYMENT)
		return NULL;

	// TODO: Create a DependencyObject from the root node.

	Xap *xap = new Xap (loader, xap_dir, element);
	return xap;
}
