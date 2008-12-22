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
	static bool default_validator (Value *value, MoonError *error);
	static bool BorderThicknessValidator (Value *value, MoonError *error);
	static bool CornerRadiusValidator (Value *value, MoonError *error);
	static bool PositiveIntValidator (Value *value, MoonError *error);
	static bool IntGreaterThanZeroValidator (Value *value, MoonError *error);
	static bool NonNullStringValidator (Value *value, MoonError *error);
	static bool MediaAttributeCollectionValidator (Value *value, MoonError *error);
	static bool TemplateValidator (Value *value, MoonError *error);
};

#endif /* __VALIDATORS_H__ */
