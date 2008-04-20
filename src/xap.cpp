/*
 * xaml.cpp: xaml parser
 *
 * Author:
 *   Miguel de Icaza (miguel@ximian.com)
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

#include "xaml.h"
#include "error.h"
#include "utils.h"
#include "zip/unzip.h"
#include "xap.h"

static char *
xap_unpack (const char *fname)
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
		int fd;
		char *fname, *output, *dirname;
		unz_file_info finfo;

		unzGetCurrentFileInfo (zipfile, &finfo, NULL, 0, NULL, 0, NULL, 0);
		fname = (char *) malloc (finfo.size_filename + 2);
		if (fname == 0)
			goto exception1;
		unzGetCurrentFileInfo (zipfile, NULL, fname, finfo.size_filename+1, NULL, 0, NULL, 0);

		output = g_build_filename (xap_dir, fname, NULL);
		dirname = g_path_get_dirname (output);
		g_mkdir_with_parents (dirname, 0644);
		g_free (dirname);

		fd = open (output, O_CREAT | O_WRONLY, 0644);
		g_free (output);
		g_free (fname);

		if (fd == -1)
			goto exception1;

		if (unzOpenCurrentFile (zipfile) != UNZ_OK)
			goto exception1;

		int exc = ExtractFile (zipfile, fd);
		unzCloseCurrentFile (zipfile);
		if (exc != UNZ_OK)
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
	char *xap_dir = xap_unpack (filename);
	Type::Kind element_type;
	DependencyObject *element;

	if (xap_dir == NULL)
		return NULL;

	// Load the AppManifest file
	char *manifest = g_build_filename (xap_dir, "AppManifest.xaml", NULL);
	element = xaml_create_from_file (loader, manifest, false, &element_type);
	g_free (manifest);

	if (element_type != Kind::DEPLOYMENT)
		return NULL;


	// TODO: Create a DependencyObject from the root node.

	Xap *xap = new Xap (loader, xap_dir, element);
	return xap;
}
