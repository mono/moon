/*
 * moonlight.h: MoonLight browser plugin.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef PLUGIN_CONFIG
#define PLUGIN_CONFIG

#include <stdio.h>
#include <string.h>

#define MOZ_X11
#ifndef XP_UNIX
#define XP_UNIX
#endif

#define Region _XxRegion
#define Visual _XxVisual
#include <npapi.h>
#include <npfunctions.h>
#include <npruntime.h>
#undef Region
#undef Visual

#include <glib.h>

#if GLIB_SIZEOF_VOID_P == 8
#define GDK_NATIVE_WINDOW_POINTER 1
#endif

#include <gtk/gtk.h>

#include "libmoon.h"

// Plugin information
#define MIME_SILVERLIGHT_1  "application/x-silverlight"
#define MIME_SILVERLIGHT_2  "application/x-silverlight-2"
#define PLUGIN_NAME         "Silverlight Plug-In"
#define PLUGIN_VERSION      VERSION
#define PLUGIN_OURNAME      "Novell Moonlight"
#define PLUGIN_SUFFIX       "Novell <a href=\"http://www.mono-project.com/Moonlight\">Moonlight</a> " VERSION " is Mono's Free/Open Source implementation of Silverlight."
#if PLUGIN_SL_2_0
#    define PLUGIN_DESCRIPTION  "2.0.31005.0"
#    define MIME_TYPES_HANDLED  MIME_SILVERLIGHT_1 ":scr:Novell Moonlight;" MIME_SILVERLIGHT_2 "::Novell Moonlight"
#else
#    define PLUGIN_DESCRIPTION  "1.0.30401.0"
#    define MIME_TYPES_HANDLED  MIME_SILVERLIGHT_1 ":scr:Novell Moonlight" 
#endif

#define MAX_STREAM_SIZE 65536

#define USE_LIBMOONLOADER 1

#if USE_LIBMOONLOADER
#define LOADER_RENAMED_SYM(x) Plugin_##x
#define LOADER_QUOTE(x) #x
#define LOADER_RENAMED_NAME(x) LOADER_QUOTE(Plugin_##x)

extern "C" {
  char *LOADER_RENAMED_SYM(NP_GetMIMEDescription) (void);
  NPError LOADER_RENAMED_SYM(NP_GetValue) (void *future, NPPVariable variable, void *value);
#ifdef XP_UNIX
  NPError OSCALL LOADER_RENAMED_SYM(NP_Initialize) (NPNetscapeFuncs *mozilla_funcs, NPPluginFuncs *plugin_funcs);
#else
  NPError OSCALL LOADER_RENAMED_SYM(NP_Initialize) (NPNetscapeFuncs *mozilla_funcs);
#endif
  NPError OSCALL LOADER_RENAMED_SYM(NP_Shutdown) (void);
}
#else
#define LOADER_RENAMED_SYM(x) x
#define LOADER_RENAMED_NAME(x) #x
#endif

#ifdef G_LOG_DOMAIN
#undef G_LOG_DOMAIN
#endif

#define G_LOG_DOMAIN "Moonlight"

#endif /* PLUGIN_CONFIG */
