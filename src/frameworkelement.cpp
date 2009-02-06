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
#include "binding.h"
#include "style.h"
#include "validators.h"

static void
binding_destroy (gpointer value)
{
	delete ((Value *) value);
}

static void
invalidate_binding (gpointer key, gpointer value, gpointer user_data)
{
	FrameworkElement *element = (FrameworkElement *) user_data;
	DependencyProperty *property = (DependencyProperty *)key;
	BindingExpressionBase *binding = ((Value *)value)->AsBindingExpressionBase ();
	
	if (!binding->GetSource ())
		element->InvalidateBinding (property, binding);
}

static void
datacontext_changed (DependencyObject *sender, PropertyChangedEventArgs *args, gpointer user_data)
{
	FrameworkElement *element = (FrameworkElement *) user_data;
	g_hash_table_foreach (element->bindings, invalidate_binding, element);
}

FrameworkElement::FrameworkElement ()
{
	SetObjectType (Type::FRAMEWORKELEMENT);

	bindings = g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL, binding_destroy);

	AddPropertyChangeHandler (FrameworkElement::DataContextProperty, datacontext_changed, this);

	measure_cb = NULL;
	arrange_cb = NULL;
	bounds_with_children = Rect ();
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

FrameworkElement::~FrameworkElement ()
{
	g_hash_table_destroy (bindings);

	RemovePropertyChangeHandler (FrameworkElement::DataContextProperty, datacontext_changed);
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

void
FrameworkElement::BoundPropertyChanged (DependencyObject *sender, PropertyChangedEventArgs *args, BindingExpressionBase *expr)
{
	DependencyProperty *property = expr->GetTargetProperty ();
	Binding *binding;
	
	binding = expr->GetBinding ();
	
	// Setting the value will unregister the binding, so grab a
	// ref before we set the new value.
	expr->ref ();
	
	// update the destination property value
	SetValue (property, args->new_value);
	
	// restore the binding
	if (binding->GetBindingMode () != BindingModeOneTime)
		SetBindingExpression (property, expr);
	
	expr->unref ();
}

void
FrameworkElement::bound_property_changed (DependencyObject *sender, PropertyChangedEventArgs *args, gpointer user_data)
{
	BindingExpressionBase *expr = (BindingExpressionBase *) user_data;
	
	expr->GetTarget ()->BoundPropertyChanged (sender, args, expr);
}

void
FrameworkElement::SetBindingExpression (DependencyProperty *property, BindingExpressionBase *expr)
{
	BindingExpressionBase *cur_expr = GetBindingExpression (property);
	
	if (cur_expr)
		ClearBindingExpression (property, cur_expr);
	
	if (expr) {
		expr->AttachListener (this, FrameworkElement::bound_property_changed, expr);
		g_hash_table_insert (bindings, property, new Value (expr));
		expr->SetTargetProperty (property);
		expr->SetTarget (this);
	}
}

BindingExpressionBase *
FrameworkElement::GetBindingExpression (DependencyProperty *property)
{
	Value *value = (Value *) g_hash_table_lookup (bindings, property);
	return value ? value->AsBindingExpressionBase () : NULL;
}

void
FrameworkElement::ClearBindingExpression (DependencyProperty *property, BindingExpressionBase *expr)
{
	expr->DetachListener (FrameworkElement::bound_property_changed);
	g_hash_table_remove (bindings, property);
}

void
FrameworkElement::ClearValue (DependencyProperty *property, bool notify_listeners)
{
	ClearValue (property, notify_listeners, NULL);
}

void
FrameworkElement::ClearValue (DependencyProperty *property, bool notify_listeners, MoonError *error)
{
	BindingExpressionBase *cur_expr = GetBindingExpression (property);
	
	if (cur_expr)
		ClearBindingExpression (property, cur_expr);
	
	UIElement::ClearValue (property, notify_listeners, error);
}

Value *
FrameworkElement::ReadLocalValue (DependencyProperty *property)
{
	Value *binding = (Value *) g_hash_table_lookup (bindings, property);
	
	if (binding)
		return binding;
	
	return UIElement::ReadLocalValue (property);
}

bool
FrameworkElement::SetValueWithErrorImpl (DependencyProperty *property, Value *value, MoonError *error)
{
	bool activeBinding = false;
	BindingExpressionBase *cur_binding = GetBindingExpression (property);
	BindingExpressionBase *new_binding = NULL;
	
	if (value && value->Is (Type::BINDINGEXPRESSIONBASE))
		new_binding = value->AsBindingExpressionBase ();
	
	if (new_binding) {
		// We are setting a new data binding; replace the
		// existing binding if there is one.
		activeBinding = true;
		SetBindingExpression (property, new_binding);
		value = new_binding->GetValue ();
	} else if (cur_binding) {
		switch (cur_binding->GetBinding ()->GetBindingMode ()) {
		case BindingModeTwoWay:
			// We have a two way binding, so we update the
			// source object
			cur_binding->UpdateSource (value);
			activeBinding = true;
			break;
		default:
			// Remove the current binding
			SetBindingExpression (property, NULL);
			break;
		}
	}
	
	if (value && value->Is (Type::STYLE) && !GetStyle()) {
		printf ("STYLE WAS SET, this = %s\n", GetTypeName());
		Style *s = value->AsStyle ();
		if (s)
			Application::GetCurrent()->ApplyStyle (this, s);
	}

	bool result = true;
	if (value == NULL && activeBinding)
		UIElement::ClearValue (property);
	else
		result = UIElement::SetValueWithErrorImpl (property, value, error);


	return result;
}


void
FrameworkElement::InvalidateBinding (DependencyProperty *property, BindingExpressionBase *binding)
{
	binding->SetGotValue (false);
	
	// Reset the binding so the new value is used
	SetValue (property, binding);
}

void
FrameworkElement::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property->GetOwnerType() != Type::FRAMEWORKELEMENT) {
		UIElement::OnPropertyChanged (args);
		return;
	}

	if (args->property == FrameworkElement::WidthProperty ||
	    args->property == FrameworkElement::MaxWidthProperty ||
	    args->property == FrameworkElement::MinWidthProperty ||
	    args->property == FrameworkElement::MaxHeightProperty ||
	    args->property == FrameworkElement::MinHeightProperty ||
	    args->property == FrameworkElement::HeightProperty ||
	    args->property == FrameworkElement::MarginProperty) {

		Point *p = GetRenderTransformOrigin ();

		/* normally we'd only update the bounds of this
		   element on a width/height change, but if the render
		   transform is someplace other than (0,0), the
		   transform needs to be updated as well. */
		FullInvalidate (p->x != 0.0 || p->y != 0.0);

		if (IsLayoutContainer () || (GetVisualParent () && GetVisualParent ()->IsLayoutContainer ())) {
			SetActualWidth (0);
			SetActualHeight (0);
		} else {
			Size actual (GetMinWidth (), GetMinHeight ());
			actual = actual.Max (GetWidth (), GetHeight ());
			actual = actual.Min (GetMaxWidth (), GetMaxHeight ());
			SetActualWidth (actual.width);
			SetActualHeight (actual.height);
		}

		InvalidateMeasure ();
	}
	else if (args->property == FrameworkElement::StyleProperty) {
		if (args->new_value) {
			Style *s = args->new_value->AsStyle ();
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

#if 0
void
FrameworkElement::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	if (prop == FrameworkElement::DataContextProperty) {
		// FIXME: do data binding stuff
	}
	
	UIElement::OnSubPropertyChanged (prop, obj, subobj_args);
}
#endif

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
FrameworkElement::FindElementsInHostCoordinates (cairo_t *cr, Point p, List *uielement_list)
{
	if (!GetRenderVisible ())
		return;

	if (!GetHitTestVisible ())
		return;
	
	// first a quick bounds check
	Rect r(0, 0, GetWidth (), GetHeight ());
	if (r.IsEmpty (true) || !r.GrowBy (1, 1, 1, 0).PointInside (p.x, p.y))
		return;

	if (!InsideClip (cr, p.x, p.y))
		return;

	/* create our node and stick it on front */
	List::Node *us = uielement_list->Prepend (new UIElementNode (this));

	VisualTreeWalker walker = VisualTreeWalker (this, ZForward);
	while (UIElement *child = walker.Step ())
		child->FindElementsInHostCoordinates (cr, p, uielement_list);

	if (us == uielement_list->First () && !CanFindElement ())
		uielement_list->Remove (us);
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
