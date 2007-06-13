#define VIDEO_DEMO
#define XAML_DEMO
#include <string.h>
#include <gtk/gtk.h>
#include <malloc.h>
#include <glib.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "runtime.h"
#include "transform.h"
#include "animation.h"
#include "shape.h"

static UIElement *v;
static Rectangle *r;
static Rectangle *square;

static RotateTransform *r_trans;
static RotateTransform *v_trans;
static ScaleTransform *s_trans;

extern NameScope *global_NameScope;

static int do_fps = FALSE;
static uint64_t last_time;

static GtkWidget *w;

static Storyboard *sb;

static int64_t
gettime (void)
{
    struct timeval tv;

    gettimeofday (&tv, NULL);

    return (int64_t) tv.tv_sec * 1000000 + tv.tv_usec;
}

static gboolean
my_expose_event (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
}

static gboolean
invalidator (gpointer data)
{
	Surface *s = (Surface *) data;

	gtk_widget_queue_draw (s->drawing_area);

	int64_t now = gettime ();

	int64_t diff = now - last_time;
	if ((now - last_time) > 1000000){
		last_time = now;
		char *res = g_strdup_printf ("%d fps", s->frames);

		gtk_window_set_title (GTK_WINDOW (w), res);
		g_free (res);
		s->frames = 0;
	}

	return TRUE;
}

static gboolean
delete_event (GtkWidget *widget, GdkEvent *e, gpointer data)
{
	gtk_main_quit ();
	return 1;
}

static void
button_press_event (GtkWidget *widget, GdkEventButton *e, gpointer data)
{
  //	printf ("button_press_event\n");
	sb->Pause ();
	sb->Seek ((TimeSpan)e->x * 100000);
}

static void
button_release_event (GtkWidget *widget, GdkEventButton *e, gpointer data)
{
  //	printf ("button_release_event\n");
	sb->Resume ();
}

static void
button_motion_event (GtkWidget *widget, GdkEventMotion *e, gpointer data)
{
  //	printf ("button_motion_event\n");
	/* let's treat pixels as 1/10th of a second */
	sb->Seek ((TimeSpan)e->x * 100000);
}

