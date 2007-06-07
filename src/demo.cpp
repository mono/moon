#define VIDEO_DEMO
#define XAML_DEMO
#include <string.h>
#include <gtk/gtk.h>
#include <malloc.h>
#include <glib.h>
#include <stdlib.h>
#include <unistd.h>

#include "runtime.h"
#include "transform.h"
#include "shape.h"

static UIElement *v;
static Rectangle *r;

static RotateTransform *r_trans;
static RotateTransform *v_trans;
static ScaleTransform *s_trans;
static TranslateTransform *t_trans;

static gboolean
animate (gpointer data)
{
	rotate_transform_set_angle (v_trans,
				    rotate_transform_get_angle (v_trans) + 3);

	rotate_transform_set_angle (r_trans,
				    rotate_transform_get_angle (r_trans) - 3);
	static double scale_change = -0.02;

	scale_transform_set_scale_x (s_trans,
				     scale_transform_get_scale_x (s_trans) + scale_change);
	scale_transform_set_scale_y (s_trans,
				     scale_transform_get_scale_y (s_trans) + scale_change);

	if ((scale_change < 0 && scale_transform_get_scale_x (s_trans) < 0.3)
	    || (scale_change > 0 && scale_transform_get_scale_x (s_trans) > 0.75)) {
	    scale_change = -scale_change;
	}

	return TRUE;
}

static gboolean
delete_event (GtkWidget *widget, GdkEvent *e, gpointer data)
{
	gtk_main_quit ();
	return 1;
}

int
main (int argc, char *argv [])
{
	GtkWidget *w, *w2, *box, *button;
	cairo_matrix_t trans;
	double dash = 3.5;

	gtk_init (&argc, &argv);
	g_thread_init (NULL);
	gdk_threads_init ();
	runtime_init ();

	w = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_signal_connect (GTK_OBJECT (w), "delete_event", G_CALLBACK (delete_event), NULL);
	Surface *t = surface_new (600, 600);
	gtk_container_add (GTK_CONTAINER(w), t->drawing_area);
		

	if (argc == 2){
		gtk_window_set_title (GTK_WINDOW (w), argv [1]);

		UIElement *e = xaml_create_from_file (argv [1]);
		if (e == NULL){
			printf ("Was not able to load the file\n");
		}

		surface_attach (t, e);
		
	} else {
		Canvas *canvas = new Canvas ();
		surface_attach (t, canvas);

		// Create our objects
		r_trans = new RotateTransform ();
		v_trans = new RotateTransform ();
		s_trans = new ScaleTransform ();
		t_trans = new TranslateTransform ();
		
		r = rectangle_new ();
		framework_element_set_width (r, 50.0);
		framework_element_set_height (r, 50.0);
		shape_set_stroke_thickness (r, 10.0);
		r->SetValue (Canvas::LeftProperty, Value (50.0));
		r->SetValue (Canvas::TopProperty, Value (50.0));
		
		rectangle_set_radius_x (r, 10);
		rectangle_set_radius_y (r, 20);
		item_set_render_transform (r, s_trans);
		Color *c = new Color (1.0, 0.0, 0.5, 0.5);
		SolidColorBrush *scb = new SolidColorBrush ();
		solid_color_brush_set_color (scb, c);
		shape_set_stroke (r, scb);
		Color *c2 = new Color (0.5, 0.5, 0.0, 0.25);
		SolidColorBrush *scb2 = new SolidColorBrush ();
		solid_color_brush_set_color (scb2, c2);
		shape_set_fill (r, scb2);
		
		Rectangle *r2 = rectangle_new ();
		framework_element_set_width (r2, 50.0);
		framework_element_set_height (r2, 50.0);
		shape_set_stroke_dash_array (r2, &dash, 1);
		r2->SetValue (Canvas::LeftProperty, Value (50.0));
		r2->SetValue (Canvas::TopProperty, Value (50.0));
		item_set_render_transform (r2, r_trans);
		shape_set_stroke (r2, scb);
		panel_child_add (canvas, r2);
		
#ifdef XAML_DEMO
		panel_child_add (canvas, xaml_create_from_str ("<Line Stroke='Blue' X1='10' Y1='10' X2='10' Y2='300' />"));
#endif
		
#ifdef VIDEO_DEMO
		v = video_new ("file:///tmp/BoxerSmacksdownInhoffe.wmv");
		item_set_render_transform (v, v_trans);
		item_set_transform_origin (v, Point (1, 1));
		printf ("Got %d\n", v);
		panel_child_add (canvas, v);
#endif
		
		panel_child_add (canvas, r);
		
#ifdef VIDEO_DEMO
		//UIElement *v2 = video_new ("file:///tmp/Countdown-Colbert-BestNailings.wmv", 100, 100);
		//UIElement *v2 = video_new ("file:///tmp/red.wmv", 100, 100);
		UIElement *v2 = video_new ("file:///tmp/BoxerSmacksdownInhoffe.wmv");
		v2->SetValue (Canvas::LeftProperty, Value (100.0));
		v2->SetValue (Canvas::TopProperty, Value (100.0));
		item_set_render_transform (v2, s_trans);
		panel_child_add (canvas, v2);
#endif
		gtk_timeout_add (60, animate, NULL);
	}		
	gtk_widget_set_usize (w, 600, 400);
	gtk_widget_show_all (w);
	gtk_main ();
}
