/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#define __MOON_GLX__
#include "context-glx.h"
#include "runtime.h"
#include "effect.h"
#include "factory.h"

using namespace Moonlight;

struct effect {
	Context     *ctx;
	MoonSurface *target;
	MoonSurface *surface;
	PixelShader *shader;
	int         count;
	int         frames;
};

const int width = 800;
const int height = 600;

gboolean
on_timeout (gpointer data)
{
	struct effect *e = (struct effect *) data;

	printf ("%d frames in 5.0 seconds = %f FPS\n", e->frames,
		e->frames / 5.0);
	e->frames = 0;

	return TRUE;
}

gboolean
on_expose_event (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	GdkDrawable    *drawable = GDK_DRAWABLE (widget->window);
	struct effect  *e = (struct effect *) data;
	GLXSurface     *window = (GLXSurface *) e->target;
	GLXContext     *ctx = (GLXContext *) e->ctx;
	static double  max = e->count;
	int            width;
	int            height;

	gdk_drawable_get_size (drawable, &width, &height);
	if (width != window->Width () || height != window->Height ()) {
		Color color = Color ();

		window->Reshape (width, height);
		glDrawBuffer (GL_FRONT);
		glReadBuffer (GL_FRONT);
		ctx->Clear (&color);
	}

	ctx->ShaderEffect (e->surface,
			   e->shader,
			   NULL,
			   NULL,
			   0,
			   NULL,
			   0,
			   NULL,
			   (double) rand () / RAND_MAX * width,
			   (double) rand () / RAND_MAX * height);
	ctx->Flush ();

	e->frames++;

	if (e->count-- > 0)
		gtk_widget_queue_draw (widget);
	else
		gtk_main_quit ();

	return TRUE;
}

int
main (int argc, char **argv)
{
	GtkWidget *window;
	struct effect e;
	Rect bounds = Rect (0, 0, width, height);
	Color color = Color (0.0, 0.0, 1.0, 1.0);

	if (argc < 2) {
		printf ("usage: %s SHADERFILE [COUNT]\n", argv[0]);
		return 1;
	}

	gtk_init (&argc, &argv);

	runtime_init_desktop ();

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size (GTK_WINDOW (window), width, height); 
	gtk_widget_set_double_buffered (window, false);
	gtk_widget_realize (window);

	e.ctx = NULL;

	if (!e.ctx) {
		GLXContext  *ctx;
		GLXSurface  *target;
		GdkDrawable *drawable = GDK_DRAWABLE (window->window);
		XID         win = gdk_x11_drawable_get_xid (drawable);

		target = new GLXSurface (GDK_DISPLAY (), win);
		ctx = new GLXContext (target);
		e.target = target;
		e.ctx = ctx;

		if (!ctx->CheckVersion ())
			return 1;
	}

	if (!e.ctx) {
		cairo_surface_t *image;
		CairoSurface    *target;

		image = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
						    width,
						    height);
		target = new CairoSurface (image);
		cairo_surface_destroy (image);
		e.ctx = new CairoContext (target);
		e.target = target;
	}

	e.shader = MoonUnmanagedFactory::CreatePixelShader ();
	e.shader->SetTokensFromPath (argv[1]);

	bounds = Rect (0, 0, 512, 512);

	e.ctx->Push (Context::Group (bounds));
	cairo_t *cr = e.ctx->Cairo ();
	cairo_set_source_rgb (cr, 0.0, 0.0, 1.0);
	cairo_paint (cr);
	cairo_scale (cr, bounds.width, bounds.height);
	cairo_move_to (cr, 0.25, 0.25);
	cairo_line_to (cr, 0.5, 0.375);
	cairo_rel_line_to (cr, 0.25, -0.125);
	cairo_arc (cr, 0.5, 0.5, 0.25 * sqrt (2), -0.25 * M_PI, 0.25 * M_PI);
	cairo_rel_curve_to (cr, -0.25, -0.125, -0.25, 0.125, -0.5, 0);
	cairo_close_path (cr);
	cairo_set_source_rgb (cr, 1.0, 0.0, 0.0);
	cairo_fill (cr);
	bounds = e.ctx->Pop (&e.surface);

	if (argc > 2)
		e.count = atoi (argv[2]);
	else
		e.count = 1;

	e.frames = 0;
	gtk_timeout_add (5000, on_timeout, &e);

	g_signal_connect (window,
			  "expose-event",
			  G_CALLBACK (on_expose_event),
			  &e);

	gtk_widget_show (window);
	gtk_main ();

	e.shader->unref ();
	e.surface->unref ();
	e.target->unref ();
	delete e.ctx;

	runtime_shutdown ();

	return 0;
}
