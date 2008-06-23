/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * panel.h
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_PANEL_H__
#define __MOON_PANEL_H__

#include <glib.h>
#include "frameworkelement.h"

/* @ContentProperty="Children" */
class Panel : public FrameworkElement {
 private:
	//
	// Contains the last element where the mouse entered
	//
	UIElement *mouse_over;

	void ChildAdded (Visual *child);
	void ChildRemoved (Visual *child);

	bool emitting_loaded;

 protected:
	virtual ~Panel ();
	bool UseBackToFront ();

 public:
	static DependencyProperty *BackgroundProperty;
	static DependencyProperty *ChildrenProperty;
	
	Rect bounds_with_children;
	
	Panel ();
	virtual Type::Kind GetObjectType () { return Type::PANEL; }
	
	virtual void SetSurface (Surface *s);

	virtual void ComputeBounds ();
	virtual void Render (cairo_t *cr, Region *region);
	virtual void RenderChildren (cairo_t *cr, Region *region);

	virtual void FrontToBack (Region *surface_region, List *render_list);
	virtual void PostRender (cairo_t *cr, Region *region, bool front_to_back);

	bool CheckOver (cairo_t *cr, UIElement *item, double x, double y);

	virtual UIElement *FindMouseOver (cairo_t *cr, double x, double y);

	virtual void HitTest (cairo_t *cr, double x, double y, List *uielement_list);

	virtual bool InsideObject (cairo_t *cr, double x, double y);

	virtual Rect GetSubtreeBounds () { return bounds_with_children; }

	virtual void ShiftPosition (Point p);

	virtual void UpdateTotalRenderVisibility ();
	virtual void UpdateTotalHitTestVisibility ();

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
	virtual void OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, PropertyChangedEventArgs *element_args);

	virtual void CacheInvalidateHint ();

	virtual void OnLoaded ();
	
	void AddChild (UIElement *item);
	
	//
	// Property Accessors
	//
	void SetBackground (Brush *background);
	Brush *GetBackground ();
	
	void SetChildren (VisualCollection *children);
	VisualCollection *GetChildren ();
};

G_BEGIN_DECLS

Panel *panel_new (void);

void panel_set_background (Panel *panel, Brush *background);
Brush *panel_get_background (Panel *panel);

void panel_set_children (Panel *panel, VisualCollection *children);
VisualCollection *panel_get_children (Panel *panel);

void panel_child_add (Panel *panel, UIElement *element);

void panel_init (void);

G_END_DECLS

#endif /* __MOON_PANEL_H__ */
