#include <gtk/gtk.h>
#include <libmoon.h>
#include <cairo.h>

#define TEST_WIDTH 400
#define TEST_HEIGHT 400

void
runTest (const char *xaml_file, const char *output_prefix)
{
	Surface *s = new Surface (TEST_WIDTH, TEST_HEIGHT);
	Type::Kind type;

	DependencyObject *dob = xaml_create_from_file (xaml_file, true, &type);

	s->Attach ((UIElement*)dob);

	cairo_surface_t* surf = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
							    s->GetWidth (), s->GetHeight ());

	cairo_t *cr = cairo_create (surf);

	s->Paint (cr, 0, 0, s->GetWidth(), s->GetHeight());

	cairo_surface_write_to_png (surf, output_prefix);

	cairo_surface_destroy (surf);
	cairo_destroy (cr);

	delete s;
}

int
main (int argc, char **argv)
{
	bool gen_expected = false;

	gtk_init (&argc, &argv);
	g_thread_init (NULL);
	gdk_threads_init ();
	runtime_init ();
	
	char *test = argv[1];
	char *png = argv[2];

	runTest (test, png);
}
