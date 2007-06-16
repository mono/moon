#define VIDEO_DEMO
#define XAML_DEMO
#include <config.h>
#include <string.h>
#include <gtk/gtk.h>
#include <malloc.h>
#include <glib.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include "runtime.h"
#include "transform.h"
#include "animation.h"
#include "shape.h"
#include "media.h"
#include "text.h"
#include "downloader.h"

static UIElement *v;
static Rectangle *r;
static Rectangle *square;

static RotateTransform *r_trans;
static RotateTransform *v_trans;
static ScaleTransform *s_trans;
static RotateTransform *t_trans;

static int do_fps = FALSE;
static uint64_t last_time;

static GtkWidget *w;

static Storyboard *sb = NULL;

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
	int64_t now = gettime ();
	int64_t diff = now - last_time;
	
	if (diff > 1000000) {
		float seconds = diff / 1000000.0;
		last_time = now;
		char *res = g_strdup_printf ("%.2f fps", s->frames / seconds);

		gtk_window_set_title (GTK_WINDOW (w), res);
		g_free (res);
		s->frames = 0;
	}

	return TRUE;
}

static gboolean
delete_event (GtkWidget *widget, GdkEvent *e, gpointer data)
{
	surface_destroy ((Surface*)data);
	runtime_shutdown ();
	gtk_main_quit ();
	return 1;
}

static void
button_press_event (GtkWidget *widget, GdkEventButton *e, gpointer data)
{
	if (sb)
		sb->Pause ();
	//sb->Seek ((TimeSpan)e->x * 100000);
}

static void
button_press_event2 (GtkWidget *widget, GdkEventButton *e, gpointer data)
{
	Surface *t = (Surface *) data;
	Value::Kind kind;

	printf ("button_press_event\n");

	printf ("Loading...\n");
	UIElement *ee = xaml_create_from_file ("../test/xaml/test-shape-ellipse.xaml", true, NULL, &kind);
	printf ("Loading... %p\n", ee);
	if (ee != NULL)
		surface_attach (t, ee);
}

static void
button_release_event (GtkWidget *widget, GdkEventButton *e, gpointer data)
{
  //	printf ("button_release_event\n");
	if (sb)
		sb->Resume ();
}

static void
button_motion_event (GtkWidget *widget, GdkEventMotion *e, gpointer data)
{
  //	printf ("button_motion_event\n");
	/* let's treat pixels as 1/10th of a second */
// 	if (sb)
// 		sb->Seek ((TimeSpan)e->x * 100000);
}

static gpointer downloader_create_state (Downloader* dl);
static void downloader_destroy_state (gpointer data);
static void downloader_open (char *verb, char *uri, bool async, gpointer state);
static void downloader_send (gpointer state);
static void downloader_abort (gpointer state);
static void downloader_abort (gpointer state);
static char* downloader_get_response_text (char *part, gpointer state);

static void
text_block_append_line_break (TextBlock *tb)
{
	Inlines *col = text_block_get_inlines (tb);
	LineBreak *lb = new LineBreak ();
	
	if (col == NULL) {
		col = new Inlines ();
		text_block_set_inlines (tb, col);
	}
	
	col->Add (lb);
}

static Run *
text_block_append_run (TextBlock *tb)
{
	Inlines *col = text_block_get_inlines (tb);
	Run *run = new Run ();
	
	if (col == NULL) {
		col = new Inlines ();
		text_block_set_inlines (tb, col);
	}
	
	col->Add (run);
	
	return run;
}

