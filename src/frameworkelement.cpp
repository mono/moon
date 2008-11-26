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

#include "runtime.h"
#include "frameworkelement.h"
#include "trigger.h"
#include "thickness.h"
#include "collection.h"
#include "binding.h"
#include "style.h"


#if SL_2_0
static void
binding_destroy (gpointer value)
{
	((BindingExpressionBase *) value)->unref ();
}
#endif

FrameworkElement::FrameworkElement ()
{
#if SL_2_0
	bindings = g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL, binding_destroy);
#endif
	measure_cb = NULL;
	arrange_cb = NULL;
}

FrameworkElement::~FrameworkElement ()
{
#if SL_2_0
	g_hash_table_destroy (bindings);
#endif
}

void
FrameworkElement::BoundPropertyChanged (DependencyObject *sender, PropertyChangedEventArgs *args, BindingExpressionBase *expr)
{
#if SL_2_0
	DependencyProperty *property = expr->GetTargetProperty ();
	Binding *binding;
	
	binding = expr->GetBinding ();
	
	// Setting the value will unregister the binding, so grab a
	// ref before we set the new value.
	expr->ref ();
	
	// update the destination property value
	SetValue (property, args->new_value);
	
	// restore the binding
	if (binding->mode != BindingModeOneTime)
		SetBindingExpression (property, expr);
	
	expr->unref ();
#endif
}

void
FrameworkElement::bound_property_changed (DependencyObject *sender, PropertyChangedEventArgs *args, gpointer user_data)
{
#if SL_2_0
	BindingExpressionBase *expr = (BindingExpressionBase *) user_data;
	
	expr->GetTarget ()->BoundPropertyChanged (sender, args, expr);
#endif
}

void
FrameworkElement::SetBindingExpression (DependencyProperty *property, BindingExpressionBase *expr)
{
#if SL_2_0
	BindingExpressionBase *cur_expr = GetBindingExpression (property);
	
	if (cur_expr) {
		cur_expr->DetachListener (FrameworkElement::bound_property_changed);
		g_hash_table_remove (bindings, property);
	}
	
	if (expr) {
		expr->AttachListener (FrameworkElement::bound_property_changed, expr);
		g_hash_table_insert (bindings, property, expr);
		expr->SetTargetProperty (property);
		expr->SetTarget (this);
		expr->ref ();
	}
#endif
}

BindingExpressionBase *
FrameworkElement::GetBindingExpression (DependencyProperty *property)
{
#if SL_2_0
	return (BindingExpressionBase *) g_hash_table_lookup (bindings, property);
#else
	return NULL;
#endif
}

Value *
FrameworkElement::GetLocalValue (DependencyProperty *property)
{
	// Will the Value created here leak? If so, how do we stop it leaking?
	BindingExpressionBase *binding = (BindingExpressionBase *) g_hash_table_lookup (bindings, property);
	return binding ? new Value (binding) : UIElement::GetLocalValue (property);
}

bool
FrameworkElement::IsValueValid (Types *additional_types, DependencyProperty *property, Value *value, MoonError *error)
{
#if SL_2_0
	// We can databind any property of a FrameworkElement 
	if (value && value->Is (Type::BINDINGEXPRESSIONBASE))
		return true;
#endif
	
	return UIElement::IsValueValid (additional_types, property, value, error);
}

