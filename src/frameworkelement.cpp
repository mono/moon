/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * frameworkelement.cpp: 
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>
#include <math.h>

#include "debug.h"
#include "geometry.h"
#include "application.h"
#include "deployment.h"
#include "runtime.h"
#include "namescope.h"
#include "frameworkelement.h"
#include "trigger.h"
#include "thickness.h"
#include "collection.h"
#include "style.h"
#include "validators.h"
#include "effect.h"
#include "projection.h"
#include "canvas.h"
#include "factory.h"

namespace Moonlight {

FrameworkElementProvider::FrameworkElementProvider (DependencyObject *obj, PropertyPrecedence precedence, int flags) : PropertyValueProvider (obj, precedence, flags)
{
	actual_height_value = NULL;
	actual_width_value = NULL;
	last = Size (-INFINITY, -INFINITY);
}
	
FrameworkElementProvider::~FrameworkElementProvider ()
{
	delete actual_height_value;
	delete actual_width_value;
}

Value *
FrameworkElementProvider::GetPropertyValue (DependencyProperty *property)
{
	if (property->GetId () != FrameworkElement::ActualHeightProperty && 
	    property->GetId () != FrameworkElement::ActualWidthProperty)
		return NULL;
	
	FrameworkElement *element = (FrameworkElement *) obj;

	Size actual = element->ComputeActualSize ();

	if (last != actual) {
		last = actual;

		if (actual_height_value)
			delete actual_height_value;

		if (actual_width_value)
			delete actual_width_value;

		actual_height_value = new Value (actual.height);
		actual_width_value = new Value (actual.width);
	}

 	if (property->GetId () == FrameworkElement::ActualHeightProperty) {
		return actual_height_value;
	} else {
		return actual_width_value;
	}
};

FrameworkElement::FrameworkElement ()
	: UIElement (Type::FRAMEWORKELEMENT), logical_parent (this, LogicalParentWeakRef), default_template (this, DefaultTemplateWeakRef)
{
	Init ();
}

FrameworkElement::FrameworkElement (Type::Kind object_type)
	: UIElement (object_type), logical_parent (this, LogicalParentWeakRef), default_template (this, DefaultTemplateWeakRef)
{
	Init ();
}


void
FrameworkElement::Init ()
{
	get_default_template_cb = NULL;
	measure_cb = NULL;
	arrange_cb = NULL;
	loaded_cb = NULL;
	style_resource_changed_cb = NULL;
	bounds_with_children = Rect ();
	global_bounds_with_children = Rect ();
	surface_bounds_with_children = Rect ();

	providers.localstyle = new StylePropertyValueProvider (this, PropertyPrecedence_LocalStyle, dispose_value);
	providers.implicitstyle = new ImplicitStylePropertyValueProvider (this, PropertyPrecedence_ImplicitStyle, dispose_value);
	providers.dynamicvalue = new FrameworkElementProvider (this, PropertyPrecedence_DynamicValue);
	providers.inheriteddatacontext = new InheritedDataContextValueProvider (this, PropertyPrecedence_InheritedDataContext);
}

FrameworkElement::~FrameworkElement ()
{
}

void
FrameworkElement::Dispose ()
{
	UIElement::Dispose ();
}

void
FrameworkElement::RenderLayoutClip (cairo_t *cr)
{
	FrameworkElement *element = this;
	cairo_matrix_t xform;

	/* store off the current transform since the following loop is about the blow it away */
	cairo_get_matrix (cr, &xform);

	while (element) {
		Geometry *geom = LayoutInformation::GetLayoutClip (element);

		if (geom) {
			geom->Draw (cr);
			cairo_clip (cr);
		}

		// translate by the negative visual offset of the
		// element to get the parent's coordinate space.
		Point *visual_offset = LayoutInformation::GetVisualOffset (element);
		if (element->Is (Type::CANVAS) || element->Is (Type::USERCONTROL))
			break;
		else if (visual_offset)
			cairo_translate (cr, -visual_offset->x, -visual_offset->y);

		element = (FrameworkElement*)element->GetVisualParent ();
	}

	/* restore the transform after we're done clipping */
	cairo_set_matrix (cr, &xform);
}

Point
FrameworkElement::GetTransformOrigin ()
{
	Point *user_xform_origin = GetRenderTransformOrigin ();
	
	double width = GetActualWidth ();
	double height = GetActualHeight ();

	return Point (width * user_xform_origin->x, 
		      height * user_xform_origin->y);
}

void
FrameworkElement::SetVisualParent (UIElement *visual_parent)
{
	UIElement::SetVisualParent (visual_parent);

	if (!logical_parent && (!visual_parent || visual_parent->Is (Type::FRAMEWORKELEMENT))) {
		providers.inheriteddatacontext->SetDataSource ((FrameworkElement *) visual_parent);
		if (IsLoaded ())
			providers.inheriteddatacontext->EmitChanged ();
	}
}

void
FrameworkElement::SetLogicalParent (DependencyObject *value, MoonError *error)
{
	if (logical_parent == value)
		return;

	if (GetDeployment ()->IsShuttingDown ()) {
		// for sanity's sake, we should verify that value is
		// NULL, but we don't care, we're going away anyway.
		logical_parent = NULL;
		return;
	}

	if (value && logical_parent && logical_parent != value) {
		MoonError::FillIn (error, MoonError::INVALID_OPERATION, "Element is a child of another element");
		return;
	}

	DependencyObject *old_parent = logical_parent;

	logical_parent = value;

	OnLogicalParentChanged (old_parent, value);
}

void
FrameworkElement::OnLogicalParentChanged (DependencyObject *old_logical_parent, DependencyObject *new_logical_parent)
{
	if (IsDisposing ()) {
		providers.inheriteddatacontext->SetDataSource (NULL);
	}
	else {
		if (new_logical_parent && new_logical_parent->Is (Type::FRAMEWORKELEMENT))
			providers.inheriteddatacontext->SetDataSource ((FrameworkElement *) new_logical_parent);
		else if (GetVisualParent () && GetVisualParent ()->Is(Type::FRAMEWORKELEMENT))
			providers.inheriteddatacontext->SetDataSource ((FrameworkElement *)GetVisualParent ());
		else
			providers.inheriteddatacontext->SetDataSource (NULL);
		if (IsLoaded ())
			providers.inheriteddatacontext->EmitChanged ();
	}
}

void
FrameworkElement::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType() != Type::FRAMEWORKELEMENT) {
		UIElement::OnPropertyChanged (args, error);
		return;
	}

	if (args->GetId () == FrameworkElement::WidthProperty ||
	    args->GetId () == FrameworkElement::MaxWidthProperty ||
	    args->GetId () == FrameworkElement::MinWidthProperty ||
	    args->GetId () == FrameworkElement::MaxHeightProperty ||
	    args->GetId () == FrameworkElement::MinHeightProperty ||
	    args->GetId () == FrameworkElement::HeightProperty ||
	    args->GetId () == FrameworkElement::MarginProperty ||
	    args->GetId () == FrameworkElement::FlowDirectionProperty) {

		Point *p = GetRenderTransformOrigin ();

		/* normally we'd only update the bounds of this
		   element on a width/height change, but if the render
		   transform is someplace other than (0,0), the
		   transform needs to be updated as well. */
		FullInvalidate (p->x != 0.0 || p->y != 0.0);
		
		FrameworkElement *visual_parent = (FrameworkElement *)GetVisualParent ();
		if (visual_parent) {
			visual_parent->InvalidateMeasure ();
		}

		InvalidateMeasure ();
		InvalidateArrange ();
		UpdateBounds ();
	}
	else if (args->GetId () == FrameworkElement::StyleProperty) {
		Style *new_style = args->GetNewValue () ? args->GetNewValue ()->AsStyle () : NULL;

		if (!error->number)
			providers.localstyle->UpdateStyle (new_style, error);

		if (error->number)
			return;
	}
	else if (args->GetId () == FrameworkElement::HorizontalAlignmentProperty ||
		 args->GetId () == FrameworkElement::VerticalAlignmentProperty) {
		InvalidateArrange ();
		FullInvalidate (true);
	}

	NotifyListenersOfPropertyChange (args, error);
}

