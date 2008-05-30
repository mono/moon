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
#include "rect.h"
#include "region.h"
#include "list.h"
#include "size.h"

#define QUANTUM_ALPHA 1

#if QUANTUM_ALPHA
#define IS_TRANSLUCENT(x) (x * 255 < 254.5)
#define IS_INVISIBLE(x) (x * 255 < .5)
#else
#define IS_TRANSLUCENT(x) (x < 1.0)
#define IS_INVISIBLE(x) (x <= 0.0)
#endif

class Surface;

class UIElement : public Visual {
	Brush *opacityMask;

	double total_opacity;

 protected:
	virtual ~UIElement ();
	Rect IntersectBoundsWithClipPath (Rect bounds, bool transform);
	void RenderClipPath (cairo_t *cr);

 public:
	UIElement ();
	virtual Type::Kind GetObjectType () { return Type::UIELEMENT; };

	int dirty_flags;
	List::Node *up_dirty_node;
	List::Node *down_dirty_node;

	bool force_invalidate_of_new_bounds;
	Region *dirty_region;

	int DumpHierarchy (UIElement *obj);

	enum UIElementFlags {
		IS_LOADED        = 0x01,

		// these two flags correspond to the 2 states of VisibilityProperty
		RENDER_VISIBLE   = 0x02,

		// the HitTestVisible property
		HIT_TEST_VISIBLE = 0x04,

		TOTAL_RENDER_VISIBLE   = 0x08,
		TOTAL_HIT_TEST_VISIBLE = 0x10,

		SHAPE_EMPTY      = 0x20,	// there's is nothing to draw, the cached path may be NULL
		SHAPE_NORMAL     = 0x40,	// normal drawing
		SHAPE_DEGENERATE = 0x80,	// degenerate drawing, use the Stroke brush for filling
		SHAPE_RADII      = 0x100,
		SHAPE_MASK       = (SHAPE_EMPTY | SHAPE_NORMAL | SHAPE_DEGENERATE | SHAPE_RADII)
	};
	
	int flags;

	// The computed bounding box
	Rect bounds;

	// Absolute affine transform, precomputed with all of its data
	cairo_matrix_t absolute_xform;

	// the transform to be multiplied by our parent transform to compute absolute_xform
	cairo_matrix_t local_transform;

	cairo_matrix_t parent_transform;

	Point transform_origin;

	virtual bool EnableAntiAlias() { return true; }

	// UpdateTotalRenderVisibility:
	//   Updates the opacity and render visibility on this item based on 
	//   its parent's opacity and visibility as well as the value of its 
	//   OpacityProperty and RenderVisibilityProperty.
	//
	virtual void UpdateTotalRenderVisibility ();
	void ComputeTotalRenderVisibility ();
	bool GetActualTotalRenderVisibility ();

	virtual void UpdateTotalHitTestVisibility ();
	void ComputeTotalHitTestVisibility ();
	bool GetActualTotalHitTestVisibility ();

	virtual void SetSurface (Surface *s);

	//
	// UpdateTransform:
	//   Updates the absolute_xform for this item
	//
	virtual void UpdateTransform ();

	void ComputeLocalTransform ();
	virtual void ComputeTransform ();

	//
	// GetRenderVisible:
	//   Returns true if the Visibility property of this item is "Visible", and false otherwise
	//
	bool GetRenderVisible () { return (flags & UIElement::TOTAL_RENDER_VISIBLE) != 0; }

	//
	// GetHitTestVisible:
	//   Returns true if the IsHitTestVisible property of this item true, and false otherwise
	//
	bool GetHitTestVisible () { return (flags & UIElement::TOTAL_HIT_TEST_VISIBLE) != 0; }

	//
	// Render: 
	//   Renders the given @item on the @surface.  the area that is
	//   exposed is delimited by x, y, width, height
	//
	virtual void Render (cairo_t *cr, Region *region);
	virtual void Render (cairo_t *cr, int x, int y, int width, int height);

	// a non virtual method for use when we want to wrap render
	// with debugging and/or timing info
	void DoRender (cairo_t *cr, Region *region);

