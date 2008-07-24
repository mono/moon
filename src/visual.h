/*
 * visual.h
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_VISUAL_H__
#define __MOON_VISUAL_H__

#include <glib.h>

#include "dependencyobject.h"
#include "collection.h"

class TimeManager;

class Visual : public DependencyObject {
	UIElement *visual_parent;
	
 protected:
	virtual ~Visual () {}

 public:
	Visual () : visual_parent (NULL) {};
	virtual Type::Kind GetObjectType () { return Type::VISUAL; };	

	//
	// InsideObject:
	//   Returns whether the position x, y is inside the object
	//
	virtual bool InsideObject (cairo_t *cr, double x, double y);
	
	virtual TimeManager *GetTimeManager ();
	
	void SetVisualParent (UIElement *visual_parent) { this->visual_parent = visual_parent; }
	UIElement *GetVisualParent () { return visual_parent; }
};

class VisualCollection : public DependencyObjectCollection {
 protected:
	virtual void AddedToCollection (Value *value);
	virtual void RemovedFromCollection (Value *value);
	
	virtual ~VisualCollection ();
	
 public:
	GPtrArray *z_sorted;
	
	VisualCollection ();
	
	virtual Type::Kind GetObjectType () { return Type::VISUAL_COLLECTION; }
	virtual Type::Kind GetElementType () { return Type::VISUAL; }
	
	virtual bool Insert (int index, Value value);
	virtual void Clear ();
	
	void ResortByZIndex ();
};


G_BEGIN_DECLS

void visual_set_surface (Visual* visual, Surface* surface);

VisualCollection *visual_collection_new (void);

G_END_DECLS

#endif /* __MOON_VISUAL_H__ */
