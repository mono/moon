/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * border.h:
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_BORDER_H__
#define __MOON_BORDER_H__

#include <glib.h>

#include "frameworkelement.h"
#include "cornerradius.h"

//
// Border
//
/* @ContentProperty="Child" */
/* @Namespace=System.Windows.Controls */
class Border : public FrameworkElement {
protected:
	virtual ~Border () { };
	
public:
	/* @PropertyType=Brush,GenerateAccessors */
	const static int BackgroundProperty;
	/* @PropertyType=Brush,GenerateAccessors */
	const static int BorderBrushProperty;
	/* @PropertyType=Thickness,DefaultValue=Thickness(0),GenerateAccessors,Validator=BorderThicknessValidator */
	const static int BorderThicknessProperty;
	/* @PropertyType=UIElement,GenerateAccessors,ManagedFieldAccess=Internal */
	const static int ChildProperty;
	/* @PropertyType=CornerRadius,GenerateAccessors,DefaultValue=CornerRadius(0),Validator=CornerRadiusValidator */
	const static int CornerRadiusProperty;
	/* @PropertyType=Thickness,DefaultValue=Thickness(0),GenerateAccessors,Validator=BorderThicknessValidator */
	const static int PaddingProperty;
	
 	/* @GenerateCBinding,GeneratePInvoke */
	Border ();
	
	virtual bool IsLayoutContainer () { return true; }
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);

	virtual Size MeasureOverride (Size availableSize);
	virtual Size ArrangeOverride (Size finalSize);

	virtual void Render (cairo_t *cr, Region *region, bool path_only = false);

	virtual bool InsideObject (cairo_t *cr, double x, double y);
	virtual bool CanFindElement () { return GetBackground () || GetBorderBrush (); }
	// property accessors

	Brush *GetBackground ();
	void SetBackground (Brush *value);

	Brush *GetBorderBrush ();
	void SetBorderBrush (Brush *value);

	Thickness *GetBorderThickness ();
	void SetBorderThickness (Thickness *value);

	UIElement *GetChild ();
	void SetChild (UIElement *value);

	CornerRadius *GetCornerRadius ();
	void SetCornerRadius (CornerRadius *value);

	Thickness *GetPadding ();
	void SetPadding (Thickness *value);
	
};

#endif /* __MOON_BORDER_H__ */
