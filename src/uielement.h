/*
 * uielement.h
 *
 * Author:
 *   Miguel de Icaza (miguel@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_UIELEMENT_H__
#define __MOON_UIELEMENT_H__

#include "visual.h"
#include "point.h"

class Surface;

typedef void (*callback_mouse_event)    (UIElement *target, int state, double x, double y);
typedef void (*callback_plain_event)    (UIElement *target);
typedef bool (*callback_keyboard_event) (UIElement *target, int state, int platformcode, int key);

class UIElement : public Visual {
	Brush *opacityMask;
 public:
	UIElement ();
	~UIElement ();
	virtual Type::Kind GetObjectType () { return Type::UIELEMENT; };

	UIElement *parent;

	int dump_hierarchy (UIElement *obj);

	enum UIElementFlags {
		IS_LOADED = 1
	};
	
	int flags;

	// The computed bounding box
	double x1, y1, x2, y2;

	// Absolute affine transform, precomputed with all of its data
	cairo_matrix_t absolute_xform;

	//
	// update_xform:
	//   Updates the absolute_xform for this item
	//
	virtual void update_xform ();
	
	//
	// render: 
	//   Renders the given @item on the @surface.  the area that is
	//   exposed is delimited by x, y, width, height
	//
	virtual void render (cairo_t *cr, int x, int y, int width, int height);

	// a non virtual method for use when we want to wrap render
	// with debugging and/or timing info
	void dorender (cairo_t *cr, int x, int y, int width, int height);

	//
	// get_size_for_brush:
	//   Gets the size of the area to be painted by a Brush (needed for image/video scaling)
	virtual void get_size_for_brush (cairo_t *cr, double *width, double *height);

	//
	// updatebounds:
	//   Updates the bounds of a item by requesting bounds update
	//   to all of its parents.
	//
	void updatebounds ();

	// 
	// getbounds:
	//   Updates the bounding box for the given item, this uses the parent
	//   chain to compute the composite affine.
	//
	// Output:
	//   the item->x1,y1,x2,y2 values are updated.
	// 
	virtual void getbounds ();

	//
	// get_xform_for
	//   Obtains the affine transform for the given child, this is
	//   implemented by containers

	virtual void get_xform_for (UIElement *item, cairo_matrix_t *result);

	//
	// Recomputes the bounding box, requests redraws, 
	// the parameter determines if we should also update the transformation
	//
	void FullInvalidate (bool render_xform);
	
	//
	// gencenter:
	//   Returns the transformation origin based on  of the item and the
	//   xform_origin
	virtual Point getxformorigin () {
		return Point (0, 0);
	}

	//
	// inside_object:
	//   Returns whether the position x, y is inside the object
	//
	virtual bool inside_object (Surface *s, double x, double y);
	
	//
	// handle_motion:
	//   handles an mouse motion event, and dispatches it to anyone that
	//   might want it.   Returns true if the event was within this UIElement
	//   boundaries.
	//
	virtual bool handle_motion (Surface *s, int state, double x, double y);

	//
	// handle_button:
	//   handles the button press or button release events and dispatches
	//   it to all the objects that might be interested in it (nested
	//   objects).
	//
	//   Returns true if the button click was handled. 
	//
	virtual bool handle_button (Surface *s, callback_mouse_event cb, int state, double x, double y);
	
	//
	// enter:
	//   Invoked when the mouse first enters this given object
	//
	virtual void enter (Surface *s, int state, double x, double y);
	
	//
	// leave:
	//   Invoke when the mouse leaves this given object
	//
	virtual void leave (Surface *s);

	//
	// GetTotalOpacity
	//   Get the cumulative opacity of this element, including all it's parents
	double GetTotalOpacity ();
	
	virtual void OnLoaded ();

	virtual void OnPropertyChanged (DependencyProperty *prop);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyProperty *subprop);

	Point GetRenderTransformOrigin () {
		Value *vu = GetValue (UIElement::RenderTransformOriginProperty);
		if (vu)
			return *vu->AsPoint ();
		return Point (0, 0);
	}

	static DependencyProperty* RenderTransformProperty;
	static DependencyProperty* OpacityProperty;
	static DependencyProperty* ClipProperty;
	static DependencyProperty* OpacityMaskProperty;
	static DependencyProperty* RenderTransformOriginProperty;
	static DependencyProperty* CursorProperty;
	static DependencyProperty* IsHitTestVisibleProperty;
	static DependencyProperty* VisibilityProperty;
	static DependencyProperty* ResourcesProperty;
	static DependencyProperty* TriggersProperty;
	static DependencyProperty* ZIndexProperty;

};

G_BEGIN_DECLS


Surface *item_get_surface          (UIElement *item);
void     item_invalidate           (UIElement *item);
void     item_update_bounds        (UIElement *item);
void     item_set_transform        (UIElement *item, double *transform);
void     item_set_transform_origin (UIElement *item, Point p);

void     item_set_render_transform (UIElement *item, Transform *transform);
void     item_get_render_affine    (UIElement *item, cairo_matrix_t *result);

double	 uielement_get_opacity     (UIElement *item);
void	 uielement_set_opacity     (UIElement *item, double opacity);

Brush   *uielement_get_opacity_mask (UIElement *item);

void     uielement_transform_point (UIElement *item, double *x, double *y);
	

void     uielement_init ();
G_END_DECLS

#endif /* __MOON_UIELEMENT_H__ */
