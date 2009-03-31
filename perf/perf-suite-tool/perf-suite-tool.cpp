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

#include <config.h>
#include <runtime.h>
#include <clock.h>
#include <timemanager.h>
#include <string.h>
#include <stdio.h>
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

/* Globals, parameters */

int interval = 40;		// By default 25 frames per second
int start_time = 0;		// By default start from 0
int end_time = 5000;		// By default end after 5 seconds
int timeout = 20000;		// By default 20 seconds
gint runs_left = 1;		// Do just one run by default
char *filename = NULL;		// No default filename
char *results_filename = NULL;	// No default results filename
int width = 400;
int height = 400;

/* Globals, other */

int current_time;
GtkWidget *moz_embed;
GtkWindow *window;
glong benchmark_start;
FILE *results_io = NULL;
uint critical_timeout_id = 0;

/* For parameter parsing */

static GOptionEntry entries [] =
{
	{ "start-time", 's', 0, G_OPTION_ARG_INT, &start_time, "Start time is S mseconds", "S" },
	{ "end-time", 'e', 0, G_OPTION_ARG_INT, &end_time, "End time is S mseconds", "S" },
	{ "interval", 'i', 0, G_OPTION_ARG_INT, &interval, "Interval between frames in S mseconds", "S" },
	{ "runs", 'n', 0, G_OPTION_ARG_INT, &runs_left, "Do N runs", "N" },
	{ "filename", 'f', 0, G_OPTION_ARG_STRING, &filename, "Filename to load", NULL },
	{ "results-filename", 'r', 0, G_OPTION_ARG_STRING, &results_filename, "Filename to save results to", NULL },
	{ "timeout", 't', 0, G_OPTION_ARG_INT, &timeout, "Timeout the test (failure) in S mseconds", "S" },
	{ "width", 'w', 0, G_OPTION_ARG_INT, &width, "Width of the test window in P pixels", "P" },
	{ "height", 'h', 0, G_OPTION_ARG_INT, &height, "Height of the test window in H pixels", "H" },
	{ NULL }
};

/* Decls */

void start_xml (void);

void end_xml (void);

void save_result (long v);

glong get_time (void);

void fake_capture (void);

void unsetup (void);

gboolean increase_timer (void *data);

void expose_handoff (Surface *s, TimeSpan time, void* data);

gboolean critical_timeout (void* data);

gboolean setup (void* data);

void do_run (void);

/* Code */

void start_xml (void)
{
	if (results_io)
		fprintf (results_io, "<DrtResult>\n");
}

void end_xml (void)
{
	if (results_io) {
		fprintf (results_io, "</DrtResult>\n");
		fclose (results_io);
		results_io = NULL;
	}
}

void save_result (long v)
{
	if (results_io) {
		fprintf (results_io, "  <DrtRun time=\"%ld\" />\n", v);
	}
}

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
	g_object_unref (root);
}

void unsetup (void)
{
	Surface *surface = (Surface *) runtime_get_surface_list ()->data; 
	surface->SetExposeHandoffFunc (NULL, NULL);
}

gboolean increase_timer (void *data)
{
	if (runtime_get_surface_list () == NULL)
		return TRUE;

	Surface *surface = (Surface *) runtime_get_surface_list ()->data; 

	if (surface == NULL)
		return TRUE;
    
	TimeManager *manager = surface->GetTimeManager ();
	ManualTimeSource *source = (ManualTimeSource *) manager->GetSource ();

	if (current_time > end_time) {
		long result = get_time () - benchmark_start;
		printf ("*** Run finished, result: %.5fs\n", result / (float) G_USEC_PER_SEC);
		save_result (result);
		unsetup ();

		if (runs_left > 0) {
			do_run ();
			return FALSE;
		} else {
			printf ("*** All done, exiting...\n");
			end_xml ();
			exit (0);
		}
	}

	source->SetCurrentTime (TimeSpan_FromSecondsFloat ((float) current_time / 1000));
	current_time += interval;

	return FALSE;
}

