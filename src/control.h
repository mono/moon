/*
 * control.h:
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_CONTROL_H__
#define __MOON_CONTROL_H__

#include "frameworkelement.h"
#include "xaml.h"

//
// Control Class
//
class Control : public FrameworkElement {
 public:
	FrameworkElement *real_object;

	Control () : real_object (NULL) { };
	virtual ~Control ();
	
	virtual Type::Kind GetObjectType () { return Type::CONTROL; }

	virtual void Render (cairo_t *cr, Region *region);
	virtual void FrontToBack (Region *surface_region, List *render_list);
	virtual void ComputeBounds ();
	virtual Rect GetSubtreeBounds () { return bounds_with_children; }
	virtual void GetTransformFor (UIElement *item, cairo_matrix_t *result);

	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, DependencyProperty *subprop);

	virtual bool InsideObject (cairo_t *cr, double x, double y);

	virtual void HitTest (cairo_t *cr, double x, double y, List *uielement_list);

	virtual bool GetRenderVisible () { return real_object && real_object->GetRenderVisible(); }
	virtual bool GetHitTestVisible () { return real_object && real_object->GetHitTestVisible(); }

	virtual void OnLoaded ();

	UIElement* InitializeFromXaml (const char *xaml,
				       Type::Kind *element_type, XamlLoader* loader);

	Rect bounds_with_children;
};

G_BEGIN_DECLS

Control *control_new (void);
UIElement* control_initialize_from_xaml (Control *control, const char *xaml,
					 Type::Kind *element_type);
UIElement* control_initialize_from_xaml_callbacks (Control *control, const char *xaml, 
					Type::Kind *element_type, XamlLoader* loader);
G_END_DECLS

#endif /* __MOON_CONTROL_H__ */
