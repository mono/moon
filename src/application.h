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

typedef void (*ApplyDefaultStyleCallback)(FrameworkElement *fwe, ManagedTypeInfo *key);
typedef void (*ApplyStyleCallback)(FrameworkElement *fwe, Style *style);

/* @SilverlightVersion="2" */
/* @ManagedDependencyProperties=Manual */
/* @Namespace=None */
class Application : public DependencyObject {
public:
	/* @PropertyType=ResourceDictionary,GenerateAccessors */
	static DependencyProperty *ResourcesProperty;

	/* @GenerateCBinding,GeneratePInvoke */
	Application ();
	
	virtual Type::Kind GetObjectType () { return Type::APPLICATION; }

	ResourceDictionary* GetResources();
	void SetResources (ResourceDictionary* value);

	/* @GenerateCBinding,GeneratePInvoke */
	static Application* GetCurrent ();
	/* @GenerateCBinding,GeneratePInvoke */
	static void SetCurrent (Application *current);

	/* @GenerateCBinding,GeneratePInvoke */
	void RegisterStyleCallbacks (ApplyDefaultStyleCallback apply_default_style_cb, ApplyStyleCallback apply_style_cb);

	void ApplyDefaultStyle (FrameworkElement *fwe, ManagedTypeInfo *key);
	void ApplyStyle (FrameworkElement *fwe, Style *style);
	
protected:
	virtual ~Application ();

private:
	static Application *_current;
	ApplyDefaultStyleCallback apply_default_style_cb;
	ApplyStyleCallback apply_style_cb;
};

#endif /* __APPLICATION_H__ */
