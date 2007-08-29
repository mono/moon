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
	virtual void Render (cairo_t *cr, int x, int y, int width, int height);

	bool CheckOver (cairo_t *cr, UIElement *item, double x, double y);

	virtual UIElement* FindMouseOver (cairo_t *cr, double x, double y);

	virtual bool InsideObject (cairo_t *cr, double x, double y);

	virtual void HandleMotion (cairo_t *cr, int state, double x, double y, MouseCursor *cursor);
	virtual void HandleButtonPress (cairo_t *cr, int state, double x, double y);
	virtual void HandleButtonRelease (cairo_t *cr, int state, double x, double y);
	virtual void Enter (cairo_t *cr, int state, double x, double y);
	virtual void Leave ();

	static DependencyProperty* ChildrenProperty;
	static DependencyProperty* BackgroundProperty;

	virtual void OnPropertyChanged (DependencyProperty *prop);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyProperty *subprop);
	virtual void OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, DependencyProperty *prop);

	virtual void OnLoaded ();
};

G_BEGIN_DECLS

void  panel_child_add      (Panel *panel, UIElement *item);
Panel *panel_new (void);
Brush *panel_get_background (Panel *panel);

void panel_init (void);

G_END_DECLS

#endif /* __MOON_PANEL_H__ */
