/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * binding.h: data binding
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __BINDING_H__
#define __BINDING_H__

#include "frameworkelement.h"
#include "expression.h"
#include "enums.h"

struct Binding {
	char *property_path;
	BindingMode mode;
	bool in_use;
};


class BindingExpressionBase : public Expression {
 protected:
	DependencyProperty *property;
	FrameworkElement *element;
	Binding *binding;
	
	virtual ~BindingExpressionBase ();
	BindingExpressionBase ();
	
 public:
	virtual Type::Kind GetObjectType () { return Type::BINDINGEXPRESSIONBASE; }
	
	void SetBinding (Binding *binding);
	Binding *GetBinding () { return binding; }
	
	void SetElement (FrameworkElement *element);
	FrameworkElement *GetElement () { return element; }
	
	void SetProperty (DependencyProperty *property);
	DependencyProperty *GetProperty () { return property; }
};


class BindingExpression : public BindingExpressionBase {
 protected:
	virtual ~BindingExpression () { }
	
 public:
	BindingExpression () { }
	
	virtual Type::Kind GetObjectType () { return Type::BINDINGEXPRESSION; }
};

#endif /* __BINDING_H__ */

