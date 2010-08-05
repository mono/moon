/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * canvas.h: canvas definitions.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_CANVAS_H__
#define __MOON_CANVAS_H__

#include <glib.h>

#include "panel.h"

namespace Moonlight {

//
// Canvas Class, the only purpose is to have the Left/Top properties that
// children can use
//
/* @Namespace=System.Windows.Controls */
class Canvas : public Panel {
 private:
	Rect coverage_bounds;

 protected:
	/* @GenerateCBinding,GeneratePInvoke */
	Canvas ();
	
	virtual ~Canvas () {}

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
	
 public:
 	/* @PropertyType=double,DefaultValue=0.0,Attached,GenerateAccessors */
	const static int LeftProperty;
 	/* @PropertyType=double,DefaultValue=0.0,Attached,GenerateAccessors */
	const static int TopProperty;
	/* @PropertyType=gint32,DefaultValue=0,Attached,GenerateAccessors */
	const static int ZIndexProperty;
	/* @PropertyType=double,DefaultValue=NAN,Attached,GenerateAccessors,ManagedFieldAccess=Internal */
	const static int ZProperty;

	
	virtual Size MeasureOverrideWithError (Size availableSize, MoonError *error);
	virtual Size ArrangeOverrideWithError (Size finalSize, MoonError *error);
	virtual bool IsLayoutContainer ();
	virtual void ShiftPosition (Point p);
	virtual void ComputeBounds ();

	virtual Rect GetCoverageBounds ();
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);

	virtual void OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args);
	
	//
	// Property Accessors
	//
	static void SetLeft (DependencyObject *item, double left);
	static double GetLeft (DependencyObject *item);
	
	static void SetTop (DependencyObject *item, double top);
	static double GetTop (DependencyObject *item);
	
	static void SetZIndex (DependencyObject *item, int zindex);
	static int GetZIndex (DependencyObject *item);

	static void SetZ (DependencyObject *item, double z);
	static double GetZ (DependencyObject *item);
};

};
#endif /* __MOON_CANVAS_H__ */
