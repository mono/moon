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

#include "frameworkelement.h"

/*@Namespace=System.Windows.Controls.Primitives*/
class Popup : public FrameworkElement {
 public:
	/* @PropertyType=UIElement,GenerateAccessors */
	static int ChildProperty;
	/* @PropertyType=double,GenerateAccessors */
	static int HorizontalOffsetProperty;
	/* @PropertyType=bool,GenerateAccessors */
	static int IsOpenProperty;
	/* @PropertyType=double,GenerateAccessors */
	static int VerticalOffsetProperty;
	
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
