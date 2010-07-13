#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include "runtime.h"
#include "effect.h"

using namespace Moonlight;

class CustomEffect : public ShaderEffect {};

const int width = 256;
const int height = 256;

int
main (int argc, char **argv)
{
	cairo_surface_t *dst, *src;
	CustomEffect *effect;
	PixelShader *shader;
	int stride = width * 4;
        Rect bounds = Rect (0, 0, width, height);
	gpointer data;
	bool status;

	if (argc < 2) {
		printf ("usage: %s SHADERFILE\n", argv[0]);
		return 1;
	}

	gtk_init (&argc, &argv);

	runtime_init_desktop ();

	data = g_malloc0 (height * stride);
        dst = cairo_image_surface_create_for_data ((unsigned char *) data,
                                                   CAIRO_FORMAT_ARGB32,
                                                   width, height, stride);
	src = cairo_surface_create_similar (dst,
					    CAIRO_CONTENT_COLOR_ALPHA,
					    width, height);

	effect = new CustomEffect ();
	shader = new PixelShader ();

	shader->SetTokensFromPath (argv[1]);
	effect->SetPixelShader (shader);

	status = effect->Composite (dst, src, &bounds, 0, 0);

	effect->unref ();
	shader->unref ();

	cairo_surface_destroy (src);
	cairo_surface_destroy (dst);
	g_free (data);

	runtime_shutdown ();

	return status != true;
}