int
main (int argc, char *argv [])
{
	GtkWidget *w2, *box, *button;
	cairo_matrix_t trans;
	double dash = 3.5;
	char *file = NULL;

	gtk_init (&argc, &argv);
	g_thread_init (NULL);
	gdk_threads_init ();
	runtime_init ();

	TimeManager::Instance()->Start();

	w = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_signal_connect (GTK_OBJECT (w), "delete_event", G_CALLBACK (delete_event), NULL);
	Surface *t = surface_new (600, 600);
	gtk_container_add (GTK_CONTAINER(w), t->drawing_area);
		
	for (int i = 1; i < argc; i++){
		if (strcmp (argv [i], "-h") == 0){
			printf ("usage is: demo [-fps] [file.xaml]\n");
			return 0;
		} else if (strcmp (argv [i], "-fps")== 0){
			do_fps = TRUE;
		}else
			file = argv [i];
	}

	if (file){
		Value::Kind kind;

		gtk_window_set_title (GTK_WINDOW (w), file);
		
		UIElement *e = xaml_create_from_file (file, &kind);
		if (e == NULL){
			printf ("Was not able to load the file\n");
			return 1;
		}
		if (kind != Value::CANVAS){
			printf ("Currently we only support Canvas toplevel elements\n");
			return 1;
		}

		surface_attach (t, e);
	} else {
		Canvas *canvas = new Canvas ();
		surface_attach (t, canvas);

		// Create our objects
		r_trans = new RotateTransform ();
		v_trans = new RotateTransform ();
		s_trans = new ScaleTransform ();

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
		panel_child_add (canvas, xaml_create_from_str ("<Line Stroke='Blue' X1='10' Y1='10' X2='10' Y2='300' />", NULL));
#endif
		
#ifdef VIDEO_DEMO
		v = (UIElement *) video_new ("file:///tmp/BoxerSmacksdownInhoffe.wmv");
		item_set_render_transform (v, v_trans);
		item_set_transform_origin (v, Point (1, 1));
		printf ("Got %d\n", v);
		panel_child_add (canvas, v);
#endif
		
		panel_child_add (canvas, r);
		
#ifdef VIDEO_DEMO
		//UIElement *v2 = video_new ("file:///tmp/Countdown-Colbert-BestNailings.wmv");
		//UIElement *v2 = video_new ("file:///tmp/red.wmv", 100, 100);
		UIElement *v2 = (UIElement *) video_new ("file:///tmp/BoxerSmacksdownInhoffe.wmv");
		v2->SetValue (Canvas::LeftProperty, Value (100.0));
		v2->SetValue (Canvas::TopProperty, Value (100.0));
		item_set_render_transform (v2, s_trans);
		panel_child_add (canvas, v2);
#endif


		sb = new Storyboard ();

		DoubleAnimation *r_anim = new DoubleAnimation ();
		DoubleAnimation *v_anim = new DoubleAnimation ();
		DoubleAnimation *sx_anim = new DoubleAnimation ();
		DoubleAnimation *sy_anim = new DoubleAnimation ();
		ColorAnimation *c_anim = new ColorAnimation ();

		// the scaled rectangle changes smoothly from Red to
		// Blue and back again
		global_NameScope->RegisterName ("solid-color-brush", scb);

		c_anim->SetFrom (Color (1.0, 0.0, 0.0, 0.5));
		c_anim->SetTo (Color (0.0, 0.0, 1.0, 0.5));
		c_anim->SetRepeatBehavior (RepeatBehavior (10.0));
		c_anim->SetDuration (Duration::FromSeconds (2));
		c_anim->SetAutoReverse (true);
		sb->AddChild (c_anim);
		Storyboard::SetTargetName (c_anim, "solid-color-brush");
		Storyboard::SetTargetProperty (c_anim, "Color");

		// The rectangle rotates completely around every 4
		// seconds, and stops after the second time around
		global_NameScope->RegisterName ("rect-transform", r_trans);
		r_anim->SetTo (-360.0);
		r_anim->SetRepeatBehavior (RepeatBehavior(2.0));
		r_anim->SetDuration (Duration::FromSeconds (4));
		sb->AddChild (r_anim);
		Storyboard::SetTargetName (r_anim, "rect-transform");
		Storyboard::SetTargetProperty (r_anim, "Angle");

		// The rotating video takes 5 seconds to complete the rotation
		global_NameScope->RegisterName ("video-transform", v_trans);
		v_anim->SetTo (360.0);
		v_anim->SetRepeatBehavior (RepeatBehavior(5.0));
		v_anim->SetDuration (Duration::FromSeconds (5));
		sb->AddChild (v_anim);
		Storyboard::SetTargetName (v_anim, "video-transform");
		Storyboard::SetTargetProperty (v_anim, "Angle");

		// for the scaled items, we scale X and Y differently,
		// the X scaling is completed in 6 seconds, and the y
		// in 7.
		global_NameScope->RegisterName ("scale-transform", s_trans);

		sx_anim->SetFrom (1.0);
		sx_anim->SetBy (-0.5);
		sx_anim->SetRepeatBehavior (RepeatBehavior (4.0));
		sx_anim->SetDuration (Duration::FromSeconds (6));
		sx_anim->SetAutoReverse (true);
		sb->AddChild (sx_anim);
		Storyboard::SetTargetName (sx_anim, "scale-transform");
		Storyboard::SetTargetProperty (sx_anim, "ScaleX");

		sy_anim->SetFrom (1.0);
		sy_anim->SetTo (0.0);
		sy_anim->SetRepeatBehavior (RepeatBehavior(4.0));
		sy_anim->SetDuration (Duration::FromSeconds (7));
		sy_anim->SetAutoReverse (true);
		sb->AddChild (sy_anim);
		Storyboard::SetTargetName (sy_anim, "scale-transform");
		Storyboard::SetTargetProperty (sy_anim, "ScaleY");

#define KEYFRAMES 1

#if KEYFRAMES
		square = rectangle_new ();
		framework_element_set_width (square, 50.0);
		framework_element_set_height (square, 50.0);
		shape_set_stroke_thickness (square, 10.0);
		square->SetValue (Canvas::LeftProperty, Value (100.0));
		square->SetValue (Canvas::TopProperty, Value (50.0));

		Transform *square_trans = new TranslateTransform ();
		square_trans->SetValue (TranslateTransform::XProperty, Value(50.0));
		square_trans->SetValue (TranslateTransform::YProperty, Value(50.0));

		item_set_render_transform (square, square_trans);
		
		shape_set_stroke (square, scb);
		panel_child_add (canvas, square);

		global_NameScope->RegisterName ("square-transform", square_trans);
		
		DoubleAnimationUsingKeyFrames* square_x_anim = new DoubleAnimationUsingKeyFrames ();
		DoubleAnimationUsingKeyFrames* square_y_anim = new DoubleAnimationUsingKeyFrames ();

		square_y_anim->SetDuration (Duration::FromSeconds (8));
		square_y_anim->SetRepeatBehavior (RepeatBehavior (4.0));
		square_x_anim->SetDuration (Duration::FromSeconds (8));
		square_x_anim->SetRepeatBehavior (RepeatBehavior (4.0));

		LinearDoubleKeyFrame *frame;

		// the X keyframes for the 4 points of the rectangle
		frame = new LinearDoubleKeyFrame ();
		frame->SetValue (100.0);
		frame->SetKeyTime (KeyTime::FromTimeSpan ((TimeSpan)2 * 1000000));
		square_x_anim->AddKeyFrame (frame);

		frame = new LinearDoubleKeyFrame ();
		frame->SetValue (100.0);
		frame->SetKeyTime (KeyTime::FromTimeSpan ((TimeSpan)4 * 1000000));
		square_x_anim->AddKeyFrame (frame);

		frame = new LinearDoubleKeyFrame ();
		frame->SetValue (50.0);
		frame->SetKeyTime (KeyTime::FromTimeSpan ((TimeSpan)6 * 1000000));
		square_x_anim->AddKeyFrame (frame);

		frame = new LinearDoubleKeyFrame ();
		frame->SetValue (50.0);
		frame->SetKeyTime (KeyTime::FromTimeSpan ((TimeSpan)8 * 1000000));
		square_x_anim->AddKeyFrame (frame);

		sb->AddChild (square_x_anim);
		Storyboard::SetTargetName (square_x_anim, "square-transform");
		Storyboard::SetTargetProperty (square_x_anim, "X");

		// the Y keyframes for the 4 points of the rectangle
		frame = new LinearDoubleKeyFrame ();
		frame->SetValue (50.0);
		frame->SetKeyTime (KeyTime::FromTimeSpan ((TimeSpan)2 * 1000000));
		square_y_anim->AddKeyFrame (frame);

		frame = new LinearDoubleKeyFrame ();
		frame->SetValue (100.0);
		frame->SetKeyTime (KeyTime::FromTimeSpan ((TimeSpan)4 * 1000000));
		square_y_anim->AddKeyFrame (frame);

		frame = new LinearDoubleKeyFrame ();
		frame->SetValue (100.0);
		frame->SetKeyTime (KeyTime::FromTimeSpan ((TimeSpan)6 * 1000000));
		square_y_anim->AddKeyFrame (frame);

		frame = new LinearDoubleKeyFrame ();
		frame->SetValue (50.0);
		frame->SetKeyTime (KeyTime::FromTimeSpan ((TimeSpan)8 * 1000000));
		square_y_anim->AddKeyFrame (frame);

		sb->AddChild (square_y_anim);
		Storyboard::SetTargetName (square_y_anim, "square-transform");
		Storyboard::SetTargetProperty (square_y_anim, "Y");
#endif


		sb->Begin ();
	}		
	if (do_fps){
		t->frames = 0;
		last_time = gettime ();
		gtk_idle_add (invalidator, t);
	}

	gtk_widget_add_events (t->drawing_area, GDK_BUTTON1_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);

	gtk_signal_connect (GTK_OBJECT (t->drawing_area), "button_press_event", G_CALLBACK (button_press_event), NULL);
	gtk_signal_connect (GTK_OBJECT (t->drawing_area), "button_release_event", G_CALLBACK (button_release_event), NULL);
	gtk_signal_connect (GTK_OBJECT (t->drawing_area), "motion_notify_event", G_CALLBACK (button_motion_event), NULL);

	gtk_widget_set_usize (w, 600, 400);
	gtk_widget_show_all (w);
	gtk_main ();
}
