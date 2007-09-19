/*
 * mango.h: 
 *
 * Author: Jeffrey Stedfast <fejj@novell.com>
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __MANGO_H__
#define __MANGO_H__

G_BEGIN_DECLS

#include "brush.h"

GType mango_renderer_get_type_safe ();

#define MOON_TYPE_MANGO_RENDERER            (mango_renderer_get_type_safe ())
#define MANGO_RENDERER(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), MOON_TYPE_MANGO_RENDERER, MangoRenderer))
#define MOON_IS_MANGO_RENDERER(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOON_TYPE_MANGO_RENDERER))
#define MANGO_RENDERER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), MOON_TYPE_MANGO_RENDERER, MangoRendererClass))
#define MOON_IS_MANGO_RENDERER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MOON_TYPE_MANGO_RENDERER))
#define MANGO_RENDERER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), MOON_TYPE_MANGO_RENDERER, MangoRendererClass))

typedef struct _MangoRenderer        MangoRenderer;
typedef struct _MangoRendererClass   MangoRendererClass;
typedef struct _MangoRendererPrivate MangoRendererPrivate;

struct _MangoRenderer {
	/*< private >*/
	PangoRenderer parent_object;
	
	MangoRendererPrivate *priv;
};

struct _MangoRendererClass {
	/*< private >*/
	PangoRendererClass parent_class;
};


GType mango_renderer_get_type (void) G_GNUC_CONST;

PangoRenderer *mango_renderer_new (void);

void mango_renderer_set_cairo_context (MangoRenderer *mango, cairo_t *cr);
void mango_renderer_show_layout (MangoRenderer *mango, PangoLayout *layout);
void mango_renderer_layout_path (MangoRenderer *mango, PangoLayout *layout);


/* Attributes used to render text in Moonlight. */

typedef struct _MangoAttrForeground MangoAttrForeground;

struct _MangoAttrForeground {
	PangoAttribute attr;
	
	UIElement *element;
	Brush *foreground;
};

PangoAttribute *mango_attr_foreground_new (UIElement *element, Brush *foreground);

G_END_DECLS

#endif /* __MANGO_H__ */
