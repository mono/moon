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


/* @SilverlightVersion="2" */
/* @Namespace=System.Windows.Data */
class BindingExpressionBase : public Expression {
 protected:
	DependencyProperty *property;
	FrameworkElement *element;
	Binding *binding;
	
	virtual ~BindingExpressionBase ();
	BindingExpressionBase ();
	
 public:
	virtual Type::Kind GetObjectType () { return Type::BINDINGEXPRESSIONBASE; }
	
	/* @GenerateCBinding,GeneratePInvoke */
	Binding *GetBinding () { return binding; }
	/* @GenerateCBinding,GeneratePInvoke */
	void SetBinding (Binding *binding);
	
	/* @GenerateCBinding,GeneratePInvoke */
	FrameworkElement *GetElement () { return element; }
	/* @GenerateCBinding,GeneratePInvoke */
	void SetElement (FrameworkElement *element);
	
	/* @GenerateCBinding,GeneratePInvoke */
	DependencyProperty *GetProperty () { return property; }
	/* @GenerateCBinding,GeneratePInvoke */
	void SetProperty (DependencyProperty *property);
	
	/* @GenerateCBinding,GeneratePInvoke */
	void AttachListener (FrameworkElement *listener);
	
	/* @GenerateCBinding,GeneratePInvoke */
	void DetachListener ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	Value *GetValue ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	void UpdateSource (Value *value);
};


/* @SilverlightVersion="2" */
/* @Namespace=System.Windows.Data */
class BindingExpression : public BindingExpressionBase {
 protected:
	virtual ~BindingExpression () { }
	
 public:
	/* @GenerateCBinding,GeneratePInvoke */
	BindingExpression () { }
	
	virtual Type::Kind GetObjectType () { return Type::BINDINGEXPRESSION; }
};

#endif /* __BINDING_H__ */

