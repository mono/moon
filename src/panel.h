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
/* @Namespace=System.Windows.Controls */
class Panel : public FrameworkElement {
 private:
	//
	// Contains the last element where the mouse entered
	//
	UIElement *mouse_over;

	void ChildAdded (UIElement *child);
	void ChildRemoved (UIElement *child);

	bool emitting_loaded;

 protected:
	virtual ~Panel ();
	bool UseBackToFront ();

 public:
 	/* @PropertyType=Brush */
	static DependencyProperty *BackgroundProperty;
 	/* @PropertyType=UIElementCollection */
	static DependencyProperty *ChildrenProperty;
	
	Rect bounds_with_children;
	
 	/* @GenerateCBinding */
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

	virtual void HitTest (cairo_t *cr, Point p, List *uielement_list);
	virtual void HitTest (cairo_t *cr, Rect r, List *uielement_list);

	virtual bool InsideObject (cairo_t *cr, double x, double y);

	virtual Rect GetSubtreeBounds () { return bounds_with_children; }

	virtual void ShiftPosition (Point p);

	virtual void UpdateTotalRenderVisibility ();
	virtual void UpdateTotalHitTestVisibility ();

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
	
	virtual void OnCollectionClear (Collection *col);
	virtual void OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args);
	virtual void OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args);
	
	virtual void CacheInvalidateHint ();

	virtual void OnLoaded ();
	
	void AddChild (UIElement *item);
	
	//
	// Property Accessors
	//
	void SetBackground (Brush *background);
	Brush *GetBackground ();
	
	void SetChildren (UIElementCollection *children);
	UIElementCollection *GetChildren ();
};

G_BEGIN_DECLS

void panel_set_background (Panel *panel, Brush *background);
Brush *panel_get_background (Panel *panel);

void panel_set_children (Panel *panel, UIElementCollection *children);
UIElementCollection *panel_get_children (Panel *panel);

void panel_child_add (Panel *panel, UIElement *element);

G_END_DECLS

#endif /* __MOON_PANEL_H__ */
