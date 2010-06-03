/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * inputmethod.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __INPUTMETHOD_H__
#define __INPUTMETHOD_H__

#include "dependencyobject.h"
#include "inputscope.h"
#include "pal.h"

/* @Namespace=System.Windows.Input */
class InputMethod : public DependencyObject {
 protected:
	virtual ~InputMethod () {}
	
 public:
	/* @PropertyType=bool,Attached,DefaultValue=true,Validator=IsInputMethodEnabledValidator */
	const static int IsInputMethodEnabledProperty;
	/* @PropertyType=ImeConversionModeValues,Attached,GenerateAccessors */
	const static int PreferredImeConversionModeProperty;
	/* @PropertyType=InputMethodState,Attached,GenerateAccessors */
	const static int PreferredImeStateProperty;
	
 	/* @ManagedAccess=Internal,GeneratePInvoke,GenerateCBinding */
	InputMethod () { SetObjectType (Type::INPUTMETHOD); }
	
	static ImeConversionModeValues GetPreferredImeConversionMode (DependencyObject *obj);
	static void SetPreferredImeConversionMode (DependencyObject *obj, ImeConversionModeValues value);
	
	static InputMethodState GetPreferredImeState (DependencyObject *obj);
	static void SetPreferredImeState (DependencyObject *obj, InputMethodState value);
};

#endif /* __INPUTMETHOD_H__ */
