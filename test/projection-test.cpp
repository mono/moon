/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#define __MOON_GLX__
#include "context-glx.h"
#include "runtime.h"
#include "projection.h"

using namespace Moonlight;

struct projection {
	Context     *ctx;
	MoonSurface *target;
	MoonSurface *surface;
	Matrix3D    *matrix;
	int         count;
	int         frames;
};

const int width = 800;
const int height = 600;

gboolean
on_timeout (gpointer data)
{
	struct projection *p = (struct projection *) data;

	printf ("%d frames in 5.0 seconds = %f FPS\n", p->frames,
		p->frames / 5.0);
	p->frames = 0;

	return TRUE;
}

gboolean
on_expose_event (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	GdkDrawable       *drawable = GDK_DRAWABLE (widget->window);
	struct projection *p = (struct projection *) data;
	GLXSurface        *window = (GLXSurface *) p->target;
	GLXContext        *ctx = (GLXContext *) p->ctx;
	static double     max = p->count;
	int               width;
	int               height;

	gdk_drawable_get_size (drawable, &width, &height);
	if (width != window->Width () || height != window->Height ()) {
		Color color = Color ();

		window->Reshape (width, height);
		glDrawBuffer (GL_FRONT);
		glReadBuffer (GL_FRONT);
		ctx->Clear (&color);
	}

	ctx->Project (p->surface,
		      (double *) p->matrix->GetMatrixValues (),
		      0.1,
		      (double) rand () / RAND_MAX * width,
		      (double) rand () / RAND_MAX * height);
	ctx->Flush ();

	p->frames++;

	if (p->count-- > 0)
		gtk_widget_queue_draw (widget);
	else
		gtk_main_quit ();

	return TRUE;
}

int
main (int argc, char **argv)
{
	GtkWidget *window;
	struct projection p;
	DoubleCollection *values;
	Rect bounds = Rect (0, 0, width, height);
	Color color = Color (0.0, 0.0, 1.0, 1.0);

	if (argc < 2) {
		printf ("usage: %s MATRIX [COUNT]\n", argv[0]);
		return 1;
	}

	gtk_init (&argc, &argv);

	Runtime::InitDesktop ();

	values = DoubleCollection::FromStr (argv[1]);
	if (!values) {
		printf ("usage: %s MATRIX [COUNT]\n", argv[0]);
		return 1;
	}

	if (values->GetCount () != 16) {
		printf ("usage: %s MATRIX [COUNT]\n", argv[0]);
		values->unref ();
		return 1;
	}

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size (GTK_WINDOW (window), width, height); 
	gtk_widget_set_double_buffered (window, false);
	gtk_widget_realize (window);

	p.ctx = NULL;

	if (!p.ctx) {
		GLXContext  *ctx;
		GLXSurface  *target;
		GdkDrawable *drawable = GDK_DRAWABLE (window->window);
		XID         win = gdk_x11_drawable_get_xid (drawable);

		target = new GLXSurface (GDK_DISPLAY (), win);
		ctx = new GLXContext (target);
		p.target = target;
		p.ctx = ctx;

		if (!ctx->Initialize ())
			return 1;
	}

	if (!p.ctx) {
		CairoSurface *target;

		target = new CairoSurface (width, height);
		p.ctx = new CairoContext (target);
		p.target = target;
	}

	p.matrix = new Matrix3D ();
	p.matrix->SetM11 (values->GetValueAt (0)->AsDouble ());
	p.matrix->SetM12 (values->GetValueAt (1)->AsDouble ());
	p.matrix->SetM13 (values->GetValueAt (2)->AsDouble ());
	p.matrix->SetM14 (values->GetValueAt (3)->AsDouble ());
	p.matrix->SetM21 (values->GetValueAt (4)->AsDouble ());
	p.matrix->SetM22 (values->GetValueAt (5)->AsDouble ());
	p.matrix->SetM23 (values->GetValueAt (6)->AsDouble ());
	p.matrix->SetM24 (values->GetValueAt (7)->AsDouble ());
	p.matrix->SetM31 (values->GetValueAt (8)->AsDouble ());
	p.matrix->SetM32 (values->GetValueAt (9)->AsDouble ());
	p.matrix->SetM33 (values->GetValueAt (10)->AsDouble ());
	p.matrix->SetM34 (values->GetValueAt (11)->AsDouble ());
	p.matrix->SetOffsetX (values->GetValueAt (12)->AsDouble ());
	p.matrix->SetOffsetY (values->GetValueAt (13)->AsDouble ());
	p.matrix->SetOffsetZ (values->GetValueAt (14)->AsDouble ());
	p.matrix->SetM44 (values->GetValueAt (15)->AsDouble ());

	values->unref ();

	bounds = Rect (0, 0, 512, 512);

	p.ctx->Push (Context::Group (bounds));
	cairo_t *cr = p.ctx->Push (Context::Cairo ());
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
	p.ctx->Pop ();
	bounds = p.ctx->Pop (&p.surface);

	if (argc > 2)
		p.count = atoi (argv[2]);
	else
		p.count = 1;

	p.frames = 0;
	gtk_timeout_add (5000, on_timeout, &p);

	g_signal_connect (window,
			  "expose-event",
			  G_CALLBACK (on_expose_event),
			  &p);

	gtk_widget_show (window);
	gtk_main ();

	p.matrix->unref ();
	p.surface->unref ();
	p.target->unref ();
	delete p.ctx;

	Runtime::Shutdown ();

	return 0;
}