Size
FrameworkElement::ApplySizeConstraints (const Size &size)
{
	Size specified (GetWidth (), GetHeight ());
	Size constrained (GetMinWidth (), GetMinHeight ());
	
	constrained = constrained.Max (size);

	if (!isnan (specified.width))
		constrained.width = specified.width;

	if (!isnan (specified.height))
		constrained.height = specified.height;

	constrained = constrained.Min (GetMaxWidth (), GetMaxHeight ());
	constrained = constrained.Max (GetMinWidth (), GetMinHeight ());
	
	if (GetUseLayoutRounding ()) {
		constrained.width = round (constrained.width);
		constrained.height = round (constrained.height);
	}
	return constrained;
}

void
FrameworkElement::ComputeBounds ()
{
	Size size (GetActualWidth (), GetActualHeight ());
	size = ApplySizeConstraints (size);

	extents = Rect (0, 0, size.width, size.height);
	extents_with_children = extents;

	VisualTreeWalker walker = VisualTreeWalker (this, Logical, false);
	while (UIElement *item = walker.Step ()) {
		if (!item->GetRenderVisible ())
			continue;

		extents_with_children = extents_with_children.Union (item->GetGlobalBounds ());
	}

	bounds = IntersectBoundsWithClipPath (extents.GrowBy (effect_padding), false).Transform (&absolute_xform);
	bounds_with_children = extents_with_children.GrowBy (effect_padding).Transform (&absolute_xform);

	ComputeGlobalBounds ();
	ComputeSurfaceBounds ();
}