	virtual void FrontToBack (Region *surface_region, List *render_list);
	virtual void PreRender (cairo_t *cr, Region *region, bool front_to_back);
	virtual void PostRender (cairo_t *cr, Region *region, bool front_to_back);

	static void CallPreRender (cairo_t *cr, UIElement *element, Region *region, bool front_to_back);
	static void CallPostRender (cairo_t *cr, UIElement *element, Region *region, bool front_to_back);

	//
	// GetSizeForBrush:
	//   Gets the size of the area to be painted by a Brush (needed for image/video scaling)
	virtual void GetSizeForBrush (cairo_t *cr, double *width, double *height);

	//
	// GetOriginPoint:
	//   Gets real origin, required e.g. for lineargradientbrushes on Path
	virtual Point GetOriginPoint ()
	{
		return Point (0.0, 0.0);
	}

	//
	// UpdateBounds:
	//   Recomputes the bounds of this element, and if they're
	//   different chains up to its parent telling it to update
	//   its bounds.
	//
	void UpdateBounds (bool force_redraw_of_new_bounds = false);

	// 
	// ComputeBounds:
	//   Updates the bounding box for the given item, this uses the parent
	//   chain to compute the composite affine.
	//
	// Output:
	//   item->bounds is updated
	// 
	virtual void ComputeBounds ();

	// 
	// GetBounds:
	//   returns the current bounding box for the given item.
	// 
	virtual Rect GetBounds () { return bounds; }

	// 
	// GetSubtreeBounds:
	//   returns the bounding box including all sub-uielements.
	//   implemented by containers.
	// 
	virtual Rect GetSubtreeBounds () { return bounds; }

	//
	// GetRenderBounds:
	// returns the bounding box to be rendered, which
	// unfortunately isn't necessarily the same as either our
	// bounds or subtree bounds (in the case of inkpresenter)
	virtual Rect GetRenderBounds () { return bounds; }

	//
	// GetTransformFor
	//   Obtains the affine transform for the given child, this is
	//   implemented by containers

	virtual void GetTransformFor (UIElement *item, cairo_matrix_t *result);


	// HitTest
	//   Accumulate a list of all elements that will generate an
	//   event from this x,y. The first node in the list is the most
	//   deeply nested node, the last node is the root.
	virtual void HitTest (cairo_t *cr, double x, double y, List *uielement_list);

	//
	// Recomputes the bounding box, requests redraws, 
	// the parameter determines if we should also update the transformation
	//
	void FullInvalidate (bool render_xform);

	virtual bool InsideObject (cairo_t *cr, double x, double y);

	//
	// Checks if the point is inside the Clip region.
	// Returns true if no Clip region is defined
	// (which is actually an infinitely big Clip region).
	// 
	bool InsideClip (cairo_t *cr, double x, double y);

	//
	// Invalidates a subrectangle of this element
	//
	void Invalidate (Rect r);

	//
	// Invalidates a subregion of this element
	void Invalidate (Region *region);
	
	//
	// Invalidates the entire bounding rectangle of this element
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
	// EmitMouseMove:
	//   handles an mouse motion event, and dispatches it to anyone that
	//   might want it.
	//
	bool EmitMouseMove (GdkEvent *event);

	//
	// EmitMouseLeftButtonDown:
	//   handles the button press event and dispatches it to all
	//   the objects that might be interested in it.
	//
	bool EmitMouseLeftButtonDown (GdkEvent *event);

	//
	// EmitMouseLeftButtonUp:
	//   handles the button release event and dispatches it to all
	//   the objects that might be interested in it.
	//
	bool EmitMouseLeftButtonUp (GdkEvent *event);

	//
	// EmitKeyDown:
	//
	bool EmitKeyDown (int state, Key key, int platform_key_code);

	//
	// EmitKeyUp:
	//
	bool EmitKeyUp (int state, Key key, int platform_key_code);

	//
	// EmitMouseEnter:
	//   Invoked when the mouse first enters this given object
	//
	bool EmitMouseEnter (GdkEvent *event);
	
