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

	int DumpHierarchy (UIElement *obj);

	enum UIElementFlags {
		IS_LOADED = 1
	};
	
	int flags;

	// The computed bounding box
	double x1, y1, x2, y2;

	// Absolute affine transform, precomputed with all of its data
	cairo_matrix_t absolute_xform;

	virtual Surface *GetSurface ();

	//
	// UpdateTransform:
	//   Updates the absolute_xform for this item
	//
	virtual void UpdateTransform ();
	
	//
	// Render: 
	//   Renders the given @item on the @surface.  the area that is
	//   exposed is delimited by x, y, width, height
	//
	virtual void Render (cairo_t *cr, int x, int y, int width, int height);

	// a non virtual method for use when we want to wrap render
	// with debugging and/or timing info
	void DoRender (cairo_t *cr, int x, int y, int width, int height);

	//
	// GetSizeForBrush:
	//   Gets the size of the area to be painted by a Brush (needed for image/video scaling)
	virtual void GetSizeForBrush (cairo_t *cr, double *width, double *height);

	//
	// UpdateBounds:
	//   Updates the bounds of a item by requesting bounds update
	//   to all of its parents.
	//
	void UpdateBounds ();

	// 
	// GetBounds:
	//   Updates the bounding box for the given item, this uses the parent
	//   chain to compute the composite affine.
	//
	// Output:
	//   the item->x1,y1,x2,y2 values are updated.
	// 
	virtual void GetBounds ();

	//
	// GetTransformFor
	//   Obtains the affine transform for the given child, this is
	//   implemented by containers

	virtual void GetTransformFor (UIElement *item, cairo_matrix_t *result);

	//
	// Recomputes the bounding box, requests redraws, 
	// the parameter determines if we should also update the transformation
	//
	void FullInvalidate (bool render_xform);
	
	//
	// Invalidates the items bounding rectangle on its surface
	//
	void Invalidate ();

	//
	// GetTransformOrigin:
	//   Returns the transformation origin based on  of the item and the
	//   xform_origin
	virtual Point GetTransformOrigin () {
		return Point (0, 0);
	}

	//
	// InsideObject:
	//   Returns whether the position x, y is inside the object
	//
	virtual bool InsideObject (Surface *s, double x, double y);
	
	//
	// HandleMotion:
	//   handles an mouse motion event, and dispatches it to anyone that
	//   might want it.   Returns true if the event was within this UIElement
	//   boundaries.
	//
	virtual bool HandleMotion (Surface *s, int state, double x, double y);

	//
	// HandleButton:
	//   handles the button press or button release events and dispatches
	//   it to all the objects that might be interested in it (nested
	//   objects).
	//
	//   Returns true if the button click was handled. 
	//
	virtual bool HandleButton (Surface *s, callback_mouse_event cb, int state, double x, double y);
	
	//
	// Enter:
	//   Invoked when the mouse first enters this given object
	//
	virtual void Enter (Surface *s, int state, double x, double y);
	
	//
	// Leave:
	//   Invoke when the mouse leaves this given object
	//
	virtual void Leave (Surface *s);

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


Surface *uielement_get_surface          (UIElement *item);
void     uielement_invalidate           (UIElement *item);
void     uielement_update_bounds        (UIElement *item);
void     uielement_set_transform        (UIElement *item, double *transform);
void     uielement_set_transform_origin (UIElement *item, Point p);

void     uielement_set_render_transform (UIElement *item, Transform *transform);
void     uielement_get_render_affine    (UIElement *item, cairo_matrix_t *result);

double	 uielement_get_opacity     (UIElement *item);
void	 uielement_set_opacity     (UIElement *item, double opacity);

Brush   *uielement_get_opacity_mask (UIElement *item);

void     uielement_transform_point (UIElement *item, double *x, double *y);
	

void     uielement_init ();
G_END_DECLS

#endif /* __MOON_UIELEMENT_H__ */
