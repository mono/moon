/*
 * Copyright (c) 2008 Novell, Inc. (http://www.novell.com)
 *
 * Contact:
 *  Moonlight List (moonlight-list@lists.ximian.com)
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <runtime.h>
#include <clock.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <signal.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <dlfcn.h>
#include <gtkmozembed.h>
#include <nsXPCOMGlue.h>

static const GREVersionRange gre_version = {
	"1.9", PR_TRUE,
	"9.9", PR_TRUE
};

float current_time = 0.0;
GtkWindow *window;
glong start_time;

glong get_time (void)
{
    static GTimeVal time_val;
    g_get_current_time (&time_val);
    return (time_val.tv_sec * G_USEC_PER_SEC) + time_val.tv_usec;
}

void fake_capture (void)
{
	int x, y, w, h;
	gtk_window_get_frame_dimensions (window, &x, &y, &w, &h);
	gdk_window_get_origin (((GtkWidget *) window)->window, &x, &y);
	gdk_drawable_get_size (((GtkWidget *) window)->window, &w, &h);
    
	GdkWindow* root = gdk_window_foreign_new (GDK_ROOT_WINDOW ());
	GdkPixbuf* buf = gdk_pixbuf_get_from_drawable (NULL, root, NULL, x, y, 0, 0, w, h);
	gdk_pixbuf_unref (buf);
}

gboolean increase_timer (void *data)
{
	if (runtime_get_surface_list () == NULL)
		return TRUE;

	Surface *surface = (Surface *) runtime_get_surface_list ()->data; 

	if (surface == NULL)
		return TRUE;
    
	TimeManager *manager = surface_get_time_manager (surface);
	ManualTimeSource *source = (ManualTimeSource *) manager->GetSource ();

	source->SetCurrentTime (TimeSpan_FromSecondsFloat (current_time));
	current_time += 0.04; // 25 frames per second

	if (current_time > 5.0) {
		printf ("BENCHMARK FINISHED! TOOK: %.5fs\n", (get_time () - start_time) / (float) G_USEC_PER_SEC);
		exit (0);
	}

	return FALSE;
}

void expose_handoff (Surface *s, TimeSpan time, void* data)
{
	g_idle_add (increase_timer, NULL);
}

gboolean setup (void* data)
{
	if (runtime_get_surface_list () == NULL)
		return TRUE;

	Surface *surface = (Surface *) runtime_get_surface_list ()->data; 

	if (surface == NULL)
		return TRUE;
 
	TimeManager *manager = surface_get_time_manager (surface);
	ManualTimeSource *source = (ManualTimeSource *) manager->GetSource ();
    
	printf ("Setting up...\n");
	surface->SetExposeHandoffFunc (expose_handoff, NULL);
	g_idle_add (increase_timer, NULL);
    start_time = get_time ();

	return FALSE;    
}

int
main (int argc, char **argv)
{
	gtk_init (&argc, &argv);
	runtime_init (RUNTIME_INIT_BROWSER);

	window = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));

	char xpcom_lib_path [PATH_MAX];
	char* xpcom_dir_path;

	GRE_GetGREPathWithProperties (&gre_version, 1, nsnull, 0, xpcom_lib_path, sizeof (xpcom_lib_path));
	xpcom_dir_path = g_path_get_dirname (xpcom_lib_path);

	gtk_moz_embed_set_path (xpcom_dir_path);
	g_free (xpcom_dir_path);

	GtkWidget *moz_embed = gtk_moz_embed_new();
	gtk_container_add (GTK_CONTAINER (window), moz_embed);

	gtk_widget_set_usize (moz_embed, 416, 416);

	char *current_directory = g_get_current_dir ();
	char *html_path = g_strdup_printf ("file://%s/test.html", current_directory);
	gtk_moz_embed_load_url (GTK_MOZ_EMBED (moz_embed), html_path);

	g_free (current_directory);
	g_free (html_path);

	gtk_widget_show_all (moz_embed);
	gtk_widget_show_all (GTK_WIDGET (window));
	g_timeout_add (500, setup, NULL);

	runtime_flags_set_manual_timesource (TRUE);
	runtime_flags_set_use_shapecache (FALSE);
	runtime_flags_set_show_fps (FALSE);
	
	gtk_main ();

	return 0;
}