void
FrameworkElement::ComputeGlobalBounds ()
{
	UIElement::ComputeGlobalBounds ();
	global_bounds_with_children = extents_with_children.GrowBy (effect_padding).Transform (local_projection);
}

void
FrameworkElement::ComputeSurfaceBounds ()
{
	UIElement::ComputeSurfaceBounds ();
	surface_bounds_with_children = extents_with_children.GrowBy (effect_padding).Transform (absolute_projection);
}

Rect
FrameworkElement::GetLocalBounds ()
{
	if (GetSubtreeObject () != NULL) 
		return bounds_with_children;

	return bounds;
}

Rect
FrameworkElement::GetGlobalBounds ()
{
	if (GetSubtreeObject () != NULL)
		return global_bounds_with_children;

	return global_bounds;
}

Rect
FrameworkElement::GetSubtreeBounds ()
{
	if (GetSubtreeObject () != NULL)
		return surface_bounds_with_children;

	return surface_bounds;
}

Rect
FrameworkElement::GetSubtreeExtents ()
{
	if (GetSubtreeObject () != NULL)
		return extents_with_children;

	return extents;
}

Size
FrameworkElement::ComputeActualSize ()
{
	UIElement *parent = GetVisualParent ();

	if (GetVisibility () != VisibilityVisible)
		return Size (0.0, 0.0);

	if ((parent && !parent->Is (Type::CANVAS)) || (IsLayoutContainer ())) 
		return GetRenderSize ();

	Size actual (0, 0);

	actual = ApplySizeConstraints (actual);

	return actual;
}

bool
FrameworkElement::InsideLayoutClip (double x, double y)
{
	Geometry *composite_clip = LayoutInformation::GetCompositeClip (this);	
	bool inside = true;

	if (!composite_clip)
		return inside;

	TransformPoint (&x, &y);
	inside = composite_clip->GetBounds ().PointInside (x, y);
	composite_clip->unref ();

	return inside;
}

bool
FrameworkElement::InsideObject (cairo_t *cr, double x, double y)
{
	Size framework (GetActualWidth (), GetActualHeight ());
	double nx = x, ny = y;
	
	TransformPoint (&nx, &ny);
	if (nx < 0 || ny < 0 || nx > framework.width || ny > framework.height)
		return false;

	if (!InsideLayoutClip (x, y))
		return false;
	
	return UIElement::InsideObject (cr, x, y);
}

void
FrameworkElement::HitTest (cairo_t *cr, Point p, List *uielement_list)
{
	if (!GetRenderVisible ())
		return;

	if (!GetIsHitTestVisible ())
		return;

	/* 
	 * FIXME OPTIMIZEME we can't currently do a bounds check here
	 * because some elements allow hits outside the
	 * rendered area (mainly textblock) and unless that changes
	 * we can't skip whole branches of the subtree this way
	 */
	// first a quick bounds check
	/*
	if (!GetSubtreeBounds().PointInside (p.x, p.y))
		return;
	*/

	/* the clip property is global so we can short out here */
	if (!InsideClip (cr, p.x, p.y))
		return;

	/* create our node and stick it on front */
	List::Node *us = uielement_list->Prepend (new UIElementNode (this));
	bool hit = false;

	VisualTreeWalker walker = VisualTreeWalker (this, ZReverse, false);
	while (UIElement *child = walker.Step ()) {
		child->HitTest (cr, p, uielement_list);

		if (us != uielement_list->First ()) {
			hit = true;
			break;
		}
	}	

	if (!hit && !(CanFindElement () && InsideObject (cr, p.x, p.y)))
		uielement_list->Remove (us);
}

void
FrameworkElement::FindElementsInHostCoordinates (cairo_t *cr, Point host, List *uielement_list)
{
	if (GetVisibility () != VisibilityVisible)
		return;

	if (!GetIsHitTestVisible ())
		return;

	if (GetSubtreeBounds ().height <= 0)
		return;

	/* the clip property is global so we can short out here */
	if (!InsideClip (cr, host.x, host.y))
		return;

	cairo_save (cr);

	/* create our node and stick it on front */
	List::Node *us = uielement_list->Prepend (new UIElementNode (this));

	VisualTreeWalker walker = VisualTreeWalker (this, ZForward, false);
	while (UIElement *child = walker.Step ())
		child->FindElementsInHostCoordinates (cr, host, uielement_list);

	if (us == uielement_list->First ()) {
		cairo_new_path (cr);
		cairo_identity_matrix (cr);

		if (!CanFindElement () || !InsideObject (cr, host.x, host.y))
			uielement_list->Remove (us);
	}
	cairo_restore (cr);
}

