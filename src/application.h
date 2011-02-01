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

#include "moonbuild.h"
#include "utils.h"
#include "enums.h"
#include "control.h"
#include "dependencyobject.h"
#include "downloader.h"

/* @CBindingRequisite */
typedef void (*ApplyDefaultStyleCallback)(FrameworkElement *fwe, ManagedTypeInfo *key);
/* @CBindingRequisite */
typedef void (*ApplyStyleCallback)(FrameworkElement *fwe, Style *style);
/* @CBindingRequisite */
typedef void *(*ConvertKeyframeValueCallback)(int kind, DependencyProperty *property, Value *original, Value *converted);
/* @CBindingRequisite */
typedef ManagedStreamCallbacks (*GetResourceCallback)(const char *resourceBase, const char *name);

enum NotifyType {NotifyStarted, NotifySize, NotifyProgressChanged, NotifyCompleted, NotifyFailed};
typedef void (*NotifyFunc) (NotifyType type, gint64 args, gpointer user_data);
typedef void (*WriteFunc) (void* buf, gint32 offset, gint32 n, gpointer user_data);

/* @Namespace=None */
/* @ManagedDependencyProperties=Manual */
/* @ManagedEvents=Manual */
class MOON_API Application : public DependencyObject {
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
	
	void GetResource (const char *resourceBase, const Uri *uri, NotifyFunc notify_cb, WriteFunc write_cb, DownloaderAccessPolicy policy, Cancellable *cancellable, gpointer user_data);
	char *GetResourceAsPath (const char *resourceBase, const Uri *uri);
	const char *GetResourceRoot ();
	
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
