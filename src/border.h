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
	virtual ~Border ();
	
public:
	/* @PropertyType=Brush */
	static DependencyProperty *BackgroundProperty;
	/* @PropertyType=Brush */
	static DependencyProperty *BorderBrushProperty;
	/* @PropertyType=Thickness */
	static DependencyProperty *BorderThicknessProperty;
	/* @PropertyType=UIElement */
	static DependencyProperty *ChildProperty;
	/* @PropertyType=CornerRadius */
	static DependencyProperty *CornerRadiusProperty;
	/* @PropertyType=Thickness */
	static DependencyProperty *PaddingProperty;
	
 	/* @GenerateCBinding,GeneratePInvoke */
	Border ();
	
	virtual Type::Kind GetObjectType () { return Type::BORDER; }
};

#endif /* __MOON_BORDER_H__ */
