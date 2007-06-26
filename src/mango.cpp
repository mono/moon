/*
 * mango.cpp: 
 *
 * Author: Jeffrey Stedfast <fejj@novell.com>
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>
#include <pango/pango.h>
#include <pango/pangocairo.h>

#include "brush.h"
#include "mango.h"


struct _MangoRendererPrivate {
	MangoAttrForeground *fg;
	cairo_t *cr;
	bool show;
};


static void mango_renderer_init (MangoRenderer *mango);
static void mango_renderer_finalize (GObject *object);

static void mango_renderer_draw_glyphs (PangoRenderer *renderer, PangoFont *font, PangoGlyphString *glyphs, int x, int y);
static void mango_renderer_prepare_run (PangoRenderer *renderer, PangoLayoutRun *run);
static void mango_renderer_begin (PangoRenderer *renderer);
static void mango_renderer_end (PangoRenderer *renderer);


static PangoAttrType mango_attr_foreground_type;


G_DEFINE_TYPE (MangoRenderer, mango_renderer, PANGO_TYPE_RENDERER)


static void
mango_renderer_class_init (MangoRendererClass *klass)
{
	PangoRendererClass *renderer_class = PANGO_RENDERER_CLASS (klass);
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	
	renderer_class->draw_glyphs = mango_renderer_draw_glyphs;
	renderer_class->prepare_run = mango_renderer_prepare_run;
	renderer_class->begin = mango_renderer_begin;
	renderer_class->end = mango_renderer_end;
	
	object_class->finalize = mango_renderer_finalize;
}

static void
mango_renderer_init (MangoRenderer *mango)
{
	mango->priv = g_new (MangoRendererPrivate, 1);
	mango->priv->show = false;
	mango->priv->cr = NULL;
	mango->priv->fg = NULL;
}

static void
mango_renderer_finalize (GObject *object)
{
	MangoRenderer *mango = (MangoRenderer *) object;
	MangoRendererPrivate *priv = mango->priv;
	
	if (priv->cr)
		cairo_destroy (priv->cr);
	
	g_free (priv);
	
	G_OBJECT_CLASS (mango_renderer_parent_class)->finalize (object);
}

static void
mango_renderer_draw_glyphs (PangoRenderer *renderer, PangoFont *font, PangoGlyphString *glyphs, int x, int y)
{
	MangoRenderer *mango = (MangoRenderer *) renderer;
	MangoRendererPrivate *priv = mango->priv;
	
	cairo_move_to (priv->cr, (double) x / PANGO_SCALE, (double) y / PANGO_SCALE);
	
	if (priv->show)
		pango_cairo_show_glyph_string (priv->cr, font, glyphs);
	else
		pango_cairo_glyph_string_path (priv->cr, font, glyphs);
}

#if 0
static void
mango_renderer_draw_rectangle (PangoRenderer *renderer, PangoRenderPart part,
			       int x, int y, int width, int height)
{
	MangoRenderer *mango = (MangoRenderer *) renderer;
	MangoRendererPrivate *priv = mango->priv;
	
	cairo_rectangle (priv->cr, (double) x / PANGO_SCALE, (double) y / PANGO_SCALE,
			 (double) width / PANGO_SCALE, (double) height / PANGO_SCALE);
	cairo_fill (priv->cr);
}

static void
mango_renderer_draw_error_underline (PangoRenderer *renderer, int x, int y, int width, int height)
{
	MangoRenderer *mango = (MangoRenderer *) renderer;
	MangoRendererPrivate *priv = mango->priv;
	
	pango_cairo_show_error_underline (priv->cr, (double) x / PANGO_SCALE, (double) y / PANGO_SCALE,
					  (double) width / PANGO_SCALE, (double) height / PANGO_SCALE);
}
#endif

static void
mango_renderer_begin (PangoRenderer *renderer)
{
	MangoRenderer *mango = (MangoRenderer *) renderer;
	MangoRendererPrivate *priv = mango->priv;
	
	priv->fg = NULL;
	
	if (!priv->cr)
		g_warning ("mango_renderer_set_cairo_context() must be used to set the cairo context before using the renderer");
}

static void
mango_renderer_end (PangoRenderer *renderer)
{
	MangoRenderer *mango = (MangoRenderer *) renderer;
	MangoRendererPrivate *priv = mango->priv;
	
	if (priv->cr) {
		cairo_destroy (priv->cr);
		priv->cr = NULL;
	}
	
	priv->fg = NULL;
}

static void
mango_renderer_prepare_run (PangoRenderer *renderer, PangoLayoutRun *run)
{
	MangoRenderer *mango = (MangoRenderer *) renderer;
	MangoRendererPrivate *priv = mango->priv;
	MangoAttrForeground *fg = NULL;
	GSList *node;
	
	for (node = run->item->analysis.extra_attrs; node; node = node->next) {
		PangoAttribute *attr = (PangoAttribute *) node->data;
		
		if (attr->klass->type == mango_attr_foreground_type)
			fg = (MangoAttrForeground *) attr;
	}
	
	if (fg && (!priv->fg || !pango_attribute_equal ((const PangoAttribute *) priv->fg, (const PangoAttribute *) fg))) {
		pango_renderer_part_changed (renderer, PANGO_RENDER_PART_STRIKETHROUGH);
		pango_renderer_part_changed (renderer, PANGO_RENDER_PART_FOREGROUND);
		pango_renderer_part_changed (renderer, PANGO_RENDER_PART_UNDERLINE);
		
		if (priv->show)
			fg->foreground->SetupBrush (priv->cr, fg->element);
		
		priv->fg = fg;
	}
	
	PANGO_RENDERER_CLASS (mango_renderer_parent_class)->prepare_run (renderer, run);
}


/**
 * mango_renderer_new:
 * 
 * Creates a new #PangoRenderer.
 *
 * Return value: a newly created #PangoRenderer. Free with g_object_unref()
 **/