// FIXME: This is not the fastest way of implementing this, decomposing the rectangle into
// a series of points is probably going to be quite slow. It's a good first effort.
void
FrameworkElement::FindElementsInHostCoordinates (cairo_t *cr, Rect r, List *uielement_list)
{
	bool res = false;
	if (GetVisibility () != VisibilityVisible)
		return;

	if (!GetIsHitTestVisible ())
		return;
		
	if (GetSubtreeBounds ().height <= 0)
		return;

	if (!GetSubtreeBounds ().IntersectsWith (r))
		return;
	
	cairo_save (cr);
	cairo_new_path (cr);

	Geometry *clip = GetClip ();
	if (clip) {
		if (!r.IntersectsWith (clip->GetBounds ().Transform (absolute_projection))) {
			cairo_restore (cr);
			return;
		}
		r = r.Intersection (clip->GetBounds ().Transform (absolute_projection));
	}

	/* create our node and stick it on front */
	List::Node *us = uielement_list->Prepend (new UIElementNode (this));

	VisualTreeWalker walker = VisualTreeWalker (this, ZForward, false);
	while (UIElement *child = walker.Step ())
		child->FindElementsInHostCoordinates (cr, r, uielement_list);

	if (us == uielement_list->First ()) {
		cairo_new_path (cr);
		cairo_identity_matrix (cr);

		res = false;
		if (CanFindElement ()) {
			res = GetBounds ().Intersection (r) == GetBounds ();
			
			for (int i= r.x; i < (r.x + r.width) && !res; i++)
				for (int j= r.y; j < (r.y + r.height) && !res; j++)
					res = InsideObject (cr, i, j);
		}
		
		if (!res)
			uielement_list->Remove (us);
	}
	cairo_restore (cr);
}

void
FrameworkElement::GetSizeForBrush (cairo_t *cr, double *width, double *height)
{
	*width = GetActualWidth ();
	*height = GetActualHeight ();
}

void
FrameworkElement::MeasureWithError (Size availableSize, MoonError *error)
{
	if (error->number)
		return;

	//LOG_LAYOUT ("measuring %p %s %g,%g\n", this, GetTypeName (), availableSize.width, availableSize.height);

	if (isnan (availableSize.width) || isnan (availableSize.height)) {
		MoonError::FillIn (error, MoonError::INVALID_OPERATION, "Cannot call Measure using a size with NaN values");
		LayoutInformation::SetLayoutExceptionElement (GetDeployment (), this);
		return;
	}

	Size *last = LayoutInformation::GetPreviousConstraint (this);
	bool domeasure = (this->dirty_flags & DirtyMeasure) > 0;

	domeasure |= !last || last->width != availableSize.width || last->height != availableSize.height;

	if (GetVisibility () != VisibilityVisible) {
		LayoutInformation::SetPreviousConstraint (this, &availableSize);
		SetDesiredSize (Size (0,0));
		return;
	}

	ApplyTemplateWithError (error);

	UIElement *parent = GetVisualParent ();
	/* unit tests show a short circuit in this case */
	/*
	if (!parent && !IsContainer () && (!GetSurface () || (GetSurface () && !GetSurface ()->IsTopLevel (this)))) {
		SetDesiredSize (Size (0,0));
		return;
	}
	*/

	if (!domeasure)
		return;

	LayoutInformation::SetPreviousConstraint (this, &availableSize);

	InvalidateArrange ();
	UpdateBounds ();

	Thickness margin = *GetMargin ();
	Size size = availableSize.GrowBy (-margin);

	size = ApplySizeConstraints (size);

	if (measure_cb)
		size = (*measure_cb)(this, size, error);
	else
		size = MeasureOverrideWithError (size, error);

	if (error->number)
		return;

	dirty_flags &= ~DirtyMeasure;
	hidden_desire = size;

	if (!parent || parent->Is (Type::CANVAS)) {
		if (Is (Type::CANVAS) || !IsLayoutContainer ()) {
			SetDesiredSize (Size (0,0));
			return;
		}
	}

	// postcondition the results
	size = ApplySizeConstraints (size);

	size = size.GrowBy (margin);
	size = size.Min (availableSize);

	if (GetUseLayoutRounding ()) {
		size.width = round (size.width);
		size.height = round (size.height);
	}
	
	SetDesiredSize (size);
}

