/*
 * openfile.h: File open / save interfaces
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007, 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __OPENFILE_H__
#define __OPENFILE_H__

#include <glib.h>

#include "pal.h"

namespace Moonlight {

G_BEGIN_DECLS

// NOTE: this is used from 'mscorlib.dll' System.IO.IsolatedStorage/MoonIsolatedStorageFile.cs
MOON_API gboolean isolated_storage_increase_quota_to (const char *primary_text, const char* secondary_text);

// NOTE: this is used from 'mscorlib.dll' System.IO.IsolatedStorage/MoonIsolatedStorage.cs
MOON_API long isolated_storage_get_current_usage (const char* root);

G_END_DECLS

};
#endif
