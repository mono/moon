/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * stackpanel.h
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */
 
#ifndef __MOON_STACKPANEL_H__
#define __MOON_STACKPANEL_H__

 
/* @Namespace=System.Windows.Controls */
/* @Version=2 */
class StackPanel : public Panel {
protected:
	virtual ~StackPanel () {}

public:
	/* @PropertyType=Orientation */
	static DependencyProperty *OrientationProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	StackPanel () {}
	virtual Type::Kind GetObjectType () { return Type::STACKPANEL; }
};

#endif // __MOON_STACKPANEL_H__