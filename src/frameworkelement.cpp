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

#include "geometry.h"
#include "application.h"
#include "runtime.h"
#include "namescope.h"
#include "frameworkelement.h"
#include "trigger.h"
#include "thickness.h"
#include "collection.h"
#include "style.h"
#include "validators.h"

FrameworkElement::FrameworkElement ()
{
	SetObjectType (Type::FRAMEWORKELEMENT);

	default_style_applied = false;
	measure_cb = NULL;
	arrange_cb = NULL;
	bounds_with_children = Rect ();
	logical_parent = NULL;
}

FrameworkElement::~FrameworkElement ()
{
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
FrameworkElement::SetLogicalParent (DependencyObject *logical_parent, MoonError *error)
{
	if (logical_parent && this->logical_parent && this->logical_parent != logical_parent) {
		MoonError::FillIn (error, MoonError::INVALID_OPERATION, "Element is a child of another element");
		return;
	}			

	this->logical_parent = logical_parent;
}

void
FrameworkElement::ElementAdded (UIElement *item)
{
	UIElement::ElementAdded (item);

		//item->UpdateLayout ();
	/*
	if (IsLayoutContainer () && item->Is (Type::FRAMEWORKELEMENT)) {
		FrameworkElement *fe = (FrameworkElement *)item;
		fe->SetActualWidth (0.0);
		fe->SetActualHeight (0.0);
	} 
	*/
}

bool
FrameworkElement::SetValueWithErrorImpl (DependencyProperty *property, Value *value, MoonError *error)
{
	if (value && value->Is (Type::STYLE) && !GetStyle()) {
		//printf ("STYLE WAS SET, this = %s\n", GetTypeName());
		Style *s = value->AsStyle ();
		if (s)
			Application::GetCurrent()->ApplyStyle (this, s);
	}

	return UIElement::SetValueWithErrorImpl (property, value, error);
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
	    args->GetId () == FrameworkElement::MarginProperty) {

		Point *p = GetRenderTransformOrigin ();

		/* normally we'd only update the bounds of this
		   element on a width/height change, but if the render
		   transform is someplace other than (0,0), the
		   transform needs to be updated as well. */
		FullInvalidate (p->x != 0.0 || p->y != 0.0);

		if (IsLayoutContainer () || (GetVisualParent () && GetVisualParent ()->IsLayoutContainer ())) {
			ClearValue (FrameworkElement::ActualHeightProperty);
			ClearValue (FrameworkElement::ActualWidthProperty);
		} else {
			// FIXME: this breaks TextBlock's ActualWidth/Height in the TextBlockTest.cs:ComputeActualWidth() test
			Size actual (GetMinWidth (), GetMinHeight ());
			actual = actual.Max (GetWidth (), GetHeight ());
			actual = actual.Min (GetMaxWidth (), GetMaxHeight ());
			SetActualWidth (actual.width);
			SetActualHeight (actual.height);
		}

		InvalidateMeasure ();
	}
	else if (args->GetId () == FrameworkElement::StyleProperty) {
		if (args->GetNewValue()) {
			Style *s = args->GetNewValue()->AsStyle ();
			if (s) {
				// this has a side effect of calling
				// ProviderValueChanged on all values
				// in the style, so we might end up
				// with lots of property notifications
				// here (reentrancy ok?)
				((StylePropertyValueProvider*)providers[PropertyPrecedence_Style])->SealStyle (s);
			}
		}
	}

	NotifyListenersOfPropertyChange (args);
}

void
FrameworkElement::ComputeBounds ()
{
	extents = Rect (0, 0, GetActualWidth (), GetActualHeight ());
	bounds = IntersectBoundsWithClipPath (extents, false).Transform (&absolute_xform);
	bounds_with_children = bounds;

	VisualTreeWalker walker = VisualTreeWalker (this);
	while (UIElement *item = walker.Step ()) {
		if (!item->GetRenderVisible ())
			continue;

		bounds_with_children = bounds_with_children.Union (item->GetSubtreeBounds ());
	}
}

