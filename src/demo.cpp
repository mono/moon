#define VIDEO_DEMO
#include <string.h>
#include <gtk/gtk.h>
#include <malloc.h>
#include <glib.h>
#include <stdlib.h>

#include "runtime.h"

static Item *v;
static Rectangle *r;

static gboolean
repaint (gpointer data)
{
	static double degree = 0;
	static double degree2 = 0;
	double trans [6];
	double trans2 [6];

	cairo_matrix_init_rotate ((cairo_matrix_t *)&trans, degree);
	degree += 0.01;

	if (v != NULL)
		item_transform_set (v, (double *) trans);

	cairo_matrix_init_rotate ((cairo_matrix_t *)&trans2, degree2);
	degree2 -= 0.1;
	item_transform_set (r, (double *) trans2);

	return TRUE;
}

int
main (int argc, char *argv [])
{
	GtkWidget *w, *box, *button;
	cairo_matrix_t trans;

	gtk_init (&argc, &argv);
	g_thread_init (NULL);
	gdk_threads_init ();
	
	w = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	box = gtk_hbox_new (FALSE, 0);
	button = gtk_button_new_with_label ("dingus");
	gtk_container_add (GTK_CONTAINER (box), button);
	gtk_widget_show_all (box);
	//gtk_container_add (GTK_CONTAINER (w), box);

	// Create our objects
	Surface *t = surface_new (600, 600);
	gtk_widget_set_usize (t->drawing_area, 400, 400);
	gtk_container_add (GTK_CONTAINER(w), t->drawing_area);

	r = rectangle_new (100, 100, 100, 100);
	Color c = Color (1.0, 0.0, 0.5, 0.5);
	shape_set_stroke (r, new SolidColorBrush (c));
	cairo_matrix_init_rotate (&trans, 0.4);
	item_transform_set (r, (double *) (&trans));
	surface_repaint (t, 0, 0, 300, 300);
	
#ifdef VIDEO_DEMO
	v = video_new ("file:///tmp/BoxerSmacksdownInhoffe.wmv", 0, 0);
	//printf ("Got %d\n", v);
	item_transform_set (v, (double *) (&trans));
	panel_child_add (t, v);
#endif

	panel_child_add (t, r);

#ifdef VIDEO_DEMO1
	Item *v2 = video_new ("file:///tmp/Countdown-Colbert-BestNailings.wmv", 100, 100);
	//Item *v2 = video_new ("file:///tmp/BoxerSmacksdownInhoffe.wmv", 100, 100);
	panel_child_add (t, v2);
#endif

	printf ("set usize\n");
	gtk_widget_set_usize (w, 600, 400);
	printf ("show\n");
	gtk_widget_show_all (w);
	gtk_timeout_add (60, repaint, w);
	printf ("timeout set\n");
	//g_idle_add (repaint, w);
	printf ("entering main\n");
	gtk_main ();
}
