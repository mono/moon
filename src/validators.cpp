/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * validators.cpp: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include <config.h>

#include <math.h>

#include "validators.h"
#include "thickness.h"
#include "cornerradius.h"
#include "style.h"
#include "frameworkelement.h"
#include "animation.h"
#include "propertypath.h"
#include "namescope.h"

bool
Validators::StyleValidator (DependencyObject* instance, DependencyProperty *property, Value *value, MoonError *error) {
	Value *current = instance->GetValue (property);
	if (current && !current->GetIsNull ()) {
		MoonError::FillIn (error, MoonError::EXCEPTION, 1001,
			g_strdup_printf ("Property 'Style' cannot be assigned to more than once\n"));
		return false;
	}
	return true;
}

bool
Validators::AudioStreamIndexValidator (DependencyObject* instance, DependencyProperty *property, Value *value, MoonError *error)
{
	// A value of -1 is converted to null. Any other value is left as-is
	if (value && value->AsInt32() == -1)
		value->SetIsNull (true);
		
	return true;
}

bool
Validators::BalanceValidator (DependencyObject* instance, DependencyProperty *property, Value *value, MoonError *error)
{
	if (value) {
		if (value->AsDouble () > 1.0) {
			*value = Value(1.0);
		} else if (value->AsDouble () < -1.0) {
			*value = Value(-1.0);
		}
	}
	
	return true;
}

bool
Validators::VolumeValidator (DependencyObject* instance, DependencyProperty *property, Value *value, MoonError *error)
{
	if (value) {
		if (value->AsDouble () > 1.0) {
			*value = Value (1.0);
		} else if (value->AsDouble () < 0.0) {
			*value = Value (0.0);
		}
	}
	
	return true;
}

bool
Validators::CursorValidator (DependencyObject* instance, DependencyProperty *property, Value *value, MoonError *error)
{
	// If the value is null, it means the default cursor has been set.
	if (value->GetIsNull ())
		*value = Value ((int) MouseCursorDefault);

	return true;
}

bool
Validators::default_validator (DependencyObject* instance, DependencyProperty *property, Value *value, MoonError *error)
{
	return true;	
}

bool
Validators::PositiveIntValidator (DependencyObject* instance, DependencyProperty *property, Value *value, MoonError *error)
{
	if (value->AsInt32() < 0) {
		MoonError::FillIn (error, MoonError::ARGUMENT, 1001, "Value must be greater than or equal to zero");
		return false;
	}
	return true;
}

bool
Validators::IntGreaterThanZeroValidator (DependencyObject* instance, DependencyProperty *property, Value *value, MoonError *error)
{
	if (value->AsInt32() < 1) {
		MoonError::FillIn (error, MoonError::ARGUMENT, 1001, "Value must be greater than zero");
		return false;
	}
	return true;
}

bool
Validators::IsInputMethodEnabledValidator (DependencyObject* instance, DependencyProperty *property, Value *value, MoonError *error)
{
	if (!instance->Is (Type::TEXTBOX)) {
		MoonError::FillIn (error, MoonError::ARGUMENT, 1001, "Target object must be a TextBox");
		return false;
	}
	return true;
}

bool
Validators::DoubleGreaterThanZeroValidator (DependencyObject* instance, DependencyProperty *property, Value *value, MoonError *error)
{
	if (value->AsDouble () <= 0) {
		MoonError::FillIn (error, MoonError::ARGUMENT, 1001, "Value must be greater than zero");
		return false;
	}
	return true;
}

bool
Validators::NonNullValidator (DependencyObject* instance, DependencyProperty *property, Value *value, MoonError *error)
{
	if (!value || value->GetIsNull ()) {
		MoonError::FillIn (error, MoonError::ARGUMENT, 1001, "Value cannot be null");
		return false;
	}
	
	return true;
}

bool
Validators::NotNullOrEmptyValidator (DependencyObject* instance, DependencyProperty *property, Value *value, MoonError *error)
{
	if (!value || value->GetIsNull () || strlen (value->AsString ()) == 0) {
		MoonError::FillIn (error, MoonError::EXCEPTION, 1001, "Value cannot be null");
		return false;
	}
	
	return true;
}

bool
Validators::MediaAttributeCollectionValidator (DependencyObject* instance, DependencyProperty *property, Value *value, MoonError *error)
{
	if (!value || value->GetIsNull ()) {
		MoonError::FillIn (error, MoonError::EXCEPTION, 1001, "Value cannot be null");
		return false;
	}
	return true;
}

bool
Validators::TemplateValidator (DependencyObject* instance, DependencyProperty *property, Value *value, MoonError *error)
{
#if false
	// this causes DRT #438 to throw an exception and subsequently
	// timeout.
	if (!value || value->GetIsNull ()) {
		MoonError::FillIn (error, MoonError::EXCEPTION, 1001, "Value cannot be null");
		return false;
	}
#endif
	if (instance->Is(Type::USERCONTROL)) {
		MoonError::FillIn (error, MoonError::INVALID_OPERATION, 1001, "Cannot set the template property on a UserControl");
		return false;
	}
	return true;
}

