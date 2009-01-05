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

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "validators.h"

bool
Validators::default_validator (DependencyObject* instance, Value *value, MoonError *error)
{
	return true;	
}

bool
Validators::PositiveIntValidator (DependencyObject* instance, Value *value, MoonError *error)
{
	if (value->AsInt32() < 0) {
		MoonError::FillIn (error, MoonError::ARGUMENT, 1001, "Value must be greater than or equal to zero");
		return false;
	}
	return true;
}

bool
Validators::IntGreaterThanZeroValidator (DependencyObject* instance, Value *value, MoonError *error)
{
	if (value->AsInt32() < 1) {
		MoonError::FillIn (error, MoonError::ARGUMENT, 1001, "Value must be greater than zero");
		return false;
	}
	return true;
}

bool
Validators::NonNullStringValidator (DependencyObject* instance, Value *value, MoonError *error)
{
	if (!value || value->GetIsNull ()) {
		MoonError::FillIn (error, MoonError::ARGUMENT, 1001, "Value cannot be null");
		return false;
	}
	
	return true;
}

bool
Validators::MediaAttributeCollectionValidator (DependencyObject* instance, Value *value, MoonError *error)
{
	if (!value || value->GetIsNull ()) {
		MoonError::FillIn (error, MoonError::EXCEPTION, 1001, "Value cannot be null");
		return false;
	}
	return true;
}

bool
Validators::TemplateValidator (DependencyObject* instance, Value *value, MoonError *error)
{
	if (!value || value->GetIsNull ()) {
		MoonError::FillIn (error, MoonError::EXCEPTION, 1001, "Value cannot be null");
		return false;
	}
	if (instance->Is(Type::USERCONTROL)) {
		MoonError::FillIn (error, MoonError::INVALID_OPERATION, 1001, "Cannot set the template property on a UserControl");
		return false;
	}
	return true;
}

bool RangeCheck (double d)
{
	bool b = (d > -(1E300)) && (d < (1E300));
	return b;
}

bool
Validators::BorderThicknessValidator (DependencyObject* instance, Value *value, MoonError *error)
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
Validators::CornerRadiusValidator (DependencyObject* instance, Value *value, MoonError *error)
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

