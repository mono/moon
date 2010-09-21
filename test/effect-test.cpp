/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <gtk/gtk.h>
#ifdef USE_GALLIUM
#define __MOON_GALLIUM__
#include "context-gallium.h"
extern "C" {
#include "pipe/p_screen.h"
#ifdef CLAMP
#undef CLAMP
#endif
#include "util/u_inlines.h"
#include "util/u_simple_screen.h"
#include "util/u_debug.h"
#define template templat
#include "state_tracker/sw_winsys.h"
#undef template
#include "sw/null/null_sw_winsys.h"
#include "softpipe/sp_public.h"
#ifdef USE_LLVM
#include "llvmpipe/lp_public.h"
#endif
};
static struct pipe_screen *
swrast_create_screen (struct sw_winsys *ws)
{
	const char         *default_driver;
	const char         *driver;
	struct pipe_screen *screen = NULL;

#ifdef USE_LLVM
	default_driver = "llvmpipe";
#else
	default_driver = "softpipe";
#endif

	driver = debug_get_option ("GALLIUM_DRIVER", default_driver);

#ifdef USE_LLVM
	if (screen == NULL && strcmp (driver, "llvmpipe") == 0)
		screen = llvmpipe_create_screen (ws);
#endif

	if (screen == NULL)
		screen = softpipe_create_screen (ws);

	return screen;
}
struct pipe_screen *screen;
#endif
#include "runtime.h"
#include "effect.h"
#include "factory.h"

using namespace Moonlight;

class CustomEffect : public ShaderEffect {};

const int width = 256;
const int height = 256;

int frames = 0;

static void
effect_alarm_handler (int sig)
{
	printf ("%d frames in 5.0 seconds = %f FPS\n", frames, frames / 5.0);
	frames = 0;
	alarm (5);
}

int
main (int argc, char **argv)
{
	Context *ctx = NULL;
	MoonSurface *surface;
	cairo_surface_t *dst, *src;
	CustomEffect *effect;
	PixelShader *shader;
	Rect bounds = Rect (0, 0, width, height);
	Color color = Color (1.0, 0.0, 0.0, 1.0);
	int count = 1;

	if (argc < 2) {
		printf ("usage: %s SHADERFILE [COUNT]\n", argv[0]);
		return 1;
	}

	gtk_init (&argc, &argv);

	runtime_init_desktop ();

#ifdef USE_GALLIUM
	if (!ctx) {
		struct sw_winsys     *sw;
		struct pipe_resource pt, *texture;
		GalliumSurface       *target;

		sw = null_sw_create ();
		screen = swrast_create_screen (sw);

		memset (&pt, 0, sizeof (pt));
		pt.target = PIPE_TEXTURE_2D;
		pt.format = PIPE_FORMAT_B8G8R8A8_UNORM;
		pt.width0 = width;
		pt.height0 = height;
		pt.depth0 = 1;
		pt.last_level = 0;
		pt.bind = PIPE_BIND_RENDER_TARGET | PIPE_BIND_TRANSFER_WRITE |
			PIPE_BIND_TRANSFER_READ;

		texture = screen->resource_create (screen, &pt);

		target = new GalliumSurface (texture);
		pipe_resource_reference (&texture, NULL);
		ctx = new GalliumContext (target);
		target->unref ();
	}
#endif

	if (!ctx) {
		cairo_surface_t *image;
		CairoSurface    *target;

		image = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
						    width,
						    height);
		target = new CairoSurface (image);
		cairo_surface_destroy (image);
		ctx = new CairoContext (target);
		target->unref ();
	}

	effect = new CustomEffect ();
	shader = MoonUnmanagedFactory::CreatePixelShader ();

	shader->SetTokensFromPath (argv[1]);
	effect->SetPixelShader (shader);

	ctx->Push (Context::Group (bounds));
	ctx->Clear (&color);
	bounds = ctx->Pop (&surface);

	if (argc > 2) {
		count = atoi (argv[2]);
		if (count > 1) {
			signal (SIGALRM, effect_alarm_handler);
			alarm (5);
		}
	}

	while (count-- > 0) {
		effect->Render (ctx,
				surface,
				(double *) NULL,
				0, 0, width, height);

		frames++;
	}

	effect->unref ();
	shader->unref ();

	surface->unref ();
	delete ctx;

#ifdef USE_GALLIUM
	(*screen->destroy) (screen);
#endif

	runtime_shutdown ();

	return 0;
}
