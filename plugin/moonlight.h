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
#include "gtk/gtk.h"

#include "runtime.h"
#include "shape.h"

// Definitions
#define DEBUG
#define SCRIPTING
#define RUNTIME

#define MOON_1_0
#define MOON_1_1

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
#else
#define DEBUGMSG(x...)
#endif

#endif /* PLUGIN_CONFIG */
