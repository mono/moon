/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
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

#include <glib.h>

#include "panel.h"

//
// Canvas Class, the only purpose is to have the Left/Top properties that
// children can use
//
class Canvas : public Panel {
 protected:
	virtual ~Canvas () {}
	
 public:
	static DependencyProperty *LeftProperty;
	static DependencyProperty *TopProperty;
	
	Canvas ();
	
	virtual Type::Kind GetObjectType () { return Type::CANVAS; }

	virtual Point GetTransformOrigin ();

	virtual void ComputeBounds ();
	virtual void GetTransformFor (UIElement *item, cairo_matrix_t *result);

	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
};

G_BEGIN_DECLS
Canvas *canvas_new (void);

void    canvas_init (void);
G_END_DECLS

#endif /* __MOON_CANVAS_H__ */
