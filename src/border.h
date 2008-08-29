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
/* @SilverlightVersion="2" */
/* @ContentProperty="Child" */
/* @Namespace=System.Windows.Controls */
class Border : public FrameworkElement {
protected:
	virtual ~Border () { };
	
public:
	/* @PropertyType=Brush,GenerateAccessors */
	static DependencyProperty *BackgroundProperty;
	/* @PropertyType=Brush,GenerateAccessors */
	static DependencyProperty *BorderBrushProperty;
	/* @PropertyType=Thickness,DefaultValue=Thickness(0),GenerateAccessors */
	static DependencyProperty *BorderThicknessProperty;
	/* @PropertyType=UIElement,GenerateAccessors */
	static DependencyProperty *ChildProperty;
	/* @PropertyType=CornerRadius,GenerateAccessors */
	static DependencyProperty *CornerRadiusProperty;
	/* @PropertyType=Thickness,DefaultValue=Thickness(0),GenerateAccessors */
	static DependencyProperty *PaddingProperty;
	
 	/* @GenerateCBinding,GeneratePInvoke */
	Border ();
	
	virtual Type::Kind GetObjectType () { return Type::BORDER; }

	virtual Size MeasureOverride (Size availableSize);
	virtual Size ArrangeOverride (Size finalSize);

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