void expose_handoff (Surface *s, TimeSpan time, void* data)
{
	g_idle_add (increase_timer, NULL);
}

gboolean critical_timeout (void* data)
{
	// If the timeout occurs, we automatically exit with fault
	printf ("*** Timeout occured! Failure...\n");
	exit (128);
}

gboolean setup (void* data)
{
	printf ("*** Setting up a run...\n");
	if (runtime_get_surface_list () == NULL)
		return TRUE;

	Surface *surface = (Surface *) runtime_get_surface_list ()->data; 

	if (surface == NULL)
		return TRUE;
 
	surface->SetExposeHandoffFunc (expose_handoff, NULL);
	g_idle_add (increase_timer, NULL);
	current_time = start_time;
	benchmark_start = get_time ();

	return FALSE;    
}

gboolean poke (void* data)
{
	int poke_no = GPOINTER_TO_INT (data);
	printf ("*** Poking %d\n", poke_no);

	if (runtime_get_surface_list () == NULL)
		return TRUE;

	Surface *surface = (Surface *) runtime_get_surface_list ()->data; 

	if (surface == NULL)
		return TRUE;
    
	TimeManager *manager = surface->GetTimeManager ();
	ManualTimeSource *source = (ManualTimeSource *) manager->GetSource ();

	source->SetCurrentTime (0);

	if (poke_no == 3) 
		g_timeout_add (1000, setup, NULL);
	else
		g_timeout_add (1000, poke, (void *) (poke_no + 1));

	return FALSE;
}

void do_run (void)
{
	printf ("*** Starting up a run...\n");

	char *current_directory = g_get_current_dir ();
	char *html_path = g_strdup_printf ("file://%s/%s", current_directory, filename);
	gtk_moz_embed_load_url (GTK_MOZ_EMBED (moz_embed), html_path);

	g_free (current_directory);
	g_free (html_path);
	
	g_timeout_add (1000, poke, (void *) 1);

	if (critical_timeout_id != 0) 
		g_source_remove (critical_timeout_id);

	critical_timeout_id = g_timeout_add (timeout, critical_timeout, NULL);

	runs_left--;
}

int
main (int argc, char **argv)
{
	GError *error = NULL;
	GOptionContext *context;

	context = g_option_context_new ("- benchmark a given HTML/XAML file");
	g_option_context_add_main_entries (context, entries, NULL);
	g_option_context_add_group (context, gtk_get_option_group (TRUE));

	if (! g_option_context_parse (context, &argc, &argv, &error)) {
		g_print ("!!! Option parsing failed: %s\n", error->message);
		exit (1);
	}

	if (filename == NULL) {
		g_print ("!!! File to load not specified!\n");
		exit (1);
	}

	gtk_init (&argc, &argv);
	runtime_init_browser(NULL);

	window = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));

	char xpcom_lib_path [PATH_MAX];
	char* xpcom_dir_path;

	GRE_GetGREPathWithProperties (&gre_version, 1, nsnull, 0, xpcom_lib_path, sizeof (xpcom_lib_path));
	xpcom_dir_path = g_path_get_dirname (xpcom_lib_path);

	gtk_moz_embed_set_path (xpcom_dir_path);
	g_free (xpcom_dir_path);

	moz_embed = gtk_moz_embed_new();
	gtk_container_add (GTK_CONTAINER (window), moz_embed);

	gtk_widget_set_usize (moz_embed, width + 16, height + 16);

	gtk_widget_show_all (moz_embed);
	gtk_widget_show_all (GTK_WIDGET (window));

	if (results_filename) {
		results_io = fopen (results_filename, "w");
	}

	start_xml ();

	runtime_flags_set_manual_timesource (TRUE);
	runtime_flags_set_use_shapecache (FALSE);
	runtime_flags_set_show_fps (FALSE);

	do_run ();
	gtk_main ();

	return 0;
}
