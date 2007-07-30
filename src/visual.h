/*
 * visual.h
 *
 * Author:
 *   Miguel de Icaza (miguel@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_VISUAL_H__
#define __MOON_VISUAL_H__

#include "dependencyobject.h"

class Visual : public DependencyObject {
 public:
	Visual () {};
	virtual Type::Kind GetObjectType () { return Type::VISUAL; };	

	//
	// InsideObject:
	//   Returns whether the position x, y is inside the object
	//
	virtual bool InsideObject (cairo_t *cr, double x, double y);
};


G_BEGIN_DECLS

G_END_DECLS

#endif /* __MOON_VISUAL_H__ */