	//
	// EmitMouseLeave:
	//   Invoked when the mouse leaves this given object
	//
	bool EmitMouseLeave ();
	
	//
	// EmitGotFocus:
	//   Invoked when the mouse focuses the given object
	//
	bool EmitGotFocus ();

	//
	// EmitLostFocus:
	//   Invoked when the given object loses mouse focus
	//
	bool EmitLostFocus ();
	
	//
	// CaptureMouse:
	//
	//    Attempts to capture the mouse.  If successful, all mouse
	//    events will be transmitted directly to this element.
	//    Leave/Enter events will no longer be sent.
	//
	bool CaptureMouse ();

	//
	// ReleaseMouseCapture:
	//
	//    Attempts to release the mouse.  If successful, any
	//    applicable Leave/Enter events for the current mouse
	//    position will be sent.
	//
	void ReleaseMouseCapture ();

	virtual void OnLoaded ();

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);

	//
	// CacheInvalidateHint:
	//   Give a hint to this UIElement that it should free any possible
	//   cached (mem-intensive) data it has. This ie. can happen when the
	//   element is removed from a collection, becomes invisible, etc.
	//
	virtual void CacheInvalidateHint () {}

	Point GetRenderTransformOrigin () {
		Value *vu = GetValue (UIElement::RenderTransformOriginProperty);
		return *vu->AsPoint ();
	}

	//
	// 2.0 methods
	//
	Size desired_size;
	
	void Measure (Size availableSize)
	{
		desired_size = MeasureCore (availableSize);
	}

	virtual Size MeasureCore (Size availableSize)
	{
		return Size (0, 0);
	}
	
	static DependencyProperty *ClipProperty;
	static DependencyProperty *CursorProperty;
	static DependencyProperty *IsHitTestVisibleProperty;
	static DependencyProperty *OpacityMaskProperty;
	static DependencyProperty *OpacityProperty;
	static DependencyProperty *RenderTransformOriginProperty;
	static DependencyProperty *RenderTransformProperty;
	static DependencyProperty *ResourcesProperty;
	static DependencyProperty *TagProperty;
	static DependencyProperty *TriggersProperty;
	static DependencyProperty *VisibilityProperty;
	static DependencyProperty *ZIndexProperty;
	
	// property accessors
	void SetOpacityMask (Brush *mask);
	Brush *GetOpacityMask ();
	
	void SetOpacity (double opacity);
	double GetOpacity ();
	
	// Events you can AddHandler to
	const static int LoadedEvent;
	const static int MouseMoveEvent;
	const static int MouseLeftButtonDownEvent;
	const static int MouseLeftButtonUpEvent;
	const static int KeyDownEvent;
	const static int KeyUpEvent;
	const static int MouseEnterEvent;
	const static int MouseLeaveEvent;
	const static int InvalidatedEvent;
	const static int GotFocusEvent;
	const static int LostFocusEvent;
};

G_BEGIN_DECLS

UIElement *uielement_new                  (void);

Surface   *uielement_get_surface          (UIElement *item);
void       uielement_invalidate           (UIElement *item);
void       uielement_update_bounds        (UIElement *item);
void       uielement_set_transform        (UIElement *item, double *transform);
void       uielement_set_transform_origin (UIElement *item, Point p);
         
void       uielement_set_render_transform (UIElement *item, Transform *transform);
void       uielement_get_render_affine    (UIElement *item, cairo_matrix_t *result);
         
double     uielement_get_opacity          (UIElement *item);
void       uielement_set_opacity          (UIElement *item, double opacity);
         
Brush     *uielement_get_opacity_mask     (UIElement *item);
void       uielement_transform_point      (UIElement *item, double *x, double *y);
UIElement *uielement_get_parent           (UIElement *item);

Size       uielement_get_desired_size     (UIElement *item);

bool       uielement_capture_mouse         (UIElement *item);
void       uielement_release_mouse_capture (UIElement *item);

void       uielement_init (void);
G_END_DECLS

#endif /* __MOON_UIELEMENT_H__ */
