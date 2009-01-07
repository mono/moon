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

#ifndef __VALIDATORS_H__
#define __VALIDATORS_H__

#include "value.h"
#include "error.h"
#include "thickness.h"
#include "cornerradius.h"

typedef	bool ValueValidator (DependencyObject *instance, DependencyProperty *property, Value *value, MoonError *error);

class Validators
{
public:
	static ValueValidator default_validator;
	static ValueValidator BorderThicknessValidator;
	static ValueValidator CornerRadiusValidator;
	static ValueValidator CursorValidator;
	static ValueValidator PositiveIntValidator;
	static ValueValidator IntGreaterThanZeroValidator;
	static ValueValidator NonNullStringValidator;
	static ValueValidator MediaAttributeCollectionValidator;
	static ValueValidator PasswordValidator;
	static ValueValidator StyleValidator;
	static ValueValidator TemplateValidator;
};

#endif /* __VALIDATORS_H__ */
