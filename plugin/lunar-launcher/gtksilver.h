/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gtksilver.h:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __GTK_SILVER_H__
#define __GTK_SILVER_H__

#include <gtk/gtk.h>

#define GTK_TYPE_SILVER            (gtk_silver_get_type ())
#define GTK_SILVER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_SILVER, GtkSilver))
#define GTK_SILVER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_SILVER, GtkSilverClass))
#define GTK_IS_SILVER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_SILVER))
#define GTK_IS_SILVER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_SILVER))
#define GTK_SILVER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_SILVER, GtkSilverClass))

typedef struct _GtkSilver GtkSilver;
typedef struct _GtkSilverClass GtkSilverClass;
typedef struct _GtkSilverPrivate GtkSilverPrivate;

struct _GtkSilver {
	GtkBin parent_object;
	
	GtkSilverPrivate *priv;
};

struct _GtkSilverClass {
	GtkBinClass parent_class;
};

GType gtk_silver_get_type (void);

GtkWidget *gtk_silver_new (void);

void gtk_silver_load_xap (GtkSilver *silver, const char *xap_path);

#endif /* __GTK_SILVER_H__ */
