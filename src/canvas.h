/*
 * canvas.h: canvas definitions.
 *
 * Author:
 *   Miguel de Icaza (miguel@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_CANVAS_H__
#define __MOON_CANVAS_H__

#include "panel.h"

class Surface;
//
// Canvas Class, the only purpose is to have the Left/Top properties that
// children can use
//
class Canvas : public Panel {
 public:
	Canvas ();

	//
	// if not-null, this is a toplevel canvas, and this points to the
	// surface
	//
	Surface *surface;

	virtual Surface *GetSurface ();

	//
	// Contains the last element where the mouse entered
	//
	UIElement *mouse_over;
	
	virtual Type::Kind GetObjectType () { return Type::CANVAS; }

	virtual Point GetTransformOrigin () { return Point (0, 0); }

	virtual void Render (cairo_t *cr, int x, int y, int width, int height);
	virtual void ComputeBounds ();
	virtual void UpdateTransform ();
	virtual void GetTransformFor (UIElement *item, cairo_matrix_t *result);

	bool CheckOver (cairo_t *cr, UIElement *item, double x, double y);

	virtual UIElement* FindMouseOver (cairo_t *cr, double x, double y);

	virtual bool InsideObject (cairo_t *cr, double x, double y);

	virtual void HandleMotion (Surface *s, cairo_t *cr, int state, double x, double y, MouseCursor *cursor);
	virtual void HandleButton (Surface *s, cairo_t *cr, callback_mouse_event cb, int state, double x, double y);
	virtual void Enter (Surface *s, cairo_t *cr, int state, double x, double y);
	virtual void Leave (Surface *s);
	
	virtual bool OnChildPropertyChanged (DependencyProperty *prop, DependencyObject *child);
	
	static DependencyProperty* TopProperty;
	static DependencyProperty* LeftProperty;
};

G_BEGIN_DECLS

Canvas *canvas_new (void);

void    canvas_init (void);
G_END_DECLS

#endif /* __MOON_CANVAS_H__ */
