/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gtksilver.cpp: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gtksilver.h"

#include "runtime.h"
#include "deployment.h"
#include "gtk/pal-gtk.h"
#include "gtk/window-gtk.h"


struct _GtkSilverPrivate {
	MoonWindow *window;
	Surface *surface;
};


static void gtk_silver_class_init (GtkSilverClass *klass);
static void gtk_silver_init (GtkSilver *silver);
static void gtk_silver_destroy (GtkObject *obj);
static void gtk_silver_finalize (GObject *obj);

static void size_allocated (GtkWidget *widget, GtkAllocation *allocated, gpointer user_data);


static GtkBinClass *parent_class = NULL;


GType
gtk_silver_get_type (void)
{
	static GType type = 0;
	
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GtkSilverClass),
			NULL, /* base_class_init */
			NULL, /* base_class_finalize */
			(GClassInitFunc) gtk_silver_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (GtkSilver),
			0,    /* n_preallocs */
			(GInstanceInitFunc) gtk_silver_init,
		};
		
		type = g_type_register_static (GTK_TYPE_BIN, "GtkSilver", &info, (GTypeFlags) 0);
	}
	
	return type;
}

static void
gtk_silver_class_init (GtkSilverClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkObjectClass *gtk_object_class = GTK_OBJECT_CLASS (klass);
	
	parent_class = (GtkBinClass *) g_type_class_ref (GTK_TYPE_BIN);
	
	object_class->finalize = gtk_silver_finalize;
	gtk_object_class->destroy = gtk_silver_destroy;
}

static void
gtk_silver_init (GtkSilver *silver)
{
	MoonWindowingSystem *winsys = runtime_get_windowing_system ();
	Deployment *deployment = Deployment::GetCurrent ();
	GtkSilverPrivate *priv;
	GtkWidget *widget;
	
	priv = silver->priv = g_new0 (GtkSilverPrivate, 1);
	priv->window = winsys->CreateWindow (false, 0, 0, NULL, NULL);
	priv->surface = new Surface (priv->window);
	
	deployment->SetSurface (priv->surface);
	priv->window->SetSurface (priv->surface);
	
	widget = ((MoonWindowGtk *) priv->window)->GetWidget ();
	g_signal_connect_after (widget, "size-allocate", G_CALLBACK (size_allocated), priv->surface);
	gtk_widget_show (widget);
	
	gtk_container_add ((GtkContainer *) silver, widget);
}

static void
gtk_silver_finalize (GObject *obj)
{
	GtkSilver *silver = (GtkSilver *) obj;
	
	g_free (silver->priv);
	
	G_OBJECT_CLASS (parent_class)->finalize (obj);
}

static void
gtk_silver_destroy (GtkObject *obj)
{
	GTK_OBJECT_CLASS (parent_class)->destroy (obj);
}

static void
size_allocated (GtkWidget *widget, GtkAllocation *allocated, gpointer user_data)
{
	Surface *surface = (Surface *) user_data;
	printf ("resizing to %dx%d\n", allocated->width, allocated->height);
	surface->Resize (allocated->width, allocated->height);
}

GtkWidget *
gtk_silver_new (void)
{
	return (GtkWidget *) g_object_new (GTK_TYPE_SILVER, NULL);
}

void
gtk_silver_load_xap (GtkSilver *silver, const char *xap_path)
{
	Deployment *deployment;
	char *xap_uri;
	
	g_return_if_fail (GTK_IS_SILVER (silver));
	
	xap_uri = g_strdup_printf ("file://%s", xap_path);
	silver->priv->surface->SetSourceLocation (xap_uri);
	
	deployment = Deployment::GetCurrent ();
	deployment->SetIsLoadedFromXap (true);
	deployment->SetXapFilename (xap_path);
	deployment->SetXapLocation (xap_uri);
	deployment->InitializeManagedDeployment (NULL, NULL, NULL);
	g_free (xap_uri);
}
