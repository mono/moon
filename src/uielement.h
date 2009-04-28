/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * uielement.h
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_UIELEMENT_H__
#define __MOON_UIELEMENT_H__

#include <glib.h>

#include "dependencyobject.h"
#include "resources.h"
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

/* @Namespace=System.Windows */
class UIElement : public DependencyObject {
public:
	UIElement ();
	virtual void Dispose ();
	
	int dirty_flags;
	List::Node *up_dirty_node;
	List::Node *down_dirty_node;

	bool force_invalidate_of_new_bounds;
	bool emitting_loaded;

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
		SHAPE_MASK       = (SHAPE_EMPTY | SHAPE_NORMAL | SHAPE_DEGENERATE | SHAPE_RADII),

		// this means the element will be emitting OnLoaded
		// shortly, and any child added to the element while
		// it is in this state should post Loaded as well.
		PENDING_LOADED   = 0x200
	};
	
	virtual TimeManager *GetTimeManager ();

	virtual bool PermitsMultipleParents () { return false; }

	void SetVisualParent (UIElement *visual_parent);
	/* @GenerateCBinding,GeneratePInvoke */
	UIElement *GetVisualParent () { return visual_parent; }

	int GetVisualLevel () { return visual_level; }
	void SetVisualLevel (int level) { visual_level = level; }

	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
	virtual void SetSubtreeObject (DependencyObject *value);

	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
	virtual DependencyObject *GetSubtreeObject () { return subtree_object; }

	/* @GenerateCBinding,GeneratePInvoke */
	virtual void ElementAdded (UIElement *obj);
	/* @GenerateCBinding,GeneratePInvoke */
	virtual void ElementRemoved (UIElement *obj);
	
	virtual bool EnableAntiAlias() { return true; }

	virtual void SetSurface (Surface *s);

	// UpdateTotalRenderVisibility:
	//   Updates the opacity and render visibility on this item based on 
	//   its parent's opacity and visibility as well as the value of its 
	//   OpacityProperty and RenderVisibilityProperty.
	//
	void UpdateTotalRenderVisibility ();
	void ComputeTotalRenderVisibility ();
	bool GetActualTotalRenderVisibility ();

	//
	// GetRenderVisible:
	//   Returns true if the Visibility property of this item is "Visible", and false otherwise
	//
	bool GetRenderVisible () { return (flags & UIElement::TOTAL_RENDER_VISIBLE) != 0; }


	// UpdateTotalHitTestVisibility:
	//   Updates the hit testability of the item based on the you know..
	//   The hit test flag.
	void UpdateTotalHitTestVisibility ();
	void ComputeTotalHitTestVisibility ();
	bool GetActualTotalHitTestVisibility ();

	//
	// GetHitTestVisible:
	//   Returns true if the IsHitTestVisible property of this item true, and false otherwise
	//
	bool GetHitTestVisible () { return (flags & UIElement::TOTAL_HIT_TEST_VISIBLE) != 0; }
	
	//
	// UpdateTransform:
	//   Updates the absolute_xform for this item
	//
	void UpdateTransform ();
	void ComputeLocalTransform ();
	void ComputeTransform ();
	virtual void TransformBounds (cairo_matrix_t *old, cairo_matrix_t *current);

	//
	// IsLoaded:
	//   Returns true if the element has been attached to a
	//   surface and is part of the visual hierarchy.
	//
	bool IsLoaded () { return (flags & UIElement::IS_LOADED) != 0; }
	void ClearLoaded ();

	//
	// Render: 
	//   Renders the given @item on the @surface.  the area that is
	//   exposed is delimited by x, y, width, height
	//
	virtual void Render (cairo_t *cr, Region *region, bool path_only = false);

	//
	// Paint:
	//   Do an optimized render pass on the this element and it's
	//   subtree.
	//
	void Paint (cairo_t *cr, Region *region, cairo_matrix_t *matrix);

	// a non virtual method for use when we want to wrap render
	// with debugging and/or timing info
	void DoRender (cairo_t *cr, Region *region);
	bool UseBackToFront ();

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
	// ShiftPosition:
	//
	//   This method should actually set the x/y of the bounds
	//   rectangle to the new position.  It is virtual to allow
	//   subclasses with specialized bounds (Panel, InkPresenter)
	//   the opportunity to set those bounds' positions as well.
	//
	virtual void ShiftPosition (Point p);

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
	//   returns the current bounding box for the given item in surface 
	//   coordinates.
	// 
	Rect GetBounds () { return bounds; }

	// 
	// GetSubtreeBounds:
	//   returns the bounding box including all sub-uielements.
	//   implemented by containers in surface coordinates.
	// 
	virtual Rect GetSubtreeBounds () { return bounds; }

	//
	// GetRenderBounds:
	// returns the bounding box to be rendered, which
	// unfortunately isn't necessarily the same as either our
	// bounds or subtree bounds (in the case of inkpresenter)
	virtual Rect GetRenderBounds () { return bounds; }

	//
	// GetCoverageBounds:
	// returns the bounding box in global coordinates that opaquely covered by this object
	//
	virtual Rect GetCoverageBounds () { return Rect (); }


	//
	// IsLayoutContainer:
	//   returns true if the container has children that require a measure
	//   pass.
	virtual bool IsLayoutContainer () { return GetSubtreeObject () != NULL; }


	// HitTest

	//   Accumulate a list of all elements that will contain the
	//   point (or intersect the rectangle). The first node in the
	//   list is the most deeply nested node, the last node is the
	//   root.
	virtual void HitTest (cairo_t *cr, Point p, List *uielement_list);
	virtual void HitTest (cairo_t *cr, Rect r, List *uielement_list);
	
	/* @GenerateCBinding,GeneratePInvoke */
	void FindElementsInHostCoordinates_p (Point p, HitTestCollection *uielement_list);
	/* @GenerateCBinding,GeneratePInvoke */
	void FindElementsInHostCoordinates_r (Rect p, HitTestCollection *uielement_list);
	
	virtual bool CanFindElement () { return false; }
	virtual void FindElementsInHostCoordinates (cairo_t *cr, Point P, List *uielement_list);
	virtual void FindElementsInHostCoordinates (cairo_t *cr, Rect r, List *uielement_list);
	
	//
	// Recomputes the bounding box, requests redraws, 
	// the parameter determines if we should also update the transformation
	//
	void FullInvalidate (bool render_xform);

	//
	// InsideObject:
	//   Returns whether the position x, y is inside the object
	//
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
	// Invalidates the paint region of the element and its subtree
	//
	void InvalidateSubtreePaint ();
	void InvalidateMask ();
	void InvalidateClip ();
	void InvalidateVisibility ();

	//
	// GetTransformOrigin:
	//   Returns the transformation origin based on  of the item and the
	//   xform_origin
	virtual Point GetTransformOrigin () {
		return Point (0, 0);
	}

	//
	// EmitKeyDown:
	//
	bool EmitKeyDown (GdkEventKey *key);

	//
	// EmitKeyUp:
	//
	bool EmitKeyUp (GdkEventKey *key);

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
	/* @GenerateCBinding,GeneratePInvoke */
	bool CaptureMouse ();

	//
	// ReleaseMouseCapture:
	//
	//    Attempts to release the mouse.  If successful, any
	//    applicable Leave/Enter events for the current mouse
	//    position will be sent.
	//
	/* @GenerateCBinding,GeneratePInvoke */
	void ReleaseMouseCapture ();

	List* WalkTreeForLoaded (bool *delay);

	void PostSubtreeLoad (List *load_list);
	static void EmitSubtreeLoad (List *load_list);
	static void emit_delayed_loaded (EventObject *data);

	virtual void OnLoaded ();
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
	virtual void OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args);

	//
	// CacheInvalidateHint:
	//   Give a hint to this UIElement that it should free any possible
	//   cached (mem-intensive) data it has. This ie. can happen when the
	//   element is removed from a collection, becomes invisible, etc.
	//
	virtual void CacheInvalidateHint ();
	
	//
	// 2.0 methods
	//

	// Layout foo

	void DoMeasure ();
	void DoArrange ();

	/* @GenerateCBinding,GeneratePInvoke */
	virtual void Measure (Size availableSize) = 0;
	/* @GenerateCBinding,GeneratePInvoke */
	virtual void Arrange (Rect finalRect) = 0;
	/* @GenerateCBinding,GeneratePInvoke */
	void InvalidateMeasure ();
	/* @GenerateCBinding,GeneratePInvoke */
	void InvalidateArrange ();
	/* @GenerateCBinding,GeneratePInvoke */
	virtual bool UpdateLayout ();

	/* @GenerateCBinding,GeneratePInvoke */
	Size GetDesiredSize () { return desired_size; }
	
	/* @GenerateCBinding,GeneratePInvoke */
	Size GetRenderSize () { return render_size; }

	/* @GenerateCBinding,GeneratePInvoke,Version=2.0 */
	GeneralTransform *GetTransformToUIElementWithError (UIElement *to_element, MoonError *error);

	//
	// TransformPoint:
	//
	// Maps the point to the coordinate space of this item
	//
	void TransformPoint (double *x, double *y);
	
 	/* @PropertyType=Geometry,GenerateAccessors */
	const static int ClipProperty;
 	/* @PropertyType=bool,DefaultValue=true,GenerateAccessors */
	const static int IsHitTestVisibleProperty;
 	/* @PropertyType=Brush,GenerateAccessors */
	const static int OpacityMaskProperty;
 	/* @PropertyType=double,DefaultValue=1.0,GenerateAccessors */
	const static int OpacityProperty;
 	/* @PropertyType=Point,DefaultValue=Point (0\,0),GenerateAccessors */
	const static int RenderTransformOriginProperty;
	/* @PropertyType=Transform,GenerateAccessors,GenerateManagedAccessors=false */
	const static int RenderTransformProperty;
 	/* @PropertyType=Visibility,DefaultValue=VisibilityVisible,GenerateAccessors */
	const static int VisibilityProperty;
	/* @PropertyType=bool,DefaultValue=true,GenerateAccessors */
	const static int UseLayoutRoundingProperty;

	// in 2.0 these properties are actually in FrameworkElement
 	/* @PropertyType=MouseCursor,DefaultValue=MouseCursorDefault,ManagedDeclaringType=FrameworkElement,ManagedPropertyType=Cursor,ManagedFieldAccess=Internal,GenerateAccessors,Validator=CursorValidator */
	const static int CursorProperty;
 	/* @PropertyType=ResourceDictionary,ManagedDeclaringType=FrameworkElement,AutoCreateValue,ManagedFieldAccess=Internal,ManagedSetterAccess=Internal,GenerateAccessors */
	const static int ResourcesProperty;
 	/* @PropertyType=object,ManagedDeclaringType=FrameworkElement,ManagedPropertyType=object */
	const static int TagProperty;
 	/* @PropertyType=TriggerCollection,ManagedDeclaringType=FrameworkElement,AutoCreateValue,ManagedFieldAccess=Internal,ManagedSetterAccess=Internal,GenerateAccessors */
	const static int TriggersProperty;
	
	//
	// Property Accessors

	//
	void SetClip (Geometry *clip);
	Geometry *GetClip ();

	MouseCursor GetCursor ();
	void SetCursor (MouseCursor value);

	void SetIsHitTestVisible (bool visible);
	bool GetIsHitTestVisible ();
	
	void SetOpacityMask (Brush *mask);
	Brush *GetOpacityMask ();
	
	void SetOpacity (double opacity);
	double GetOpacity ();
	
	void SetRenderTransform (Transform *transform);
	Transform *GetRenderTransform ();
	
	void SetRenderTransformOrigin (Point *origin);
	Point *GetRenderTransformOrigin ();

	ResourceDictionary* GetResources();
	void SetResources (ResourceDictionary *value);

	TriggerCollection *GetTriggers ();
	void SetTriggers (TriggerCollection* value);
	
	Visibility GetVisibility ();
	void SetVisibility (Visibility value);

	bool GetUseLayoutRounding ();
	void SetUseLayoutRounding (bool value);

	// Events you can AddHandler to
	const static int LoadedEvent;
	const static int UnloadedEvent;
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
	const static int LostMouseCaptureEvent;

	const static int MouseRightButtonDownEvent;
	const static int MouseRightButtonUpEvent;
	const static int MouseWheelEvent;