Rect
FrameworkElement::GetSubtreeBounds ()
{
	VisualTreeWalker walker = VisualTreeWalker (this);
	if (GetSubtreeObject () != NULL) 
		return bounds_with_children;

	return bounds;
}

bool
FrameworkElement::InsideObject (cairo_t *cr, double x, double y)
{
	double width = GetActualWidth ();
	double height = GetActualHeight ();
	double nx = x, ny = y;
	
	TransformPoint (&nx, &ny);
	if (nx < 0 || ny < 0 || nx > width || ny > height)
		return false;

	Geometry *layout_clip = LayoutInformation::GetLayoutClip (this);
	if (layout_clip && !layout_clip->GetBounds ().PointInside (nx, ny))
		return false;

	return UIElement::InsideObject (cr, x, y);
}

void
FrameworkElement::HitTest (cairo_t *cr, Point p, List *uielement_list)
{
	if (!GetRenderVisible ())
		return;

	if (!GetHitTestVisible ())
		return;
	
	// first a quick bounds check
	if (!GetSubtreeBounds().PointInside (p.x, p.y))
		return;

	if (!InsideClip (cr, p.x, p.y))
		return;

	/* create our node and stick it on front */
	List::Node *us = uielement_list->Prepend (new UIElementNode (this));
	bool hit = false;

	VisualTreeWalker walker = VisualTreeWalker (this, ZReverse);
	while (UIElement *child = walker.Step ()) {
		child->HitTest (cr, p, uielement_list);

		if (us != uielement_list->First ()) {
			hit = true;
			break;
		}
	}	

	if (!hit && !InsideObject (cr, p.x, p.y))
		uielement_list->Remove (us);
}