static MediaElement *
video_new (char *uri)
{
	MediaElement *video = new MediaElement ();
	media_base_set_source (video, uri);
	video->Play ();
	return video;
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

	downloader_set_functions (downloader_create_state,
				  downloader_destroy_state,
				  downloader_open,
				  downloader_send,
				  downloader_abort,
				  downloader_get_response_text);

	w = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	Surface *t = surface_new (600, 600);
	gtk_signal_connect (GTK_OBJECT (w), "delete_event", G_CALLBACK (delete_event), t);
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
		
		UIElement *e = xaml_create_from_file (file, true, NULL, &kind);
		if (e == NULL){
			printf ("Was not able to load the file\n");
			return 1;
		}
		if (kind != Value::CANVAS){
			printf ("Currently we only support Canvas toplevel elements\n");
			return 1;
		}

		gtk_signal_connect (GTK_OBJECT (t->drawing_area), "button_press_event", G_CALLBACK (button_press_event2), t);
		surface_attach (t, e);
	} else {
		NameScope *namescope = new NameScope();

		Canvas *canvas = new Canvas ();
		surface_attach (t, canvas);

		//Control control = new Control ();
		//control_initialize_from_xaml (control, "<Line X1='0' Y1='0' X2='100' Y2='100' Stroke='#80808080'/>");
		
		NameScope::SetNameScope (canvas, namescope);

		// Create our objects
		r_trans = new RotateTransform ();
		v_trans = new RotateTransform ();
		s_trans = new ScaleTransform ();
		t_trans = new RotateTransform ();
		
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
		
		TextBlock *tb = text_block_new ();
		text_block_set_font_size (tb, 24.0);
		text_block_set_font_weight (tb, FontWeightsBold);
		text_block_set_text (tb, "This is a Moonlight Demo");
#if 0
		Run *run;
		SolidColorBrush *blue_brush = new SolidColorBrush ();
		SolidColorBrush *magenta_brush = new SolidColorBrush ();
		Color *font_color = color_from_str ("SteelBlue");
		solid_color_brush_set_color (blue_brush, font_color);
		delete font_color;
		font_color = color_from_str ("Magenta");
		solid_color_brush_set_color (magenta_brush, font_color);
		delete font_color;
		
		text_block_append_line_break (tb);
		run = text_block_append_run (tb);
		inline_set_font_family (run, "Times New Roman");
		inline_set_foreground (run, blue_brush);
		inline_set_font_size (run, 10.0);
		run_set_text (run, "Brought to you by ");
		run = text_block_append_run (tb);
		inline_set_font_family (run, "Times New Roman");
		inline_set_font_style (run, FontStylesItalic);
		inline_set_foreground (run, magenta_brush);
		inline_set_font_size (run, 10.0);
		run_set_text (run, "The Fejjster");
		run = text_block_append_run (tb);
		inline_set_font_family (run, "Times New Roman");
		inline_set_foreground (run, blue_brush);
		inline_set_font_size (run, 10.0);
		run_set_text (run, " ...and by many other cool hackers");
#endif
		
		tb->SetValue (Canvas::LeftProperty, Value (75.0));
		tb->SetValue (Canvas::TopProperty, Value (175.0));
		item_set_transform_origin (tb, Point (0.5, 0.5));
		item_set_render_transform (tb, t_trans);
		panel_child_add (canvas, tb);
		
#ifdef XAML_DEMO
		panel_child_add (canvas, xaml_create_from_str ("<Line Stroke='Blue' X1='10' Y1='10' X2='10' Y2='300' />", false, NULL, NULL));
#endif
		
#ifdef VIDEO_DEMO
		v = (UIElement *) video_new ("file:///tmp/BoxerSmacksdownInhoffe.wmv");
		item_set_render_transform (v, v_trans);
		item_set_transform_origin (v, Point (1, 1));
		printf ("Got %d\n", v);
		panel_child_add (canvas, v);
#endif
		
		panel_child_add (canvas, r);
		
		Image *i = image_new ();
		i->SetValue (MediaBase::SourceProperty, Value ("/tmp/mono.png"));
		i->SetValue (Canvas::LeftProperty, Value (100.0));
		i->SetValue (Canvas::TopProperty, Value (100.0));
		item_set_render_transform (i, s_trans);
		panel_child_add (canvas, i);


		sb = new Storyboard ();

		canvas->GetValue(UIElement::ResourcesProperty)->AsResourceCollection()->Add (sb);

		DoubleAnimation *r_anim = new DoubleAnimation ();
		DoubleAnimation *t_anim = new DoubleAnimation ();
		DoubleAnimation *v_anim = new DoubleAnimation ();
		DoubleAnimation *sx_anim = new DoubleAnimation ();
		DoubleAnimation *sy_anim = new DoubleAnimation ();
		ColorAnimation *c_anim = new ColorAnimation ();

		// the scaled rectangle changes smoothly from Red to
		// Blue and back again
		namescope->RegisterName ("solid-color-brush", scb);

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
		namescope->RegisterName ("rect-transform", r_trans);
		r_anim->SetTo (-360.0);
		r_anim->SetRepeatBehavior (RepeatBehavior(2.0));
		r_anim->SetDuration (Duration::FromSeconds (4));
		sb->AddChild (r_anim);
		Storyboard::SetTargetName (r_anim, "rect-transform");
		Storyboard::SetTargetProperty (r_anim, "Angle");

		// The rotating video takes 5 seconds to complete the rotation
		namescope->RegisterName ("video-transform", v_trans);
		v_anim->SetTo (360.0);
		v_anim->SetRepeatBehavior (RepeatBehavior(5.0));
		v_anim->SetDuration (Duration::FromSeconds (5));
		sb->AddChild (v_anim);
		Storyboard::SetTargetName (v_anim, "video-transform");
		Storyboard::SetTargetProperty (v_anim, "Angle");
		
		// The text rotates completely around every 8
		// seconds, and stops after the third time around
		namescope->RegisterName ("text-transform", t_trans);
		t_anim->SetTo (360.0);
		t_anim->SetRepeatBehavior (RepeatBehavior (5.0));
		t_anim->SetDuration (Duration::FromSeconds (8));
		sb->AddChild (t_anim);
		Storyboard::SetTargetName (t_anim, "text-transform");
		Storyboard::SetTargetProperty (t_anim, "Angle");
		
		// for the scaled items, we scale X and Y differently,
		// the X scaling is completed in 6 seconds, and the y
		// in 7.
		namescope->RegisterName ("scale-transform", s_trans);

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

		namescope->RegisterName ("square-transform", square_trans);
		
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

		gtk_widget_add_events (t->drawing_area, GDK_BUTTON1_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);

		gtk_signal_connect (GTK_OBJECT (t->drawing_area), "button_press_event", G_CALLBACK (button_press_event), NULL);
		gtk_signal_connect (GTK_OBJECT (t->drawing_area), "button_release_event", G_CALLBACK (button_release_event), NULL);
		gtk_signal_connect (GTK_OBJECT (t->drawing_area), "motion_notify_event", G_CALLBACK (button_motion_event), NULL);

		sb->Begin ();
	}		
	if (do_fps){
		t->frames = 0;
		last_time = gettime ();
		gtk_timeout_add (1000, invalidator, t);
	}

	gtk_widget_set_usize (w, 600, 400);
	gtk_widget_show_all (w);
	gtk_main ();
}




