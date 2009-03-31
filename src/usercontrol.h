/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * usercontrol.h:
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_USERCONTROL_H__
#define __MOON_USERCONTROL_H__

#include <glib.h>

#include "control.h"

//
// UserControl
//
/* @ContentProperty="Content" */
/* @Namespace=System.Windows.Controls */
class UserControl : public Control {
protected:
	virtual ~UserControl ();
	
public:
	/* @PropertyType=UIElement,ManagedAccess=ProtectedInternal */
	const static int ContentProperty;
	
 	/* @GenerateCBinding,GeneratePInvoke */
	UserControl ();
	
	virtual bool IsLayoutContainer () { return true; }
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);

	virtual Size MeasureOverride (Size availableSize);
	virtual Size ArrangeOverride (Size finalSize);
};

G_BEGIN_DECLS

UIElement *user_control_get_content (UserControl *user_control);

G_END_DECLS

#endif /* __MOON_USERCONTROL_H__ */