PangoRenderer *
mango_renderer_new (void)
{
	return (PangoRenderer *) g_object_new (MOON_TYPE_MANGO_RENDERER, NULL);
}


void
mango_renderer_set_cairo_context (MangoRenderer *mango, cairo_t *cr)
{
	cairo_t *cairo = cairo_reference (cr);
	
	if (mango->priv->cr)
		cairo_destroy (mango->priv->cr);
	
	mango->priv->cr = cairo;
}


void
mango_renderer_show_layout (MangoRenderer *mango, PangoLayout *layout)
{
	PangoRenderer *renderer = (PangoRenderer *) mango;
	bool show = mango->priv->show;
	
	mango->priv->show = true;
	pango_renderer_draw_layout (renderer, layout, 0, 0);
	mango->priv->show = show;
}


void
mango_renderer_layout_path (MangoRenderer *mango, PangoLayout *layout)
{
	PangoRenderer *renderer = (PangoRenderer *) mango;
	bool show = mango->priv->show;
	
	mango->priv->show = false;
	pango_renderer_draw_layout (renderer, layout, 0, 0);
	mango->priv->show = show;
}


static PangoAttribute *
mango_attr_foreground_copy (const PangoAttribute *attr)
{
	const MangoAttrForeground *src = (const MangoAttrForeground *) attr;
	
	return mango_attr_foreground_new (src->element, src->foreground);
}

static void
mango_attr_foreground_destroy (PangoAttribute *attr)
{
	MangoAttrForeground *fg = (MangoAttrForeground *) attr;
	
	if (fg->element)
		fg->element->unref ();
	
	if (fg->foreground)
		fg->foreground->unref ();
	
	g_free (attr);
}

static gboolean
mango_attr_foreground_equal (const PangoAttribute *attr1, const PangoAttribute *attr2)
{
	const MangoAttrForeground *a = (const MangoAttrForeground *) attr1;
	const MangoAttrForeground *b = (const MangoAttrForeground *) attr2;
	
	return a->element == b->element && a->foreground == b->foreground;
}


/**
 * mango_attr_foreground_new:
 * @element: a #UIElement
 * @foreground: a #Brush
 *
 * Creates a new attribute containing a foreground brush to be used
 * when rendering the text.
 *
 * Return value: new #PangoAttribute
 **/
PangoAttribute *
mango_attr_foreground_new (UIElement *element, Brush *foreground)
{
	MangoAttrForeground *attr;
	static PangoAttrClass klass = {
		(PangoAttrType) 0,
		mango_attr_foreground_copy,
		mango_attr_foreground_destroy,
		mango_attr_foreground_equal
	};
	
	if (!klass.type) {
		klass.type = pango_attr_type_register ("MangoAttrForeground");
		mango_attr_foreground_type = klass.type;
	}
	
	attr = g_new (MangoAttrForeground, 1);
	attr->attr.klass = &klass;
	
	if (element)
		element->ref ();
	
	if (foreground)
		foreground->ref ();
	
	attr->element = element;
	attr->foreground = foreground;
	
	return (PangoAttribute *) attr;
}
