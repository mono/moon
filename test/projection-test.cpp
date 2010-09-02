/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include "runtime.h"
#include "effect.h"
#include "factory.h"

using namespace Moonlight;

const int width = 256;
const int height = 256;

int frames = 0;

void
projection_alarm_handler (int sig)
{
	printf ("%d frames in 5.0 seconds = %f FPS\n", frames, frames / 5.0);
	frames = 0;
	alarm (5);
}

int
main (int argc, char **argv)
{
	cairo_t *cr;
	MoonSurface *surface;
	cairo_surface_t *dst, *src;
	TransformEffect *effect;
	Matrix3D *matrix;
	DoubleCollection *values;
	int stride = width * 4;
	Rect bounds = Rect (0, 0, width, height);
	gpointer data;
	bool status = true;
	int count = 1;

	if (argc < 2) {
		printf ("usage: %s MATRIX [COUNT]\n", argv[0]);
		return 1;
	}

	gtk_init (&argc, &argv);

	runtime_init_desktop ();

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

	data = g_malloc0 (height * stride);
	dst = cairo_image_surface_create_for_data ((unsigned char *) data,
						   CAIRO_FORMAT_ARGB32,
						   width, height, stride);
	src = cairo_surface_create_similar (dst,
					    CAIRO_CONTENT_COLOR_ALPHA,
					    width, height);
	surface = new CairoSurface (src);
	cairo_surface_destroy (src);
	cr = cairo_create (dst);

	effect = new TransformEffect ();

	matrix = new Matrix3D ();
	matrix->SetM11 (values->GetValueAt (0)->AsDouble ());
	matrix->SetM12 (values->GetValueAt (1)->AsDouble ());
	matrix->SetM13 (values->GetValueAt (2)->AsDouble ());
	matrix->SetM14 (values->GetValueAt (3)->AsDouble ());
	matrix->SetM21 (values->GetValueAt (4)->AsDouble ());
	matrix->SetM22 (values->GetValueAt (5)->AsDouble ());
	matrix->SetM23 (values->GetValueAt (6)->AsDouble ());
	matrix->SetM24 (values->GetValueAt (7)->AsDouble ());
	matrix->SetM31 (values->GetValueAt (8)->AsDouble ());
	matrix->SetM32 (values->GetValueAt (9)->AsDouble ());
	matrix->SetM33 (values->GetValueAt (10)->AsDouble ());
	matrix->SetM34 (values->GetValueAt (11)->AsDouble ());
	matrix->SetOffsetX (values->GetValueAt (12)->AsDouble ());
	matrix->SetOffsetY (values->GetValueAt (13)->AsDouble ());
	matrix->SetOffsetZ (values->GetValueAt (14)->AsDouble ());
	matrix->SetM44 (values->GetValueAt (15)->AsDouble ());

	values->unref ();

	if (argc > 2) {
		count = atoi (argv[2]);
		if (count > 1) {
			signal (SIGALRM, projection_alarm_handler);
			alarm (5);
		}
	}

	while (status && count-- > 0) {
		status = effect->Render (cr,
					 surface,
					 (double *) matrix->GetMatrixValues (),
					 0, 0, width, height);

		frames++;
	}

	matrix->unref ();

	cairo_destroy (cr);
	surface->unref ();
	cairo_surface_destroy (dst);
	g_free (data);

	runtime_shutdown ();

	return status != true;
}