bool
FrameworkElement::SetValueWithErrorImpl (DependencyProperty *property, Value *value, MoonError *error)
{
#if SL_2_0
	BindingExpressionBase *cur_binding = GetBindingExpression (property);
	BindingExpressionBase *new_binding = NULL;
	
	if (value && value->Is (Type::BINDINGEXPRESSIONBASE))
		new_binding = value->AsBindingExpressionBase ();
	
	if (new_binding) {
		// We are setting a new data binding; replace the
		// existing binding if there is one.
		SetBindingExpression (property, new_binding);
		value = new_binding->GetValue ();
	} else if (cur_binding) {
		switch (cur_binding->GetBinding ()->mode) {
		case BindingModeTwoWay:
			// We have a two way binding, so we update the
			// source object
			cur_binding->UpdateSource (value);
			break;
		default:
			// Remove the current binding
			SetBindingExpression (property, NULL);
			break;
		}
	}
	
	if (property == FrameworkElement::StyleProperty && GetValue (property)) {
		MoonError::FillIn (error, MoonError::EXCEPTION, 1001,
			g_strdup_printf ("Property 'Style' cannot be assigned to more than once"));
		return false;
	}
	
	bool result = UIElement::SetValueWithErrorImpl (property, value, error);
	Value *styleVal = GetValueNoDefault (FrameworkElement::StyleProperty);
	
	if (result && styleVal) {
		Style *style = styleVal->AsStyle ();
		SetterBaseCollection *setters = style->GetSetters ();
		if (!setters)
			return true;

		CollectionIterator *iter = setters->GetIterator ();
		Value *setterBase;
		int err;
		
		while (iter->Next () && (setterBase = iter->GetCurrent (&err))) {
			if (err) {
		 		// Something bad happened - what to do?
		 	}

			if (!setterBase->Is (Type::SETTER))
				continue;
			
			Setter *setter = setterBase->AsSetter ();
			if (!(value = setter->GetValue (Setter::PropertyProperty)))
				continue;
			
			char *propertyName = value->AsString ();
			if (!(property = DependencyProperty::GetDependencyProperty (GetType ()->GetKind (), propertyName))) {
				continue;
			}
			
			if (!GetValueNoDefault (property)) {
				value = setter->GetValue (Setter::ValueProperty);
				
				// Ensure we don't end up recursing forever - call the base method
				if (!UIElement::SetValueWithErrorImpl(property, value, error))
					return false;
			}
		}
	}
	
	return result;
#else
	return UIElement::SetValueWithErrorImpl (property, value, error);
#endif
}

void
FrameworkElement::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property->GetOwnerType() != Type::FRAMEWORKELEMENT) {
		UIElement::OnPropertyChanged (args);
		return;
	}

	if (args->property == FrameworkElement::WidthProperty ||
#if SL_2_0
	    args->property == FrameworkElement::MaxWidthProperty ||
	    args->property == FrameworkElement::MinWidthProperty ||
	    args->property == FrameworkElement::MaxHeightProperty ||
	    args->property == FrameworkElement::MinHeightProperty ||
#endif
	    args->property == FrameworkElement::HeightProperty) {

		Point *p = GetRenderTransformOrigin ();

		/* normally we'd only update the bounds of this
		   element on a width/height change, but if the render
		   transform is someplace other than (0,0), the
		   transform needs to be updated as well. */
		FullInvalidate (p->x != 0.0 || p->y != 0.0);

		InvalidateMeasure ();
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
	extents = Rect (0.0, 0.0, GetWidth (), GetHeight ());
	bounds = IntersectBoundsWithClipPath (extents, false).Transform (&absolute_xform);
}

bool
FrameworkElement::InsideObject (cairo_t *cr, double x, double y)
{
	double height = GetHeight ();
	double width = GetWidth ();
	double nx = x, ny = y;
	
	TransformPoint (&nx, &ny);
	if (nx < 0 || ny < 0 || nx > width || ny > height)
		return false;
	
	return UIElement::InsideObject (cr, x, y);
}

void
FrameworkElement::GetSizeForBrush (cairo_t *cr, double *width, double *height)
{
	*height = GetHeight ();
	*width = GetWidth ();
}

