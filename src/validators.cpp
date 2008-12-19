/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * validators.h: 
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
Validators::default_validator (Value *value, MoonError *error)
{
	return true;	
}

bool
Validators::PositiveIntValidator (Value *value, MoonError *error)
{
	if (value->AsInt32() < 0) {
		MoonError::FillIn (error, MoonError::ARGUMENT, 1001, "Value must be greater than or equal to zero");
		return false;
	}
	return true;
}

bool
Validators::IntGreaterThanZeroValidator (Value *value, MoonError *error)
{
	if (value->AsInt32() < 1) {
		MoonError::FillIn (error, MoonError::ARGUMENT, 1001, "Value must be greater than zero");
		return false;
	}
	return true;
}

bool
Validators::NonNullStringValidator (Value *value, MoonError *error)
{
	if (value->AsString ())
		return true;
	MoonError::FillIn (error, MoonError::ARGUMENT, 1001, "Value cannot be null");
	return false;
}

bool
Validators::MediaAttributeCollectionValidator (Value *value, MoonError *error)
{
	if (value->AsMediaAttributeCollection ())
		return true;
	MoonError::FillIn (error, MoonError::EXCEPTION, 1001, "Value cannot be null");
	return false;
}

bool
Validators::TemplateValidator (Value *value, MoonError *error)
{
	if (value->AsControlTemplate ())
		return true;
	MoonError::FillIn (error, MoonError::EXCEPTION, 1001, "Value cannot be null");
	return false;
}