protected:
	virtual ~UIElement ();
	Rect IntersectBoundsWithClipPath (Rect bounds, bool transform);
	void RenderClipPath (cairo_t *cr, bool path_only = false);

	void SetDesiredSize (Size s) { desired_size = s; }
	void SetRenderSize (Size s) { render_size = s; }

	// The computed bounding box
	Rect bounds;
	Rect extents;

	int flags;

	// Absolute affine transform, precomputed with all of its data
	cairo_matrix_t absolute_xform;
	cairo_matrix_t layout_xform;

	void FrontToBack (Region *surface_region, List *render_list);
	virtual void PreRender (cairo_t *cr, Region *region, bool front_to_back);
	virtual void PostRender (cairo_t *cr, Region *region, bool front_to_back);

	static void CallPreRender (cairo_t *cr, UIElement *element, Region *region, bool front_to_back);
	static void CallPostRender (cairo_t *cr, UIElement *element, Region *region, bool front_to_back);

private:
	int visual_level;
	UIElement *visual_parent;
	DependencyObject *subtree_object;
	double total_opacity;
	Brush *opacityMask;
	Size desired_size;
	Size render_size;

	// The local render transform including tranform origin
	cairo_matrix_t local_xform;
};

/* @IncludeInKinds,Namespace=System.Windows.Controls.Primitives,ManagedDependencyProperties=Manual */
class LayoutInformation {
public:
	/* @PropertyType=Geometry,Attached,GenerateAccessors */
	const static int LayoutClipProperty;
	/* @PropertyType=Rect,Attached,GenerateAccessors */
	const static int LayoutSlotProperty;
	/* @PropertyType=Size,Attached,GenerateAccessors */
	const static int LastMeasureProperty;
	/* @PropertyType=Size,Attached,GenerateAccessors */
	const static int LastArrangeProperty;
	/* @PropertyType=Size,Attached,GenerateAccessors */
	const static int LastRenderSizeProperty;

	static void SetLayoutClip (DependencyObject *item, Geometry *clip);
	static Geometry* GetLayoutClip (DependencyObject *item);

	static void SetLayoutSlot (DependencyObject *item, Rect *slot);
	static Rect *GetLayoutSlot (DependencyObject *item);

	static void SetLastMeasure (DependencyObject *item, Size *size);
	static Size *GetLastMeasure (DependencyObject *item);

	static void SetLastArrange (DependencyObject *item, Size *size);
	static Size *GetLastArrange (DependencyObject *item);

	static void SetLastRenderSize (DependencyObject *item, Size *size);
	static Size *GetLastRenderSize (DependencyObject *item);

	static void SetBounds (DependencyObject *item, Rect *bounds);
	static Rect *GetBounds (DependencyObject *item);
};
#endif /* __MOON_UIELEMENT_H__ */
