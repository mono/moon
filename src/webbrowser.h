/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * webbrowser.h
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __MOON_WEBBROWSER_H__
#define __MOON_WEBBROWSER_H__

#include "frameworkelement.h"

/* @Namespace=System.Windows.Controls */
class WebBrowser : public FrameworkElement {
protected:
	virtual ~WebBrowser () {}

public:
	/* @GeneratePInvoke,GenerateCBinding */
	WebBrowser ();

	/* @PropertyType=Uri */
	const static int SourceProperty;

	/* @DelegateType=LoadCompletedEventHandler */
	const static int LoadCompletedEvent;
	/* @DelegateType=EventHandler<NotifyEventArgs> */
	const static int ScriptNotifyEvent;
};

#endif /* __MOON_WEBBROWSER_H__ */

