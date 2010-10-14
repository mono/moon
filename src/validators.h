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

namespace Moonlight {

typedef	bool ValueValidator (DependencyObject *instance, DependencyProperty *property, Value *value, MoonError *error);

class Validators
{
public:
	static ValueValidator default_validator;
	static ValueValidator AudioStreamIndexValidator;
	static ValueValidator BorderThicknessValidator;
	static ValueValidator BufferingTimeValidator;
	static ValueValidator CornerRadiusValidator;
	static ValueValidator CursorValidator;
	static ValueValidator FloatValidator;
	static ValueValidator PositiveIntValidator;
	static ValueValidator IntGreaterThanZeroValidator;
	static ValueValidator IsInputMethodEnabledValidator;
	static ValueValidator DoubleGreaterThanZeroValidator;
	static ValueValidator DoubleNotNegativeValidator;
	static ValueValidator NonNullValidator;
	static ValueValidator NonNullExceptionValidator;
	static ValueValidator NotNullOrEmptyValidator;
	static ValueValidator MediaAttributeCollectionValidator;
	static ValueValidator TemplateValidator;
	static ValueValidator IsTimelineValidator;
	static ValueValidator StoryboardTargetPropertyValidator;
	static ValueValidator IsSetterSealedValidator;
	static ValueValidator ContentControlContentValidator;
	static ValueValidator LanguageValidator;
	static ValueValidator NameValidator;
	static ValueValidator VolumeValidator;
	static ValueValidator BalanceValidator;
	static ValueValidator BasedOnValidator;
	static ValueValidator StyleValidator;
	static ValueValidator OnlyDuringInitializationValidator;
	static ValueValidator NonNullOnlyDuringInitializationValidator;
	static ValueValidator NullOrInDesignMode;
	static ValueValidator UserInitiatedValidator;
};

};
#endif /* __VALIDATORS_H__ */
