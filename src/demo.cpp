#define VIDEO_DEMO
#define XAML_DEMO
#include <string.h>
#include <gtk/gtk.h>
#include <malloc.h>
#include <glib.h>
#include <stdlib.h>
#include <unistd.h>

#include "runtime.h"

static UIElement *v;
static Rectangle *r;

static gboolean
animate (gpointer data)
{
	static double degree = 0;
	static double degree2 = 0;
	double trans [6];
	double trans2 [6];

	cairo_matrix_init_rotate ((cairo_matrix_t *)&trans, degree);
	degree += 0.01;

	if (v != NULL)
		item_set_transform (v, (double *) trans);

	cairo_matrix_init_rotate ((cairo_matrix_t *)&trans2, degree2);
	degree2 -= 0.1;
	item_set_transform (r, (double *) trans2);

	return TRUE;
}

int
main (int argc, char *argv [])
{
	GtkWidget *w, *w2, *box, *button;
	cairo_matrix_t trans;

	gtk_init (&argc, &argv);
	g_thread_init (NULL);
	gdk_threads_init ();
	

	w = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	// Create our objects
	Surface *t = surface_new (600, 600);
	gtk_container_add (GTK_CONTAINER(w), t->drawing_area);

	r = rectangle_new (50, 50, 50, 50);
	Color c = Color (1.0, 0.0, 0.5, 0.5);
	shape_set_stroke (r, new SolidColorBrush (c));

	Rectangle *r2 = new Rectangle (50, 50, 50, 50);
	shape_set_stroke (r2, new SolidColorBrush (c));
	panel_child_add (t, r2);

#ifdef XAML_DEMO
	panel_child_add (t, xaml_create_from_str ("<Line Stroke='Blue' X1='10' Y1='10' X2='10' Y2='300' />"));;
#endif

#ifdef VIDEO_DEMO
	v = video_new ("file:///tmp/BoxerSmacksdownInhoffe.wmv", 0, 0);
	item_set_transform_origin (v, Point (1, 1));
	printf ("Got %d\n", v);
	panel_child_add (t, v);
#endif

	panel_child_add (t, r);

#ifdef VIDEO_DEMO
	//UIElement *v2 = video_new ("file:///tmp/Countdown-Colbert-BestNailings.wmv", 100, 100);
	//UIElement *v2 = video_new ("file:///tmp/red.wmv", 100, 100);
	UIElement *v2 = video_new ("file:///tmp/BoxerSmacksdownInhoffe.wmv", 100, 100);
	panel_child_add (t, v2);
#endif

	printf ("set usize\n");
	gtk_widget_set_usize (w, 600, 400);
	printf ("show\n");
	gtk_widget_show_all (w);
	gtk_timeout_add (60, animate, NULL);
	printf ("timeout set\n");
	//g_idle_add (animate, w);
	printf ("entering main\n");
	gtk_main ();
}