bool
Validators::NameValidator (DependencyObject* instance, DependencyProperty *property, Value *value, MoonError *error)
{
	NameScope *scope = instance->FindNameScope ();
	if (scope && value) {
		DependencyObject *o = scope->FindName (value->AsString ());
		if (o && o != instance) {
			MoonError::FillIn (error, MoonError::ARGUMENT, 2028,
					g_strdup_printf ("The name already exists in the tree: %s (%p %p).",
							value->AsString (), o, instance));
			return false;
		}
	}
	// TODO: Name validation
	// This doesn't happen in 1.0 or 2.0b according to my tests, but according to the 2.0 docs
	// names need to start with a '_' or letter.  They can't start with a _.  Also characters
	// should be limited to a-z A-Z 0-9 and _.  Once a newer beta actually enforces this
	// I'll implement the validation method.
	return true;
}

bool RangeCheck (double d)
{
	bool b = (d > -(1E300)) && (d < (1E300));
	return b;
}

bool
Validators::BorderThicknessValidator (DependencyObject* instance, DependencyProperty *property, Value *value, MoonError *error)
{
	Thickness t = *value->AsThickness ();

	if (!RangeCheck (t.left) || !RangeCheck (t.right) || !RangeCheck (t.top) || !RangeCheck (t.bottom)){
		MoonError::FillIn (error, MoonError::EXCEPTION, 1001, "Value is out of range");
		return false;
	}

	if (t.left < 0 || t.right < 0 || t.top < 0 || t.bottom < 0){
		MoonError::FillIn (error, MoonError::ARGUMENT, 1001, "Value is out of range");
		return false;
	}
	return true;
}

bool
Validators::CornerRadiusValidator (DependencyObject* instance, DependencyProperty *property, Value *value, MoonError *error)
{
	CornerRadius t = *value->AsCornerRadius ();

	if (!RangeCheck (t.topLeft) || !RangeCheck (t.topRight) || !RangeCheck (t.bottomLeft) || !RangeCheck (t.bottomRight)){
		MoonError::FillIn (error, MoonError::EXCEPTION, 1001, "Value is out of range");
		return false;
	}

	if (t.topLeft < 0 || t.topRight < 0 || t.bottomLeft < 0 || t.bottomRight < 0){
		MoonError::FillIn (error, MoonError::ARGUMENT, 1001, "Value is out of range");
		return false;
	}
	return true;
}

bool
Validators::BufferingTimeValidator (DependencyObject* instance, DependencyProperty *property, Value *value, MoonError *error)
{
	if (value->AsTimeSpan () > 21427200000000000LL) {
		MoonError::FillIn (error, MoonError::EXCEPTION, 1001, "Value is out of range");
		return false;
	}
	
	return true;
}

bool
Validators::IsTimelineValidator (DependencyObject* instance, DependencyProperty *property, Value *value, MoonError *error)
{
	if (!instance->Is (Type::TIMELINE)) {
		MoonError::FillIn (error, MoonError::EXCEPTION, 1001, "Instance is not a Timeline");
		return false;
	}
	
	return true;
}

bool
Validators::StoryboardTargetPropertyValidator (DependencyObject* instance, DependencyProperty *property, Value *value, MoonError *error)
{
	if (!IsTimelineValidator (instance, property, value, error))
		return false;

	PropertyPath *existing = Storyboard::GetTargetProperty (instance);

	if (existing && existing->property != NULL) {
		// it was initialized using a DP, we only allow it to be overriden with another DP.
		PropertyPath *new_path = value->AsPropertyPath();
		if (new_path->property == NULL)
			return false;
	}

	return true;
}

bool
Validators::IsSetterSealedValidator (DependencyObject* instance, DependencyProperty *property, Value *value, MoonError *error)
{
	if (instance->Is (Type::SETTERBASE)) {
		if (((SetterBase*) instance)->GetIsSealed ()) {
			MoonError::FillIn (error, MoonError::UNAUTHORIZED_ACCESS, "Cannot modify a setter after it is used");
			return false;
		}
	}
	
	return true;
}


bool
Validators::ContentControlContentValidator (DependencyObject* instance, DependencyProperty *property, Value *value, MoonError *error)
{
	if (value->Is (instance->GetDeployment (), Type::FRAMEWORKELEMENT)) {
		FrameworkElement *fwe = value->AsFrameworkElement();

		if (fwe->GetLogicalParent () && fwe->GetLogicalParent ()->Is (Type::PANEL)) {
			MoonError::FillIn (error, MoonError::ARGUMENT, "Content is already a child of another element");
			return false;
		}
	}

	return true;
}

bool
Validators::CrossDomainValidator (DependencyObject* instance, DependencyProperty *property, Value *value, MoonError *error)
{
	if (instance->GetValueNoDefault (property)) {
		MoonError::FillIn (error, MoonError::ARGUMENT, 1001,
			g_strdup_printf ("Property 'ExternalCallersFromCrossDomain' cannot be changed.\n"));
		return false;
	}
	return true;
}

bool
Validators::FloatValidator (DependencyObject *instance, DependencyProperty *property, Value *value, MoonError *error)
{
	double d = value->AsDouble ();
	
	switch (fpclassify (d)) {
	case FP_SUBNORMAL:
	case FP_INFINITE:
	case FP_NAN:
		MoonError::FillIn (error, MoonError::EXCEPTION, 1001, "Value is out of range");
		return false;
	default:
		if ((float) d < -HUGE || (float) d > HUGE) {
			MoonError::FillIn (error, MoonError::EXCEPTION, 1001, "Value is out of range");
			return false;
		}
	}
	
	return true;
}
