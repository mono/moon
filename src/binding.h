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

typedef Value (*GetValueCallback)();
typedef void  (*SetValueCallback)(void *value);

/* @IncludeInKinds */
/* @SilverlightVersion="2" */
/* @Namespace=System.Windows.Data */
class Binding : public EventObject {
	bool notify_on_validation_error;
	bool validates_on_exceptions;
	BindingMode binding_mode;
	char *property_path;
	bool is_sealed;
	
 protected:
	virtual ~Binding ();
	
 public:
	/* @GenerateCBinding,GeneratePInvoke */
	Binding ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	void SetPropertyPath (const char *path);
	/* @GenerateCBinding,GeneratePInvoke */
	char *GetPropertyPath ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	void SetBindingMode (BindingMode mode);
	/* @GenerateCBinding,GeneratePInvoke */
	BindingMode GetBindingMode ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	void SetNotifyOnValidationError (bool value) { notify_on_validation_error = value; }
	/* @GenerateCBinding,GeneratePInvoke */
	bool GetNotifyOnValidationError () { return notify_on_validation_error; }
	
	/* @GenerateCBinding,GeneratePInvoke */
	void SetValidatesOnExceptions (bool value) { validates_on_exceptions = value; }
	/* @GenerateCBinding,GeneratePInvoke */
	bool GetValidatesOnExceptions () { return validates_on_exceptions; }
	
	/* @GenerateCBinding,GeneratePInvoke */
	void SetIsSealed (bool isSealed) { is_sealed = isSealed; }
	/* @GenerateCBinding,GeneratePInvoke */
	bool GetIsSealed () { return is_sealed; }
};


/* @SilverlightVersion="2" */
/* @Namespace=System.Windows.Data */
class BindingExpressionBase : public Expression {
 protected:
	DependencyProperty *target_property;
	FrameworkElement *target;
	DependencyObject *source;
	Binding *binding;
	Value *converter;
	char *culture;
	Value *param;
	
	virtual ~BindingExpressionBase ();
	BindingExpressionBase ();
	
 public:
	virtual Type::Kind GetObjectType () { return Type::BINDINGEXPRESSIONBASE; }
	
	/* @GenerateCBinding,GeneratePInvoke */
	Binding *GetBinding () { return binding; }
	/* @GenerateCBinding,GeneratePInvoke */
	void SetBinding (Binding *binding);
	
	/* @GenerateCBinding,GeneratePInvoke */
	const Value *GetConverterParameter () { return param; }
	/* @GenerateCBinding,GeneratePInvoke */
	void SetConverterParameter (Value *param);
	
	/* @GenerateCBinding,GeneratePInvoke */
	const char *GetConverterCulture () { return culture; }
	/* @GenerateCBinding,GeneratePInvoke */
	void SetConverterCulture (const char *culture);
	
	/* @GenerateCBinding,GeneratePInvoke */
	const Value *GetConverter () { return converter; }
	/* @GenerateCBinding,GeneratePInvoke */
	void SetConverter (Value *converter);
	
	/* @GenerateCBinding,GeneratePInvoke */
	DependencyObject *GetSource () { return source; }
	/* @GenerateCBinding,GeneratePInvoke */
	void SetSource (DependencyObject *source);
	
	/* @GenerateCBinding,GeneratePInvoke */
	FrameworkElement *GetTarget () { return target; }
	/* @GenerateCBinding,GeneratePInvoke */
	void SetTarget (FrameworkElement *target);
	
	/* @GenerateCBinding,GeneratePInvoke */
	DependencyProperty *GetTargetProperty () { return target_property; }
	/* @GenerateCBinding,GeneratePInvoke */
	void SetTargetProperty (DependencyProperty *property) { target_property = property; }
	
	/* @GenerateCBinding,GeneratePInvoke */
	Value *GetValue ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	void UpdateSource (Value *value);
	
	// The following methods are meant only for use by FrameworkElement internals.
	void AttachListener (DependencyObject *target, PropertyChangeHandler handler, gpointer user_data);
	void DetachListener (PropertyChangeHandler handler);

	/* @GenerateCBinding,GeneratePInvoke */
	void RegisterManagedOverrides (GetValueCallback gv_callback, SetValueCallback sv_callback);
	
	void SetGotValue (bool value) { this->got_value = value; }
	
private:
 	bool got_value;
 	GetValueCallback gv_callback;
 	SetValueCallback sv_callback;
 	Value *stored_value;
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

