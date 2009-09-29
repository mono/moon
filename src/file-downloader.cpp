/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * file-downloader.cpp: File Downloader class.
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

#include <glib/gstdio.h>
#include <fcntl.h>
#include <errno.h>

#include "file-downloader.h"
#include "zip/unzip.h"
#include "utils.h"
#include "error.h"

//TODO: Move all the zip related semantics in here to clean up downloader.cpp

FileDownloader::FileDownloader (Downloader *dl) : InternalDownloader (dl, Type::FILEDOWNLOADER)
{
	filename = NULL;
	unzipdir = NULL;
	uri = NULL;
	
	unzipped = false;
	unlinkit = false;
}

FileDownloader::~FileDownloader ()
{
	CleanupUnzipDir ();
	
	if (filename) {
		if (unlinkit)
			unlink (filename);
		g_free (filename);
	}
}

void
FileDownloader::CleanupUnzipDir ()
{
	if (!unzipdir)
		return;
	
	RemoveDir (unzipdir);
	g_free (unzipdir);
	unzipped = false;
	unzipdir = NULL;
}

bool
FileDownloader::DownloadedFileIsZipped ()
{
	unzFile zipfile;
	
	if (!filename)
		return false;
	
	if (!(zipfile = unzOpen (filename)))
		return false;
	
	unzClose (zipfile);
	
	return true;
}

char *
FileDownloader::GetResponseText (const char *partname, gint64 *size)
{
	TextStream *stream;
	char buffer[4096];
	GByteArray *buf;
	struct stat st;
	ssize_t nread;
	char *data;
	char *path;
	
	if (!(path = GetDownloadedFilename (partname)))
		return NULL;
	
	if (g_stat (path, &st) == -1) {
		g_free (path);
		return NULL;
	}
	
	if (st.st_size > 0) {
		stream = new TextStream ();
		
		if (!stream->OpenFile (path, true)) {
			delete stream;
			g_free (path);
			return NULL;
		}
		
		g_free (path);
		
		buf = g_byte_array_new ();
		while ((nread = stream->Read (buffer, sizeof (buffer))) > 0)
			g_byte_array_append (buf, (const guint8 *) buffer, nread);
		
		*size = buf->len;
		
		g_byte_array_append (buf, (const guint8 *) "", 1);
		data = (char *) buf->data;
		
		g_byte_array_free (buf, false);
		delete stream;
	} else {
		data = g_strdup ("");
		*size = 0;
	}
	
	return data;
}

const char *
FileDownloader::GetDownloadedFile ()
{
	return filename;
}

char *
FileDownloader::GetDownloadedFilename (const char *partname)
{
	char *dirname, *path, *part;
	unzFile zipfile;
	struct stat st;
	int rv, fd;
	
	if (!filename)
		return NULL;
	
	if (!partname || !partname[0])
		return g_strdup (filename);
	
	if (!DownloadedFileIsZipped ())
		return NULL;
	
	if (!unzipdir && !(unzipdir = CreateTempDir (filename)))
		return NULL;
	
	part = g_ascii_strdown (partname, -1);
	path = g_build_filename (unzipdir, part, NULL);
	if ((rv = g_stat (path, &st)) == -1 && errno == ENOENT) {
		if (strchr (part, '/') != NULL) {
			// create the directory path
			dirname = g_path_get_dirname (path);
			rv = g_mkdir_with_parents (dirname, 0700);
			g_free (dirname);
			
			if (rv == -1 && errno != EEXIST)
				goto exception1;
		}
		
		// open the zip archive...
		if (!(zipfile = unzOpen (filename)))
			goto exception1;
		
		// locate the file we want to extract... (2 = case-insensitive)
		if (unzLocateFile (zipfile, partname, 2) != UNZ_OK)
			goto exception2;
		
		// open the requested part within the zip file
		if (unzOpenCurrentFile (zipfile) != UNZ_OK)
			goto exception2;
		
		// open the output file
		if ((fd = g_open (path, O_CREAT | O_WRONLY | O_TRUNC, 0600)) == -1)
			goto exception3;
		
		// extract the file from the zip archive... (closes the fd on success and fail)
		if (!ExtractFile (zipfile, fd))
			goto exception3;
		
		unzCloseCurrentFile (zipfile);
		unzClose (zipfile);
	} else if (rv == -1) {
		// irrecoverable error
		goto exception0;
	}
	
	g_free (part);
	
	return path;
	
exception3:
	
	unzCloseCurrentFile (zipfile);
	
exception2:
	
	unzClose (zipfile);
	
exception1:
	
	g_free (part);
	
exception0:
	
	g_free (path);
	
	return NULL;
}

const char *
FileDownloader::GetUnzippedPath ()
{
	char filename[256], *p;
	unz_file_info info;
	const char *name;
	GString *path;
	unzFile zip;
	size_t len;
	int fd;
	
	if (!this->filename)
		return NULL;
	
	if (!DownloadedFileIsZipped ())
		return this->filename;
	
	if (!unzipdir && !(unzipdir = CreateTempDir (this->filename)))
		return NULL;
	
	if (unzipped)
		return unzipdir;
	
	// open the zip archive...
	if (!(zip = unzOpen (this->filename)))
		return NULL;
	
	path = g_string_new (unzipdir);
	g_string_append_c (path, G_DIR_SEPARATOR);
	len = path->len;
	
	unzipped = true;
	
	// extract all the parts
	do {
		if (unzOpenCurrentFile (zip) != UNZ_OK)
			break;
		
		unzGetCurrentFileInfo (zip, &info, filename, sizeof (filename),
				       NULL, 0, NULL, 0);
		
		// convert filename to lowercase
		for (p = filename; *p; p++) {
			if (*p >= 'A' && *p <= 'Z')
				*p += 0x20;
		}
		
		if ((name = strrchr (filename, '/'))) {
			// make sure the full directory path exists, if not create it
			g_string_append_len (path, filename, name - filename);
			g_mkdir_with_parents (path->str, 0700);
			g_string_append (path, name);
		} else {
			g_string_append (path, filename);
		}
		
		if ((fd = g_open (path->str, O_WRONLY | O_CREAT | O_EXCL, 0600)) != -1) {
			if (!ExtractFile (zip, fd))
				unzipped = false;
		} else if (errno != EEXIST) {
			unzipped = false;
		}
		
		g_string_truncate (path, len);
		unzCloseCurrentFile (zip);
	} while (unzGoToNextFile (zip) == UNZ_OK);
	
	g_string_free (path, true);
	unzClose (zip);
	
	return unzipdir;
}

void
FileDownloader::Open (const char *verb, const char *uri)
{
	CleanupUnzipDir ();
	
	if (filename) {
		if (unlinkit)
			unlink (filename);
		g_free (filename);
	}
	
	unlinkit = false;
	unzipped = false;
	
	filename = NULL;
	
	dl->InternalOpen (verb, uri);
}

void
FileDownloader::Write (void *buf, gint32 offset, gint32 n)
{
	dl->InternalWrite (buf, offset, n);
}
