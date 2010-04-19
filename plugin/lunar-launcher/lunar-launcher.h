/*
 * lunar-launcher.h:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */
 
#ifndef __LUNAR_LAUNCHER_H__
#define __LUNAR_LAUNCHER_H__

#include <glib.h>
#include <gtk/gtk.h>

class LunarLauncher {
private:
	char *application;
	guint32 width;
	guint32 height;
	char *title;
	GtkWindow *top_level_window;
	GtkWidget *moz_embed;

	void ShowUsage ();

	void SetApplication (const char *value);
	void SetTitle (const char *title);
	void SetWidth (guint32 value) { width = value; }
	void SetHeight (guint32 value) { height = value; }

	static gboolean DeleteCallback (GtkWidget* widget, GdkEventKey* event, GtkWindow* window);
	static gboolean KeyPressCallback (GtkWidget* widget, GdkEventKey* event, GtkWindow* window);

public:

	LunarLauncher ();
	~LunarLauncher ();

	int Launch (int argc, char **argv);
};

#endif /* __LUNAR_LAUNCHER_H__ */
 