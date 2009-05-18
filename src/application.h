/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * application.h:
 *
 * Copyright 2008 Novell, Inc.  (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <glib.h>

#include "enums.h"
#include "control.h"
#include "dependencyobject.h"

/* @CBindingRequisite */
typedef void (*ApplyDefaultStyleCallback)(FrameworkElement *fwe, ManagedTypeInfo *key);
/* @CBindingRequisite */
typedef void (*ApplyStyleCallback)(FrameworkElement *fwe, Style *style);
/* @CBindingRequisite */
typedef void *(*ConvertKeyframeValueCallback)(int kind, DependencyProperty *property, Value *original, Value *converted);
/* @CBindingRequisite */
typedef void *(*GetResourceCallback)(const char *name, int *size);

/* @ManagedDependencyProperties=Manual */
/* @Namespace=None */
class Application : public DependencyObject {
public:
	/* @PropertyType=ResourceDictionary,AutoCreateValue,GenerateAccessors */
	const static int ResourcesProperty;

	/* @GenerateCBinding,GeneratePInvoke */
	Application ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	void RegisterCallbacks (ApplyDefaultStyleCallback apply_default_style_cb, ApplyStyleCallback apply_style_cb, GetResourceCallback get_resource_cb, ConvertKeyframeValueCallback convert_keyframe_callback);
	
	void ApplyDefaultStyle (FrameworkElement *fwe, ManagedTypeInfo *key);
	void ApplyStyle (FrameworkElement *fwe, Style *style);
	
	void ConvertKeyframeValue (Type::Kind kind, DependencyProperty *property, Value *original, Value *converted);
	
	gpointer GetResource (const Uri *uri, int *size);
	char *GetResourceAsPath (const Uri *uri);
	
	/* @GenerateCBinding,GeneratePInvoke */
	static Application *GetCurrent ();
	/* @GenerateCBinding,GeneratePInvoke */
	static void SetCurrent (Application *current);
	
	//
	// Property Accessors
	//
	void SetResources (ResourceDictionary *value);
	ResourceDictionary *GetResources ();
	
protected:
	virtual ~Application ();

private:
	ApplyDefaultStyleCallback apply_default_style_cb;
	ApplyStyleCallback apply_style_cb;
	ConvertKeyframeValueCallback convert_keyframe_callback;
	GetResourceCallback get_resource_cb;
	char *resource_root;
};

#endif /* __APPLICATION_H__ */