void
FrameworkElement::Measure (Size availableSize)
{
	this->dirty_flags &= ~DirtyMeasure;

	// XXX this is hack to get around the 0.0 w/h defaults we still
	// have in place due to 1.0
	Value *vw = GetValueNoDefault (FrameworkElement::WidthProperty);
	Value *vh = GetValueNoDefault (FrameworkElement::HeightProperty);
	Size specified = Size (vw ? GetWidth () : NAN, vh ? GetHeight () : NAN);
	
	
#if SL_2_0
	Thickness margin = *GetMargin ();
	Size size = availableSize.GrowBy (-margin);


	size = size.Min (specified);
	size = size.Max (specified);

	size = size.Min (GetMaxWidth (), GetMaxHeight ());
	size = size.Max (GetMinWidth (), GetMinHeight ());
#else
	Size size = availableSize;

	size = size.Min (specified);
	size = size.Max (specified);
#endif

	if (measure_cb)
		size = (*measure_cb)(size);
	else
		size = MeasureOverride (size);

#if SL_2_0
	SetActualWidth (GetWidth ());
	SetActualHeight (GetHeight ());
#endif

	// XXX ugly hack to fake some sort of exception case
	if (isnan (size.width) || isnan (size.height)) {
                SetDesiredSize (Size (0,0));
		return;
        }

	// postcondition the results
	size = size.Min (specified);
	size = size.Max (specified);

#if SL_2_0
	size = size.Min (GetMaxWidth (), GetMaxHeight ());
	size = size.Max (GetMinWidth (), GetMinHeight ());

	size = size.GrowBy (margin);
#endif
	size = size.Min (availableSize);

	SetDesiredSize (size);
}

Size
FrameworkElement::MeasureOverride (Size availableSize)
{
	if (!GetVisualParent () || GetVisualParent ()->Is (Type::CANVAS))
		return Size (NAN,NAN);

	return Size (0,0).GrowBy (GetWidth (), GetHeight ());
}


// not sure about the disconnect between these two methods..  I would
// imagine both should take Rects and ArrangeOverride would return a
// rectangle as well..
void
FrameworkElement::Arrange (Rect finalRect)
{
	this->dirty_flags &= ~DirtyArrange;

#if SL_2_0
	Thickness margin = *GetMargin ();
	finalRect = finalRect.GrowBy (-margin);
#endif
	Size size = Size (finalRect.width, finalRect.height);

	if (arrange_cb)
		size = (*arrange_cb)(size);
	else
		size = ArrangeOverride (size);

	// XXX ugly hack to fake some sort of exception case
	if (isnan (size.width) || isnan (size.height)) {
                SetRenderSize (Size (0,0));
		return;
        }

	SetRenderSize (size);
#if SL_2_0
	double old_width = GetActualWidth ();
	double old_height = GetActualHeight ();

	SetActualWidth (size.width);
	SetActualHeight (size.height);
	if (old_width != size.width || old_height != size.height) {
		SizeChangedEventArgs *args = new SizeChangedEventArgs (Size (old_width, old_height), size);

		Emit(SizeChangedEvent, args);
	}
#endif
	// XXX what do we do with finalRect.x and y?

	g_warning ("more here in FrameworkElement::Arrange.  move the bounds or something?  set properties?  who knows!?");
				     
}

Size
FrameworkElement::ArrangeOverride (Size finalSize)
{
	Size size = finalSize;

	if (!GetVisualParent () || GetVisualParent ()->Is (Type::CANVAS))
		return Size (NAN,NAN);

	Value *vw = GetValueNoDefault (FrameworkElement::WidthProperty);
	Value *vh = GetValueNoDefault (FrameworkElement::HeightProperty);
	Size specified = Size (vw ? GetWidth () : NAN, vh ? GetHeight () : NAN);

	// postcondition the results
	size = size.Min (specified);
	size = size.Max (specified);

	return size;
}

bool
FrameworkElement::UpdateLayout ()
{
	bool rv = UIElement::UpdateLayout ();

#if SL_2_0
	if (rv) {
		// we only emit the event if either a measure or
		// arrange pass was done.
		Emit (LayoutUpdatedEvent);
	}
#endif

	return rv;
}

void
FrameworkElement::RegisterManagedOverrides (MeasureOverrideCallback measure_cb, ArrangeOverrideCallback arrange_cb)
{
	this->measure_cb = measure_cb;
	this->arrange_cb = arrange_cb;
}
