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

#include "panel.h"


/* @ContentProperty="Child" */
/*@Namespace=System.Windows.Controls.Primitives*/
/* @CallInitialize */
class Popup : public FrameworkElement {
 public:
	/* @PropertyType=UIElement,GenerateAccessors,Validator=ContentControlContentValidator */
	const static int ChildProperty;
	/* @PropertyType=double,GenerateAccessors,DefaultValue=0.0 */
	const static int HorizontalOffsetProperty;
	/* @PropertyType=bool,GenerateAccessors,DefaultValue=false */
	const static int IsOpenProperty;
	/* @PropertyType=double,GenerateAccessors,DefaultValue=0.0 */
	const static int VerticalOffsetProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	Popup ();
	virtual void Dispose ();

	//
	// Property Accessors
	//
	void SetChild (UIElement *element);
	UIElement *GetChild ();
	
	void SetHorizontalOffset (double offset);
	double GetHorizontalOffset ();
	
	void SetIsOpen (bool open);
	bool GetIsOpen ();
	
	virtual void SetSurface (Surface *s);
	
	void SetVerticalOffset (double offset);
	double GetVerticalOffset ();

	virtual void HitTest (cairo_t *cr, Point p, List *uielement_list);
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);

	const static int OpenedEvent;
	const static int ClosedEvent;
 private:
	EVENTHANDLER (Popup, ShuttingDown, Deployment, EventArgs);
 	void Hide (UIElement *child);
 	void Show (UIElement *child);
 	void PropagateIsEnabledState (UIElement *child, bool enabled_parent);
 	bool shutting_down;
	bool visible;
};


#endif /* __POPUP_H__ */