void
FrameworkElement::FindElementsInHostCoordinates (cairo_t *cr, Point host, List *uielement_list)
{
	Point p = host.Transform (&absolute_xform);

	if (!GetRenderVisible ())
		return;

	if (!GetHitTestVisible ())
		return;
	
	cairo_save (cr);
	cairo_new_path (cr);
	
	if (GetClip ()) {
		RenderClipPath (cr, true);
		if (!cairo_in_fill (cr, host.x, host.y)) {
			cairo_restore (cr);
			return;
		}
	}

	/* create our node and stick it on front */
	List::Node *us = uielement_list->Prepend (new UIElementNode (this));

	VisualTreeWalker walker = VisualTreeWalker (this, ZForward);
	while (UIElement *child = walker.Step ())
		child->FindElementsInHostCoordinates (cr, host, uielement_list);

	if (us == uielement_list->First ()) {
		cairo_new_path (cr);
		Region all(extents);
		cairo_set_matrix (cr, &absolute_xform);
		Render (cr, &all, true);
		cairo_identity_matrix (cr);

		if (!CanFindElement () || 
		    !cairo_in_fill (cr, host.x, host.y))
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
FrameworkElement::Measure (Size availableSize)
{
	if (Is(Type::CONTROL)) {
		Control *control = (Control*)this;

		if (control->GetTemplate() && !control->GetSubtreeObject()) {
			control->ApplyTemplate();
			// XXX loaded event?
		}
	}


	Size *last = LayoutInformation::GetLastMeasure (this);
	bool domeasure = this->dirty_flags & DirtyMeasure;
	

	domeasure |= last ? ((*last).width != availableSize.width) && ((*last).height != availableSize.height) : true;

	if (!domeasure)
		return;

	LayoutInformation::SetLastMeasure (this, &availableSize);

	InvalidateArrange ();
	UpdateBounds ();

	this->dirty_flags &= ~DirtyMeasure;

	Size specified = Size (GetWidth (), GetHeight ());
	Thickness margin = *GetMargin ();
	Size size = availableSize.GrowBy (-margin);

	size = size.Min (specified);
	size = size.Max (specified);

	size = size.Min (GetMaxWidth (), GetMaxHeight ());
	size = size.Max (GetMinWidth (), GetMinHeight ());

	if (measure_cb)
		size = (*measure_cb)(size);
	else
		size = MeasureOverride (size);

	/* XXX FIXME horrible hack */
	if (!(IsLayoutContainer () || (GetVisualParent () && GetVisualParent ()->IsLayoutContainer ()))) {
		SetDesiredSize (Size (0,0));
		return;
	}

	// postcondition the results
	size = size.Min (specified);
	size = size.Max (specified);

	size = size.Min (GetMaxWidth (), GetMaxHeight ());
	size = size.Max (GetMinWidth (), GetMinHeight ());

	size = size.GrowBy (margin);
	size = size.Min (availableSize);

	if (GetUseLayoutRounding ()) {
		size.width = round (size.width);
		size.height = round (size.height);
	}

	SetDesiredSize (size);
}

Size
FrameworkElement::MeasureOverride (Size availableSize)
{
	Size desired = Size (0,0);
	Size specified = Size (GetWidth (), GetHeight ());

	if (!IsLayoutContainer ())
		return desired;
	
	availableSize = availableSize.Max (specified);
	availableSize = availableSize.Min (specified);

	VisualTreeWalker walker = VisualTreeWalker (this);
	while (UIElement *child = walker.Step ()) {
		if (child->GetVisibility () != VisibilityVisible)
			continue;
		
		child->Measure (availableSize);
		desired = child->GetDesiredSize ();
	}

	desired = desired.Max (specified);
	desired = desired.Min (specified);

	return desired;
}


// not sure about the disconnect between these two methods..  I would
// imagine both should take Rects and ArrangeOverride would return a
// rectangle as well..
void
FrameworkElement::Arrange (Rect finalRect)
{
	Rect *slot = LayoutInformation::GetLayoutSlot (this);
	bool doarrange = this->dirty_flags & DirtyArrange;
	
	doarrange |= slot ? *slot != finalRect : true;


	if (finalRect.width < 0 || finalRect.height < 0 
	    || isinf (finalRect.width) || isinf (finalRect.height)
	    || isnan (finalRect.width) || isnan (finalRect.height)) {
		Size desired = GetDesiredSize ();
		g_warning ("invalid arguments to Arrange (%g,%g,%g,%g) Desired = (%g,%g)", finalRect.x, finalRect.y, finalRect.width, finalRect.height, desired.width, desired.height);
		return;
	}

	if (!doarrange)
		return;
	

	LayoutInformation::SetLayoutSlot (this, &finalRect);
	LayoutInformation::SetLayoutClip (this, NULL);

	this->dirty_flags &= ~DirtyArrange;

	Thickness margin = *GetMargin ();
	Rect child_rect = finalRect.GrowBy (-margin);

	cairo_matrix_init_translate (&layout_xform, child_rect.x, child_rect.y);
	UpdateTransform ();

	Size offer (child_rect.width, child_rect.height);
	Size response;
	
	HorizontalAlignment horiz = GetHorizontalAlignment ();
	VerticalAlignment vert = GetVerticalAlignment ();

	if (!isnan (GetWidth ()))
		offer.width = GetWidth ();

	if (!isnan (GetHeight ()))
		offer.height = GetHeight ();

	if (arrange_cb)
		response = (*arrange_cb)(offer);
	else
		response = ArrangeOverride (offer);

	Size old (GetActualWidth (), GetActualHeight ());

	/* XXX FIXME horrible hack */
	if (!(IsLayoutContainer () || (GetVisualParent () && GetVisualParent ()->IsLayoutContainer ()))) {
		Size actual (GetMinWidth (), GetMinHeight ());
		actual = actual.Max (GetWidth (), GetHeight ());
		actual = actual.Min (GetMaxWidth (), GetMaxHeight ());
		SetActualWidth (actual.width);
		SetActualHeight (actual.height);
		SetRenderSize (Size (0,0));
		return;
	}

	if (IsLayoutContainer () && GetUseLayoutRounding ()) {
		response.width = round (response.width);
		response.height = round (response.height);
	}

	SetActualWidth (response.width);
	SetActualHeight (response.height);

	if (GetVisualParent ()) {
		switch (horiz) {
		case HorizontalAlignmentLeft:
			break;
		case HorizontalAlignmentRight:
			cairo_matrix_translate (&layout_xform, child_rect.width - response.width, 0);
			break;
		case HorizontalAlignmentCenter:
			cairo_matrix_translate (&layout_xform, (child_rect.width  - response.width) * .5, 0);
			break;
		default:
			cairo_matrix_translate (&layout_xform, MAX ((child_rect.width  - response.width) * .5, 0), 0);
			break;
		}
	
		switch (vert) {
		case VerticalAlignmentTop:
			break;
		case VerticalAlignmentBottom:
			cairo_matrix_translate (&layout_xform, 0, child_rect.height - response.height);
			break;
		case VerticalAlignmentCenter:
			cairo_matrix_translate (&layout_xform, 0, (child_rect.height - response.height) * .5);
			break;
		default:
			cairo_matrix_translate (&layout_xform, 0, MAX ((child_rect.height - response.height) * .5, 0));
			break;
		}
	}

	SetRenderSize (response);
	
	cairo_matrix_t inverse_layout = layout_xform;
	cairo_matrix_invert (&inverse_layout);
	
	Rect layout_clip = finalRect.Transform (&inverse_layout);
	layout_clip = layout_clip.GrowBy (-margin);
	RectangleGeometry *rectangle = new RectangleGeometry ();
	rectangle->SetRect (&layout_clip);
	LayoutInformation::SetLayoutClip (this, rectangle);
	rectangle->unref ();

	if (old != response) {
		SizeChangedEventArgs *args = new SizeChangedEventArgs (old, response);

		Emit(SizeChangedEvent, args);
	}
	// XXX what do we do with finalRect.x and y?
	//printf ("\u231a");
}

Size
FrameworkElement::ArrangeOverride (Size finalSize)
{
	if (!IsLayoutContainer ())
		return finalSize.Max (GetWidth (), GetHeight ());

	Size specified = Size (GetWidth (), GetHeight ());

	finalSize = finalSize.Max (specified);
	finalSize = finalSize.Min (specified);

	Size arranged = finalSize;

	VisualTreeWalker walker = VisualTreeWalker (this);
	while (UIElement *child = walker.Step ()) {
		if (child->GetVisibility () != VisibilityVisible)
			continue;

		Size desired = child->GetDesiredSize ();
		Rect childRect (0,0,finalSize.width,finalSize.height);

		if (GetHorizontalAlignment () != HorizontalAlignmentStretch && isnan (GetWidth ()))
			childRect.width = MIN (desired.width, childRect.width);

		if (GetVerticalAlignment () != VerticalAlignmentStretch && isnan (GetHeight ()))
			childRect.height = MIN (desired.height, childRect.height);

		child->Arrange (childRect);
		arranged = child->GetRenderSize ();

		if (GetHorizontalAlignment () == HorizontalAlignmentStretch || !isnan (GetWidth ()))
			arranged.width = MAX (arranged.width, finalSize.width);
		    
		if (GetVerticalAlignment () == VerticalAlignmentStretch || !isnan (GetHeight()))
			arranged.height = MAX (arranged.height, finalSize.height);
	}

	return arranged;
}

bool
FrameworkElement::UpdateLayout ()
{
	bool rv = UIElement::UpdateLayout ();

	if (rv) {
		// we only emit the event if either a measure or
		// arrange pass was done.
		Emit (LayoutUpdatedEvent);
	}

	return rv;
}

void
FrameworkElement::RegisterManagedOverrides (MeasureOverrideCallback measure_cb, ArrangeOverrideCallback arrange_cb)
{
	this->measure_cb = measure_cb;
	this->arrange_cb = arrange_cb;
}

void
FrameworkElement::SetDefaultStyle (Style *style)
{
	if (!GetStyle()) {
		Application::GetCurrent()->ApplyStyle (this, style);
		((StylePropertyValueProvider*)providers[PropertyPrecedence_Style])->SealStyle (style);
	}
	default_style_applied = true;
}
