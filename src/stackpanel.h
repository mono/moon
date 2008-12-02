/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * stackpanel.h
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */
 
#ifndef __STACKPANEL_H__
#define __STACKPANEL_H__

#include <panel.h>

/* @Namespace=System.Windows.Controls */
/* @Version=2 */
class StackPanel : public Panel {
 protected:
	virtual ~StackPanel () {}

 public:
	/* @PropertyType=Orientation,GenerateAccessors */
	static DependencyProperty *OrientationProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	StackPanel () {}
	virtual Type::Kind GetObjectType () { return Type::STACKPANEL; }

	// property accessors
	Orientation GetOrientation ();
	void SetOrientation (Orientation value);

	virtual void GetTransformFor (UIElement *item, cairo_matrix_t *result);

	virtual Point GetTransformOrigin ();
	virtual void OnLoaded ();
	virtual void ElementAdded (UIElement *item);
	virtual void ElementRemoved (UIElement *item);
};

#endif // __STACKPANEL_H__
