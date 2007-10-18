/*
 * config.h: MoonLight browser plugin.
 *
 * Author:
 *   Everaldo Canuto (everaldo@novell.com)
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
#include <config.h>

#include <npapi.h>
#include <npupp.h>
#include <npruntime.h>

#include <glib.h>

#if GLIB_SIZEOF_VOID_P == 8
#define GDK_NATIVE_WINDOW_POINTER 1
#endif

#include <gtk/gtk.h>

#include "libmoon.h"

// Definitions
#define DEBUG

// Plugin information
#define PLUGIN_NAME         "Silverlight Plug-In"
#define PLUGIN_VERSION      "0.99.0"
#define PLUGIN_OURNAME      "Novell Moonlight"
#define PLUGIN_OURVERSION   "0.1"
#define PLUGIN_DESCRIPTION  "Novell Moonlight is Mono's Free/Open Source implementation of SilverLight"
#define MIME_TYPES_HANDLED  "application/x-silverlight:xaml:Novell MoonLight"

#define MAX_STREAM_SIZE 65536

#define USE_LIBMOONLOADER 1

#if USE_LIBMOONLOADER
#define LOADER_RENAMED_SYM(x) Plugin_##x
#define LOADER_QUOTE(x) #x
#define LOADER_RENAMED_NAME(x) LOADER_QUOTE(Plugin_##x)

// define his to 1 if we're building the xpi, leave it 0 if we're not
#define PLUGIN_INSTALL 0

extern "C" {
  char* LOADER_RENAMED_SYM(NP_GetMIMEDescription) (void);
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

#ifdef DEBUG
#ifdef G_LOG_DOMAIN
#undef G_LOG_DOMAIN
#endif
#define G_LOG_DOMAIN "Moonlight"
#define DEBUGMSG(x...) g_message (x)
#define DEBUG_WARN_NOTIMPLEMENTED(msg) g_warning ("functionality not yet implemented (%s):" G_STRLOC, msg)
#else
#define DEBUGMSG(x...)
#define DEBUG_WARN_NOTIMPLEMENTED(msg)
#endif

#endif /* PLUGIN_CONFIG */
