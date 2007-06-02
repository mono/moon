/*
 * moon-plugin.h: MoonLight browser plugin.
 *
 * Author:
 *   Everaldo Canuto (everaldo@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef MOON_PLUGIN
#define MOON_PLUGIN

#include <stdio.h>
#include <string.h>
#include <Xlib.h>

#include "npapi.h"
#include "npupp.h"

#include "glib.h"
#include "gtk/gtk.h"

static NPWindow windowlessWindow;

#define DEBUG_PLUGIN

#define PLUGIN_NAME         "Novell MoonLight"
#define PLUGIN_DESCRIPTION  "Novell MoonLight is a Free Unix implementation of SilverLight";
#define PLUGIN_VERSION      "0.1"
#define MIME_TYPES_HANDLED  "application/ag-plugin:scr:Novell MoonLight"

#ifdef DEBUG_PLUGIN
#ifdef G_LOG_DOMAIN
#undef G_LOG_DOMAIN
#endif
#define G_LOG_DOMAIN "MoonLight"
#define DEBUG(x...) g_message (x)
#else
#define DEBUG(msg)
#endif

G_BEGIN_DECLS

typedef struct {

    uint16 mode;
    NPWindow *window;
    uint32 x, y;
    uint32 width, height;

    NPP instance;
    NPBool pluginsHidden;

    GtkWidget *container;
    GtkWidget *canvas;
    guchar *four_quads;
    gboolean dialog_up;

    gboolean windowless;

    /* when we have to go raw */
    XImage *ximage;

} PluginInstance;

NPError moon_plugin_initialise ();
NPError moon_plugin_shutdown ();
NPError moon_plugin_get_value (void *instance, NPPVariable variable, void *result);
NPError moon_plugin_new (NPMIMEType pluginType, NPP instance, uint16 mode, int16 argc, char* argn[], char* argv[], NPSavedData* saved);
NPError moon_plugin_destroy (NPP instance, NPSavedData** save);
NPError moon_plugin_set_window (NPP instance, NPWindow* window);
NPError moon_plugin_create_window (NPP instance, NPWindow* window);
NPError moon_plugin_new_stream (NPP instance, NPMIMEType type, NPStream* stream, NPBool seekable, uint16* stype);
void moon_plugin_stream_as_file (NPP instance, NPStream* stream, const char* fname);
NPError moon_plugin_destroy_stream (NPP instance, NPStream* stream, NPError reason);
int32 moon_plugin_write_ready (NPP instance, NPStream* stream);
int32 moon_plugin_write (NPP instance, NPStream* stream, int32 offset, int32 len, void* buffer);
void moon_plugin_url_notify (NPP instance, const char* url, NPReason reason, void* notifyData);
void moon_plugin_print (NPP instance, NPPrint* platformPrint);
int16 moon_plugin_handle_event (NPP instance, void* event);
gboolean moon_plugin_canvas_event (GtkWidget *widget, GdkEvent *event, gpointer user_data);

gboolean moon_plugin_show_menu (PluginInstance *plugin);

static void moon_plugin_menu_about (PluginInstance *plugin);

G_END_DECLS

#endif /* MOON_PLUGIN */

