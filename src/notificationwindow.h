/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * notificationwindow.h
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __MOON_NOTIFICATION_WINDOW_H__
#define __MOON_NOTIFICATION_WINDOW_H__
 
#include "dependencyobject.h"

/* @Namespace=System.Windows */ 
class Window : public DependencyObject {
protected:
	virtual ~Window () {}

public:
	/* @GenerateCBinding,GeneratePInvoke */
	Window ();
	
	/* @PropertyType=double,ManagedFieldAccess=Internal */
	const static int HeightProperty;
	/* @PropertyType=double,ManagedFieldAccess=Internal */
	const static int WidthProperty;
	/* @PropertyType=double,ManagedFieldAccess=Internal */
	const static int LeftProperty;
	/* @PropertyType=double,ManagedFieldAccess=Internal */
	const static int TopProperty;
	/* @PropertyType=bool,ManagedFieldAccess=Internal,ManagedSetterAccess=Internal */
	const static int IsActiveProperty;
	/* @PropertyType=bool,ManagedFieldAccess=Internal */
	const static int TopMostProperty;
	/* @PropertyType=WindowState,ManagedFieldAccess=Internal */
	const static int WindowStateProperty;
};

/* @Namespace=System.Windows */
/* @CallInitialize */
class NotificationWindow : public DependencyObject {
protected:
	virtual ~NotificationWindow () {}

public:
	/* @GeneratePInvoke,GenerateCBinding */
	NotificationWindow ();

	/* @PropertyType=FrameworkElement */
	const static int ContentProperty;
	/* @PropertyType=double */
	const static int HeightProperty;
	/* @PropertyType=Visibility,DefaultValue=VisibilityVisible,GenerateAccessors,ManagedSetterAccess=Private,ManagedFieldAccess=Internal*/
	const static int VisibilityProperty;
	/* @PropertyType=double */
	const static int WidthProperty;

	/* @DelegateType=EventHandler */
	const static int ClosedEvent;

	void SetVisibility (Visibility value);
	Visibility GetVisibility ();
};

#endif /* __MOON_NOTIFICATION_WINDOW_H__ */