class FileDownloadState {
 public:
	FileDownloadState (Downloader *dl) : downloader(dl), fd (-1), async_idle (-1) { }

	virtual ~FileDownloadState () { Close (); }

	void Abort () { Close (); }
	char* GetResponseText (char* PartName) { return NULL; } // XXX
	void Open (char *verb, char *uri, bool async)
	{
		fd = open (uri, O_RDONLY);
		if (fd == -1) {
			printf ("failed open\n");
			return;
		}

		struct stat sb;
		fstat (fd, &sb);
		downloader_notify_size (downloader, sb.st_size);
		async_idle = g_idle_add (async_fill_buffer, this);
	}

	void Send () { }

	void Close ()
	{
		if (fd != -1) {
			close (fd);
			fd = -1;
		}
		if (async_idle != -1) { 
			g_source_remove (async_idle);
			async_idle = -1;
		}
	}
 private:
	int fd;
	int async_idle;
	Downloader *downloader;
	guchar buf[8192];

	gboolean AsyncFillBuffer ()
	{
		int n = read (fd, buf, sizeof (buf));

		if (n >= 0)
			downloader_write (downloader, buf, 0, n);

		return n > 0;
	}

	static gboolean async_fill_buffer (gpointer cb_data)
	{
		return ((FileDownloadState*)cb_data)->AsyncFillBuffer ();
	}
};

static gpointer
downloader_create_state (Downloader *dl)
{
	return new FileDownloadState (dl);
}

static void
downloader_destroy_state (gpointer data)
{
	delete ((FileDownloadState*)data);
}

static void
downloader_open (char *verb, char *uri, bool async, gpointer state)
{
	((FileDownloadState*)state)->Open (verb, uri, async);
}

static void
downloader_send (gpointer state)
{
	((FileDownloadState*)state)->Send ();
}

static void
downloader_abort (gpointer state)
{
	((FileDownloadState*)state)->Abort ();
}

static char*
downloader_get_response_text (char *part, gpointer state)
{
	return ((FileDownloadState*)state)->GetResponseText (part);
}

