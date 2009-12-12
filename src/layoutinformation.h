/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * layoutinformation.h
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_LAYOUTINFORMATION_H__ 
#define __MOON_LAYOUTINFORMATION_H__

/* @IncludeInKinds,Namespace=System.Windows.Controls.Primitives,ManagedDependencyProperties=Manual */
class LayoutInformation {
public:
	/* @PropertyType=Geometry,Attached,GenerateAccessors */
	const static int LayoutClipProperty;
	/* @PropertyType=Rect,Attached,GenerateAccessors */
	const static int LayoutSlotProperty;
	/* @PropertyType=Size,Attached,GenerateAccessors */
	const static int PreviousConstraintProperty;
	/* @PropertyType=Size,Attached,GenerateAccessors */
	const static int FinalRectProperty;
	/* @PropertyType=Size,Attached,GenerateAccessors */
	const static int LastRenderSizeProperty;
	/* @PropertyType=Point,Attached,GenerateAccessors */
	const static int VisualOffsetProperty;

	static void SetLayoutClip (DependencyObject *item, Geometry *clip);
	static Geometry* GetLayoutClip (DependencyObject *item);

	static void SetLayoutSlot (DependencyObject *item, Rect *slot);
	static Rect *GetLayoutSlot (DependencyObject *item);

	static void SetPreviousConstraint (DependencyObject *item, Size *size);
	static Size *GetPreviousConstraint (DependencyObject *item);

	static void SetFinalRect (DependencyObject *item, Size *size);
	static Size *GetFinalRect (DependencyObject *item);

	static void SetLastRenderSize (DependencyObject *item, Size *size);
	static Size *GetLastRenderSize (DependencyObject *item);

	static void SetVisualOffset (DependencyObject *item, Point *offset);
	static Point *GetVisualOffset (DependencyObject *item);

	static void SetBounds (DependencyObject *item, Rect *bounds);
	static Rect *GetBounds (DependencyObject *item);

	static Geometry *GetCompositeClip (FrameworkElement *item);
};
#endif /* __MOON_LAYOUTINFORMATION_H__ */
