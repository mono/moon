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

#include "npapi.h"
#include "npupp.h"
#include "npruntime.h"

#include "glib.h"

#if GLIB_SIZEOF_VOID_P == 8
#define GDK_NATIVE_WINDOW_POINTER 1
#endif

#include "gtk/gtk.h"

#include "libmoon.h"

// Definitions
#define DEBUG

// Plugin information
#define PLUGIN_NAME         "WPFe Plug-In"
#define PLUGIN_VERSION      "0.99.0"
#define PLUGIN_OURNAME      "Novell Moonlight"
#define PLUGIN_OURVERSION   "0.1"
#define PLUGIN_DESCRIPTION  "Novell Moonlight is Mono's Free/Open Source implementation of SilverLight"
#define MIME_TYPES_HANDLED  "application/ag-plugin:xaml:Novell MoonLight"

#define MAX_STREAM_SIZE 65536

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
