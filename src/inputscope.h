// /* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
// /*
//  * inputscope.h
//  *
//  * Contact:
//  *   Moonlight List (moonlight-list@lists.ximian.com)
//  *
//  * Copyright 2010 Novell, Inc. (http://www.novell.com)
//  *
//  * See the LICENSE file included with the distribution for details.
//  */
// 

#ifndef __MOON_INPUTSCOPE_H__
#define __MOON_INPUTSCOPE_H__

#include "dependencyobject.h"

namespace Moonlight {

/* @Namespace=System.Windows.Input */
/* @CallInitialize */
class InputScope : public DependencyObject {
 public:
	/* @GeneratePInvoke */
	InputScope () { SetObjectType (Type::INPUTSCOPE); }

 protected:
	virtual ~InputScope () {}
};

/* @Namespace=System.Windows.Input */
/* @ContentProperty=NameValue */
/* @CallInitialize */
class InputScopeName : public DependencyObject {
 public:
	/* @GeneratePInvoke */
	InputScopeName () : DependencyObject (Type::INPUTSCOPENAME) {}

 protected:
	virtual ~InputScopeName () {}
};

};
#endif /* __MOON_INPUTSCOPE_H__ */
