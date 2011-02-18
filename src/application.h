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

#include "utils.h"
#include "enums.h"
#include "control.h"
#include "dependencyobject.h"
#include "downloader.h"

namespace Moonlight {

/* @CBindingRequisite */
typedef void (*GetDefaultStyleCallback)(FrameworkElement *el, Style ***styles);
/* @CBindingRequisite */
typedef void (*ConvertSetterValuesCallback)(Style *style);
/* @CBindingRequisite */
typedef void *(*ConvertKeyframeValueCallback)(int kind, DependencyProperty *property, Value *original, Value *converted);
/* @CBindingRequisite */
typedef ManagedStreamCallbacks (*GetResourceCallback)(const Uri *resourceBase, const Uri *name);

enum NotifyType {NotifyStarted, NotifyProgressChanged, NotifyCompleted, NotifyFailed};
typedef void (*NotifyFunc) (NotifyType type, gint64 args, gpointer user_data);
typedef void (*WriteFunc) (void* buf, gint32 offset, gint32 n, gpointer user_data);

/* @Namespace=None */
/* @ManagedDependencyProperties=Manual */
/* @ManagedEvents=Manual */
class MOON_API Application : public DependencyObject {
public:
	/* @PropertyType=ResourceDictionary,AutoCreateValue,GenerateAccessors */
	const static int ResourcesProperty;

	/* @GeneratePInvoke */
	void RegisterCallbacks (GetDefaultStyleCallback get_default_style_cb, ConvertSetterValuesCallback convert_setter_values_cb, GetResourceCallback get_resource_cb, ConvertKeyframeValueCallback convert_keyframe_callback);
	
	Style **GetDefaultStyle (FrameworkElement *el);
	void ConvertSetterValues (Style *style);
	
	void ConvertKeyframeValue (Type::Kind kind, DependencyProperty *property, Value *original, Value *converted);
	
	bool GetResource (const Uri *resourceBase, const Uri *uri, NotifyFunc notify_cb, EventHandler write_cb, DownloaderAccessPolicy policy, HttpRequest::Options options, Cancellable *cancellable, gpointer user_data);
	char *GetResourceAsPath (const Uri *resourceBase, const Uri *uri);
	const char *GetResourceRoot ();
	
	/* @GeneratePInvoke */
	void CheckAndDownloadUpdateAsync ();
	
	void UpdateComplete (bool updated, const char *error);
	
	/* @GeneratePInvoke */
	bool IsRunningOutOfBrowser ();
	
	void SetInstallState (InstallState state);
	/* @GeneratePInvoke */
	InstallState GetInstallState ();
	
	bool IsInstallable ();
	
	// The test harness calls the InstallWithError p/invoke through reflection, so if you update the signature here, be sure to update the harness too.
	/* @GeneratePInvoke */
	bool InstallWithError (MoonError *error, bool user_initiated, bool unattended);
	void Uninstall ();
	bool Install ();
	
	/* @GeneratePInvoke */
	static Application *GetCurrent ();
	/* @GeneratePInvoke */
	static void SetCurrent (Application *current);
	
	//
	// Property Accessors
	//
	void SetResources (ResourceDictionary *value);
	ResourceDictionary *GetResources ();
	
	const static int InstallStateChangedEvent;
	const static int CheckAndDownloadUpdateCompletedEvent;
	
protected:
	/* @GeneratePInvoke */
	Application ();
	
	virtual ~Application ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
private:
	GetDefaultStyleCallback get_default_style_cb;
	ConvertSetterValuesCallback convert_setter_values_cb;
	ConvertKeyframeValueCallback convert_keyframe_callback;
	GetResourceCallback get_resource_cb;
	InstallState install_state;
	char *resource_root;
	
	static void update_complete (bool updated, const char *error, gpointer user_data);
};

};

#endif /* __APPLICATION_H__ */
