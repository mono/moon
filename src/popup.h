/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * popup.h: 
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __POPUP_H__
#define __POPUP_H__

#include <glib.h>

#include "provider.h"
#include "dependencyproperty.h"
#include "dependencyobject.h"
#include "value.h"

/*@Namespace=System.Windows.Controls.Primitives*/
/* @IncludeInKinds */
class Popup : public DependencyObject {
 public:
	/* @PropertyType=UIElement,GenerateAccessors */
	static DependencyProperty *ChildProperty;
	/* @PropertyType=double,GenerateAccessors */
	static DependencyProperty *HorizontalOffsetProperty;
	/* @PropertyType=bool,GenerateAccessors */
	static DependencyProperty *IsOpenProperty;
	/* @PropertyType=double,GenerateAccessors */
	static DependencyProperty *VerticalOffsetProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	Popup ();

	//
	// Property Accessors
	//
	void SetChild (UIElement *element);
	UIElement *GetChild ();
	
	void SetHorizontalOffset (double offset);
	double GetHorizontalOffset ();
	
	void SetIsOpen (bool open);
	bool GetIsOpen ();
	
	void SetVerticalOffset (double offset);
	double GetVerticalOffset ();

	/* @GenerateCBinding,GeneratePInvoke */
	void SetActiveSurface (Surface *surface);

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);

	const static int IsOpenChangedEvent;
 private:
	Surface *surface;
};


#endif /* __POPUP_H__ */
