/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * utils.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */


#ifndef __UTILS_H__
#define __UTILS_H__

#include <glib.h>
#include <cairo.h>
#include <sys/types.h>

#include "moonbuild.h"
#include "downloader.h"
#include "zip/unzip.h"

G_BEGIN_DECLS

typedef gboolean (*Stream_CanSeek)  (void *handle);
typedef gboolean (*Stream_CanRead)  (void *handle);
typedef gint64   (*Stream_Length)   (void *handle);
typedef gint64   (*Stream_Position) (void *handle);
typedef gint32   (*Stream_Read)     (void *handle,  void *buffer, gint32 offset, gint32 count);
typedef void     (*Stream_Write)    (void *handle,  void *buffer, gint32 offset, gint32 count);
typedef void     (*Stream_Seek)     (void *handle, gint64 offset, gint32 origin);
typedef void     (*Stream_Close)    (void *handle);

struct ManagedStreamCallbacks {
        void *handle;
        Stream_CanSeek CanSeek;
        Stream_CanRead CanRead;
        Stream_Length Length;
        Stream_Position Position;
        Stream_Read Read;
        Stream_Write Write;
        Stream_Seek Seek;
	Stream_Close Close;
};

/* @GeneratePInvoke */
gboolean managed_unzip_stream_to_stream (ManagedStreamCallbacks *source, ManagedStreamCallbacks *dest, const char *partname) MOON_API;

/* @GeneratePInvoke */
gboolean managed_unzip_stream_to_stream_first_file (ManagedStreamCallbacks *source, ManagedStreamCallbacks *dest) MOON_API;

gboolean managed_unzip_extract_to_stream (unzFile zipFile, ManagedStreamCallbacks *dest);

G_GNUC_INTERNAL void g_ptr_array_insert (GPtrArray *array, guint index, void *item);

G_GNUC_INTERNAL void g_ptr_array_insert_sorted (GPtrArray *array, GCompareFunc cmp, void *item);

enum CanonMode {
	CanonModeNone,
	CanonModeXap,
	CanonModeResource
};

const char *CanonicalizeFilename (char *filename, int n, CanonMode mode);

bool ExtractFile (unzFile zip, int fd);
bool ExtractAll (unzFile zip, const char *dir, CanonMode mode);

char *MakeTempDir (char *tmpdir);
char *CreateTempDir (const char *filename);

int RemoveDir (const char *dir);

int CopyFileTo (const char *filename, int fd) MOON_API;

int write_all (int fd, char *buf, size_t len);

cairo_t *measuring_context_create (void);
void     measuring_context_destroy (cairo_t *cr);

GArray *double_garray_from_str (const char *s, gint max);

/*
 * Returns a pointer to the first token found.
 *
 * @input: the input string
 * @c:     output, upon return contains the first character not in the token.
 * @end:   output, upon return contains a pointer to the remaining input (NULL if no more data)
 * 
 * Note: the input string is modified.
 */
char *parse_rfc_1945_token (char *input, char *c, char **end);

/*
 * Returns a pointer to the unquoted string.
 * 
 * @input: the input string
 * @c:     output, upon return contains the first character not in the quoted string.
 * @end:   output, upon return contains a pointer to the remaining input (NULL if no more data)
 * 
 * Note: the input string is modified.
 */
char *parse_rfc_1945_quoted_string (char *input, char *c, char **end);

G_END_DECLS

class TextStream {
 protected:
	char buffer[4096];
	size_t buflen;
	char *bufptr;
	GIConv cd;
	
	char *textbuf;
	char *textbufptr;
	int textbufsize;
	
	int fd;
	bool eof;
	
	bool fmode;
	
	bool ReadBOM (bool force);
	ssize_t ReadInternal (char *buf, ssize_t n);
	
 public:
	TextStream ();
	~TextStream ();
	
	bool OpenBuffer (const char *buf, int size);
	bool OpenFile (const char *filename, bool force);
	void Close ();
	
	bool Eof ();
	
	ssize_t Read (char *buf, size_t n);
};

typedef void (*CancelCallback) (gpointer user_data, void *context);

class Cancellable {
 private:
	CancelCallback cancel_cb;
	Downloader *downloader;
	void *context;
 public:
	Cancellable ();
	~Cancellable ();

	void Cancel ();
	void SetCancelFuncAndData (CancelCallback cb, Downloader *user_data, void *context);
};

#endif /* __UTILS_H__ */
