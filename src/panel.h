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

#include "frameworkelement.h"

/* @ContentProperty="Children" */
class Panel : public FrameworkElement {
	Brush *background;

	//
	// Contains the last element where the mouse entered
	//
	UIElement *mouse_over;

 public:
	Panel ();
	virtual ~Panel ();
	virtual Type::Kind GetObjectType () { return Type::PANEL; }

	VisualCollection *GetChildren ();
	void SetChildren (VisualCollection *col);

	virtual void ComputeBounds ();
	virtual void Render (cairo_t *cr, Region *region);
	virtual void RenderChildren (cairo_t *cr, Region *region);

	virtual void FrontToBack (Region *surface_region, List *render_list);
	virtual void PostRender (cairo_t *cr, Region *region, bool front_to_back);

	bool CheckOver (cairo_t *cr, UIElement *item, double x, double y);

	virtual UIElement* FindMouseOver (cairo_t *cr, double x, double y);

	virtual void HitTest (cairo_t *cr, double x, double y, List *uielement_list);

	virtual bool InsideObject (cairo_t *cr, double x, double y);

	virtual Rect GetSubtreeBounds () { return bounds_with_children; }

	static DependencyProperty* ChildrenProperty;
	static DependencyProperty* BackgroundProperty;

	virtual void UpdateTotalRenderVisibility ();
	virtual void UpdateTotalHitTestVisibility ();

	virtual void OnPropertyChanged (DependencyProperty *prop);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, DependencyProperty *subprop);
	virtual void OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, DependencyProperty *prop);

	virtual void OnLoaded ();

	Rect bounds_with_children;
};

G_BEGIN_DECLS

void  panel_child_add      (Panel *panel, UIElement *item);
Panel *panel_new (void);
Brush *panel_get_background (Panel *panel);

void panel_init (void);

G_END_DECLS

#endif /* __MOON_PANEL_H__ */