Size
FrameworkElement::MeasureOverrideWithError (Size availableSize, MoonError *error)
{
	Size desired = Size (0,0);

	availableSize = availableSize.Max (desired);

	VisualTreeWalker walker = VisualTreeWalker (this);
	while (UIElement *child = walker.Step ()) {
		child->MeasureWithError (availableSize, error);
		desired = child->GetDesiredSize ();
	}

	return desired.Min (availableSize);
}


// not sure about the disconnect between these two methods..  I would
// imagine both should take Rects and ArrangeOverride would return a
// rectangle as well..
void
FrameworkElement::ArrangeWithError (Rect finalRect, MoonError *error)
{
	if (error->number)
		return;

	//LOG_LAYOUT ("arranging %p %s %g,%g,%g,%g\n", this, GetTypeName (), finalRect.x, finalRect.y, finalRect.width, finalRect.height);
	Value *slotValue = ReadLocalValue (LayoutInformation::LayoutSlotProperty);
	Rect *slot = Value::IsNull (slotValue) ? NULL : slotValue->AsRect ();
	bool doarrange = this->dirty_flags & DirtyArrange;
	
	if (GetUseLayoutRounding ()) {
		Rect rounded;
		rounded.x = round (finalRect.x);
		rounded.y = round (finalRect.y);
		//rounded.width = MAX (round ((finalRect.x + finalRect.width) - rounded.x), 0);
		//rounded.height = MAX (round ((finalRect.y + finalRect.height) - rounded.y), 0);
		rounded.width = round (finalRect.width);
		rounded.height = round (finalRect.height);
 		finalRect = rounded;
	}

	doarrange |= slot ? *slot != finalRect : true;

	if (finalRect.width < 0 || finalRect.height < 0 
	    || isinf (finalRect.width) || isinf (finalRect.height)
	    || isnan (finalRect.width) || isnan (finalRect.height)) {
		Size desired = GetDesiredSize ();
		g_warning ("invalid arguments to Arrange (%g,%g,%g,%g) Desired = (%g,%g)", finalRect.x, finalRect.y, finalRect.width, finalRect.height, desired.width, desired.height);
		return;
	}

	UIElement *parent = GetVisualParent ();
	/* unit tests show a short circuit in this case */
	/*
	if (!parent && !IsContainer () && (!GetSurface () || (GetSurface () && !GetSurface ()->IsTopLevel (this)))) {
		return;
	}
	*/
	if (GetVisibility () != VisibilityVisible) {
		LayoutInformation::SetLayoutSlot (this, &finalRect);
		return;
	}

	if (!doarrange)
		return;

	/*
	 * FIXME I'm not happy with doing this here but until I come
	 * up with a better plan make sure that layout elements have
	 * been measured at least once
	 */
        Size *measure = LayoutInformation::GetPreviousConstraint (this);
	if (IsContainer () && !measure)
		MeasureWithError (Size (finalRect.width, finalRect.height), error);
	measure = LayoutInformation::GetPreviousConstraint (this);

	ClearValue (LayoutInformation::LayoutClipProperty);

	Thickness margin = *GetMargin ();
	Rect child_rect = finalRect.GrowBy (-margin);

	UpdateTransform ();
	UpdateProjection ();
	UpdateBounds ();

	Size offer = hidden_desire;
	Size response;

	Size stretched = ApplySizeConstraints (Size (child_rect.width, child_rect.height));
	Size framework = ApplySizeConstraints (Size ());

	HorizontalAlignment horiz = GetHorizontalAlignment ();
	VerticalAlignment vert = GetVerticalAlignment ();
       
	if (horiz == HorizontalAlignmentStretch)
		framework.width = MAX (framework.width, stretched.width);

	if (vert == VerticalAlignmentStretch)
		framework.height = MAX (framework.height, stretched.height);

	offer = offer.Max (framework);

	LayoutInformation::SetLayoutSlot (this, &finalRect);

	if (arrange_cb)
		response = (*arrange_cb)(this,offer, error);
	else
		response = ArrangeOverrideWithError (offer, error);

	bool flip_horiz = false;
	
	if (parent)
		flip_horiz = ((FrameworkElement *)parent)->GetFlowDirection () != GetFlowDirection ();
	else if (GetParent () && GetParent ()->Is (Type::POPUP)) {
		flip_horiz = ((FrameworkElement *)GetParent())->GetFlowDirection () != GetFlowDirection ();		
	} else {
		flip_horiz = GetFlowDirection() == FlowDirectionRightToLeft;
	}

	cairo_matrix_init_identity (&layout_xform);
	cairo_matrix_translate (&layout_xform, child_rect.x, child_rect.y);
	if (flip_horiz) {
		cairo_matrix_translate (&layout_xform, offer.width, 0.0);
		cairo_matrix_scale (&layout_xform, -1, 1);
	}

	if (error->number)
		return;

	this->dirty_flags &= ~DirtyArrange;
	Point visual_offset (child_rect.x, child_rect.y);
	LayoutInformation::SetVisualOffset (this, &visual_offset);

	Size old_size = GetRenderSize ();

	if (GetUseLayoutRounding ()) {
		response.width = round (response.width);
		response.height = round (response.height);
	}

	SetRenderSize (response);
	Size constrainedResponse = response.Min (ApplySizeConstraints (response));

	if (!parent || parent->Is (Type::CANVAS)) {
		if (!IsLayoutContainer ()) {
			SetRenderSize (Size (0,0));
			return;
		}
	}

	/* it doesn't appear we apply aligment or layout clipping to toplevel elements */
	bool toplevel = IsAttached () && GetDeployment ()->GetSurface ()->IsTopLevel (this);

	if (!toplevel) {
		switch (horiz) {
		case HorizontalAlignmentLeft:
			break;
		case HorizontalAlignmentRight:
			visual_offset.x += child_rect.width - constrainedResponse.width;
			break;
		case HorizontalAlignmentCenter:
			visual_offset.x += (child_rect.width - constrainedResponse.width) * .5;
			break;
		default:
			visual_offset.x += MAX ((child_rect.width  - constrainedResponse.width) * .5, 0);
			break;
		}
		
		switch (vert) {
		case VerticalAlignmentTop:
			break;
		case VerticalAlignmentBottom:
			visual_offset.y += child_rect.height - constrainedResponse.height;
			break;
		case VerticalAlignmentCenter:
			visual_offset.y += (child_rect.height - constrainedResponse.height) * .5;
			break;
		default:
			visual_offset.y += MAX ((child_rect.height - constrainedResponse.height) * .5, 0);

			break;
		}
	}

	if (GetUseLayoutRounding ()) {
		visual_offset.x = round (visual_offset.x);
		visual_offset.y = round (visual_offset.y);
	}

	cairo_matrix_init_identity (&layout_xform);
	cairo_matrix_translate (&layout_xform, visual_offset.x, visual_offset.y);
	if (flip_horiz) {
		cairo_matrix_translate (&layout_xform, response.width, 0);
		cairo_matrix_scale (&layout_xform, -1, 1);
	}

	LayoutInformation::SetVisualOffset (this, &visual_offset);

	Rect element (0, 0, response.width, response.height);
	Rect layout_clip = child_rect;
	layout_clip.x = MAX (child_rect.x - visual_offset.x, 0);
	layout_clip.y = MAX (child_rect.y - visual_offset.y, 0);
	if (GetUseLayoutRounding ()) {
		layout_clip.x = round (layout_clip.x);
		layout_clip.y = round (layout_clip.y);
	}

	if (((!toplevel && element != element.Intersection (layout_clip)) || constrainedResponse != response) && !Is (Type::CANVAS) && ((parent && !parent->Is (Type::CANVAS)) || IsContainer ())) {
		Size framework_clip = ApplySizeConstraints (Size (INFINITY, INFINITY));
		layout_clip = layout_clip.Intersection (Rect (0, 0, framework_clip.width, framework_clip.height));
		RectangleGeometry *rectangle = MoonUnmanagedFactory::CreateRectangleGeometry ();
		rectangle->SetRect (&layout_clip);
		LayoutInformation::SetLayoutClip (this, rectangle);
		rectangle->unref ();
	}

	if (old_size != response) { // || (old_offset && *old_offset != visual_offset)) {
		if (!LayoutInformation::GetLastRenderSize (this)) {
			LayoutInformation::SetLastRenderSize (this, &old_size);
			PropagateFlagUp (DIRTY_SIZE_HINT);
		}
	}
}

