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
#define Window _XxWindow
#include "moz-sdk/npapi.h"
#include "moz-sdk/npfunctions.h"
#include "moz-sdk/npruntime.h"
#undef Region
#undef Visual
#undef Window

#include "libmoon.h"
#include "silverlight-versions.h"

// Plugin information
#define MIME_SILVERLIGHT_1  "application/x-silverlight"
#define MIME_SILVERLIGHT_2  "application/x-silverlight-2"
#define PLUGIN_NAME         "Silverlight Plug-In"
#define PLUGIN_VERSION      SILVERLIGHT_4_0_TARGETRUNTIME
#define PLUGIN_OURNAME      "Novell Moonlight"
#define PLUGIN_SUFFIX       "Novell <a href=\"http://www.mono-project.com/Moonlight\">Moonlight</a> " VERSION " is Mono's Free/Open Source implementation of Silverlight."
#define PLUGIN_DESCRIPTION SILVERLIGHT_4_0_TARGETRUNTIME
#define MIME_TYPES_HANDLED  MIME_SILVERLIGHT_1 ":xaml:Novell Moonlight;" MIME_SILVERLIGHT_2 "::Novell Moonlight"

#define MAX_STREAM_SIZE G_MAXINT32

#define USE_LIBMOONLOADER 1

#if USE_LIBMOONLOADER
#define LOADER_RENAMED_SYM(x) MOON_##x
#define LOADER_QUOTE(x) #x
#define LOADER_RENAMED_NAME(x) LOADER_QUOTE(MOON_##x)

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
