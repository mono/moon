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

class Validators
{
public:
	static bool default_validator (DependencyObject* instance, Value *value, MoonError *error);
	static bool BorderThicknessValidator (DependencyObject* instance, Value *value, MoonError *error);
	static bool CornerRadiusValidator (DependencyObject* instance, Value *value, MoonError *error);
	static bool PositiveIntValidator (DependencyObject* instance, Value *value, MoonError *error);
	static bool IntGreaterThanZeroValidator (DependencyObject* instance, Value *value, MoonError *error);
	static bool NonNullStringValidator (DependencyObject* instance, Value *value, MoonError *error);
	static bool MediaAttributeCollectionValidator (DependencyObject* instance, Value *value, MoonError *error);
	static bool TemplateValidator (DependencyObject* instance, Value *value, MoonError *error);
};

#endif /* __VALIDATORS_H__ */