Size
FrameworkElement::ArrangeOverrideWithError (Size finalSize, MoonError *error)
{
	Size arranged = finalSize;

	VisualTreeWalker walker = VisualTreeWalker (this);
	while (UIElement *child = walker.Step ()) {
		Rect childRect (0,0,finalSize.width,finalSize.height);

		child->ArrangeWithError (childRect, error);
		arranged = arranged.Max (finalSize);
	}

	return arranged;
}

void
FrameworkElement::UpdateLayoutWithError (MoonError *error)
{
	if (IsAttached ()) {
		GetDeployment ()->GetSurface ()->UpdateLayout (error);
	} else {
		LayoutPass *pass = new LayoutPass ();
		UpdateLayer (pass, error);
		if (pass->updated) {
			GetDeployment ()->LayoutUpdated ();		
		}
		delete pass;
	}	
}

void
FrameworkElement::UpdateLayer (LayoutPass *pass, MoonError *error)
{
	UIElement *element = this;
	UIElement *parent = NULL;

	// Seek to the top
	while ((parent = element->GetVisualParent ())) {
		element = parent;
	}

        LOG_LAYOUT ("\nFrameworkElement::UpdateLayout: ");
	while (pass->count < LayoutPass::MaxCount) {
		LOG_LAYOUT ("\u267c");
		
		// If we abort the arrange phase because InvalidateMeasure was called or if
		// we abort the size phase because InvalidateMeasure/InvalidateArrange
		// was called, we need to put the hint flags back otherwise we'll skip that
		// branch during the next pass.
		while (UIElementNode *node = (UIElementNode *)pass->arrange_list->First ()) {
			node->uielement->PropagateFlagUp (DIRTY_ARRANGE_HINT);
			pass->arrange_list->Remove (node);
		}
		while (UIElementNode *node = (UIElementNode *)pass->size_list->First ()) {
			node->uielement->PropagateFlagUp (DIRTY_SIZE_HINT);
			pass->size_list->Remove (node);
		}
		
		pass->count = pass->count +1;
		// Figure out which type of elements we should be selected - dirty measure, arrange or size
		UIElementFlags flag = NONE;
		if (element->GetVisibility () == VisibilityVisible) {
			if (element->HasFlag (DIRTY_MEASURE_HINT))
				flag = DIRTY_MEASURE_HINT;
			else if (element->HasFlag (DIRTY_ARRANGE_HINT))
				flag = DIRTY_ARRANGE_HINT;
			else if (element->HasFlag (DIRTY_SIZE_HINT))
				flag = DIRTY_SIZE_HINT;
		}

		if (flag != NONE) {
			DeepTreeWalker measure_walker (element);
			while (FrameworkElement *child = (FrameworkElement *)measure_walker.Step ()) {
				if (child->GetVisibility () != VisibilityVisible || !child->HasFlag (flag)) {
					measure_walker.SkipBranch ();
					continue;
				}

				child->ClearFlag (flag);
				switch (flag) {
					case DIRTY_MEASURE_HINT:
						if (child->dirty_flags & DirtyMeasure)
							pass->measure_list->Append (new UIElementNode (child));
					break;
					case DIRTY_ARRANGE_HINT:
						if (child->dirty_flags & DirtyArrange)
							pass->arrange_list->Append (new UIElementNode (child));
					break;
					case DIRTY_SIZE_HINT:
						if (child->ReadLocalValue (LayoutInformation::LastRenderSizeProperty))
							pass->size_list->Append (new UIElementNode (child));
					break;
					default:
					break;
				}
			}
		}

		if (flag == DIRTY_MEASURE_HINT) {
			while (UIElementNode* node = (UIElementNode*)pass->measure_list->First ()) {
				pass->measure_list->Unlink (node);
				
				node->uielement->DoMeasureWithError (error);
				
				pass->updated = true;
				delete (node);
			}
		} else if (flag == DIRTY_ARRANGE_HINT) {
			while (UIElementNode *node = (UIElementNode*)pass->arrange_list->First ()) {
				pass->arrange_list->Unlink (node);
				
				node->uielement->DoArrangeWithError (error);
			
				pass->updated = true;
				delete (node);
				if (element->HasFlag (DIRTY_MEASURE_HINT))
					break;
			}
		} else if (flag == DIRTY_SIZE_HINT) {
			while (UIElementNode *node = (UIElementNode*)pass->size_list->First ()) {
				//if (element->HasFlag (DIRTY_MEASURE_HINT) ||
				//	element->HasFlag (DIRTY_ARRANGE_HINT)) {
				//	break;
				//}

				pass->size_list->Unlink (node);
				FrameworkElement *fe = (FrameworkElement*) node->uielement;

				pass->updated = true;
				Size *last = LayoutInformation::GetLastRenderSize (fe);
				if (last) {
					Size last_v = *last;
					fe->ClearValue (LayoutInformation::LastRenderSizeProperty, false);

					if (fe->HasHandlers (FrameworkElement::SizeChangedEvent)) {
						SizeChangedEventArgs *args = new SizeChangedEventArgs (last_v, fe->GetRenderSize ());
						fe->Emit (FrameworkElement::SizeChangedEvent, args);
					}
				}
				delete (node);
			}
		} else {
				break;
		}
	}
	
	if (pass->count < LayoutPass::MaxCount) {
		LOG_LAYOUT (" (%d)\n", pass->count);

#if SANITY
		DeepTreeWalker verifier (element);
		while (UIElement *e = verifier.Step ()) {
			if (e->GetVisibility () != VisibilityVisible) {
				verifier.SkipBranch ();
				continue;
			}
			if (e->dirty_flags & DirtyMeasure)
				g_warning ("%s still has dirty measure after the layout pass\n", e->GetType ()->GetName());
			if (e->dirty_flags & DirtyArrange)
				g_warning ("%s still has dirty arrange after the layout pass\n", e->GetType ()->GetName());
			if (e->ReadLocalValue (LayoutInformation::LastRenderSizeProperty))
				g_warning ("%s still has LastRenderSize after the layout pass\n", e->GetType ()->GetName());
		}
#endif
	}
}

