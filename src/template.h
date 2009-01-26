/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * template.h:
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_TEMPLATE_H__
#define __MOON_TEMPLATE_H__

#include <glib.h>

#include "dependencyobject.h"
#include "control.h"

class TemplateBinding;
class XamlTemplateBinding;

//
// FrameworkTemplate
//
/* @SilverlightVersion="2" */
/* @Namespace=System.Windows */
/* @IncludeInKinds */
class FrameworkTemplate : public DependencyObject {
public:
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Protected */
	FrameworkTemplate ();

	DependencyObject *GetVisualTree (FrameworkElement *templateBindingSource, List *templateBindings);

	void AddXamlBinding (XamlTemplateBinding *binding);

	void SetXamlBuffer (XamlContext *context, const char *buffer);

protected:
	virtual ~FrameworkTemplate ();

	FrameworkElement *templateBindingSource; // only valid when loading the xaml buffer
	List *templateBindings; // only valid when loading the xaml buffer

	char *xaml_buffer;
	XamlContext *xaml_context;
};

//
// ControlTemplate
//
/* @SilverlightVersion="2" */
/* @Namespace=System.Windows.Controls */
/* @IncludeInKinds */
class ControlTemplate : public FrameworkTemplate {
public:
	/* @PropertyType=ManagedTypeInfo,ManagedPropertyType=System.Type,Access=Internal,ManagedAccessorAccess=Public,ManagedFieldAccess=Private */
	static DependencyProperty *TargetTypeProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	ControlTemplate ();

	FrameworkElement * Apply (Control *toControl, List *bindings);

protected:
	virtual ~ControlTemplate () {}

#if false
	DependencyObject* DuplicateObject (Control *source, NameScope *template_namescope, DependencyObject *dob, List* bindings);

	static void duplicate_value (DependencyProperty *key, Value *value, gpointer closure);
#endif
};

//
// DataTemplate
//
/* @SilverlightVersion="2" */
/* @Namespace=System.Windows */
/* @IncludeInKinds */
class DataTemplate : public FrameworkTemplate {
public:
	/* @GenerateCBinding,GeneratePInvoke */
	DataTemplate ();

	/* @GenerateCBinding,GeneratePInvoke */
	DependencyObject* LoadContentWithError (MoonError *error);

protected:
	virtual ~DataTemplate () {}
};

/* @SilverlightVersion="2" */
class XamlTemplateBinding : public EventObject {
public:
	XamlTemplateBinding (DependencyObject *target,
			     const char *targetPropertyName,
			     const char *sourcePropertyName);

	TemplateBinding *Attach (DependencyObject *source);

	DependencyObject* GetTarget() { return target; }

protected:
	virtual ~XamlTemplateBinding ();

private:
	DependencyObject *target;
	char *targetPropertyName;
	char *sourcePropertyName;
};

/* @SilverlightVersion="2" */
class TemplateBinding : public EventObject {
public:
	TemplateBinding (DependencyObject *source,
			 DependencyProperty *sourceProperty,
			 DependencyObject *target,
			 DependencyProperty *targetProperty);
protected:
	virtual ~TemplateBinding ();

private:
	DependencyObject *source;
	DependencyProperty *sourceProperty;

	DependencyObject *target;
	DependencyProperty *targetProperty;

	void OnSourcePropertyChanged (DependencyObject *sender, PropertyChangedEventArgs *args);
	static void SourcePropertyChangedCallback (DependencyObject *sender, PropertyChangedEventArgs *args, gpointer closure);
};

#endif /* __MOON_TEMPLATE_H__ */