void
FrameworkElement::RegisterManagedOverrides (MeasureOverrideCallback measure_cb, ArrangeOverrideCallback arrange_cb,
					    GetDefaultTemplateCallback get_default_template_cb, LoadedCallback loaded_cb,
					    StyleResourceChangedCallback style_resource_changed_cb)
{
	this->measure_cb = measure_cb;
	this->arrange_cb = arrange_cb;
	this->get_default_template_cb = get_default_template_cb;
	this->loaded_cb = loaded_cb;
	this->style_resource_changed_cb = style_resource_changed_cb;
}

void
FrameworkElement::OnIsAttachedChanged (bool attached)
{
	UIElement::OnIsAttachedChanged (attached);
}


void
FrameworkElement::OnIsLoadedChanged (bool loaded)
{
	if (loaded)
		SetImplicitStyles (ImplicitStylePropertyValueProvider::StyleMaskAll);
	else
		ClearImplicitStyles (ImplicitStylePropertyValueProvider::StyleMaskVisualTree);

	UIElement::OnIsLoadedChanged (loaded);
	if (providers.inheriteddatacontext)
		providers.inheriteddatacontext->EmitChanged ();

	if (loaded && loaded_cb)
		(*loaded_cb) (this);
}

bool
FrameworkElement::ApplyTemplateWithError (MoonError *error)
{
	if (GetSubtreeObject ())
		return false;
	
	bool result = DoApplyTemplateWithError (error);
	if (GetSubtreeObject ())
		GetSubtreeObject ()->SetMentor (this);
	if (result)
		OnApplyTemplate ();
	return result;
}

bool
FrameworkElement::DoApplyTemplateWithError (MoonError *error)
{
	UIElement *e = GetDefaultTemplate ();
	if (e) {
		e->SetParent (this, error);
		if (error->number)
			return false;

		if (default_template) {
			default_template->SetParent (NULL, NULL);
			default_template = NULL;
		}

		this->default_template = e;

		SetSubtreeObject (e);
		ElementAdded (e);
	}
	return e != NULL;
}

void
FrameworkElement::OnApplyTemplate ()
{
	Emit (TemplateAppliedEvent);
}

void
FrameworkElement::ElementRemoved (UIElement *obj)
{
	UIElement::ElementRemoved (obj);
	if (GetSubtreeObject () == obj) {
		MoonError e;
		obj->SetParent (NULL, &e);
		obj->SetMentor (NULL);
		SetSubtreeObject (NULL);
	}
}

UIElement *
FrameworkElement::GetDefaultTemplate ()
{
	if (get_default_template_cb)
		return get_default_template_cb (this);
	return NULL;
}

void
FrameworkElement::StyleResourceChanged (const char *key, Style *value)
{
	if (style_resource_changed_cb)
		style_resource_changed_cb (this, key, value);
}

void
FrameworkElement::SetImplicitStyles (ImplicitStylePropertyValueProvider::StyleMask mask, Style **styles)
{
	Application *app = Application::GetCurrent ();
	if (!app)
		return;

	if (styles == NULL)
		styles = app->GetImplicitStyles (this, mask);

	// verify all the styles
	MoonError e;
	DependencyProperty *style_prop = GetDeployment ()->GetTypes ()->GetProperty (FrameworkElement::StyleProperty);

	if (styles) {
		for (int i = 0; i < ImplicitStylePropertyValueProvider::StyleIndexCount; i ++) {
			Style *style = styles[i];
			if (!style)
				continue;

			Value val (style);
			if (!Validators::StyleValidator (this, style_prop, &val, &e)) {
				printf ("Error in the implicit style\n");
				g_free (styles);
				return;
			}
		}
	}
	
	providers.implicitstyle->SetStyles (mask, styles, &e);
}

void
FrameworkElement::ClearImplicitStyles (ImplicitStylePropertyValueProvider::StyleMask style_mask)
{
	MoonError e;
	providers.implicitstyle->ClearStyles (style_mask, &e);
}


};
