/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * frameworkelement.h
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_FRAMEWORKELEMENT_H__
#define __MOON_FRAMEWORKELEMENT_H__

#include <glib.h>

#include "uielement.h"

class FrameworkElement : public UIElement {
 protected:
	virtual ~FrameworkElement () {}

 public:
	/* @PropertyType=double,DefaultValue=0.0 */
	static DependencyProperty *HeightProperty;
	/* @PropertyType=double,DefaultValue=0.0 */
	static DependencyProperty *WidthProperty;

	// 2.0 only DPs
	/* @PropertyType=double,DefaultValue=0.0,Version=2 */
	static DependencyProperty *ActualHeightProperty;
	/* @PropertyType=double,DefaultValue=0.0,Version=2 */
	static DependencyProperty *ActualWidthProperty;
	/* @PropertyType=Managed,Version=2 */
	static DependencyProperty *DataContextProperty;
	/* @PropertyType=gint32,DefaultValue=HorizontalAlignmentStretch,Version=2 */
	static DependencyProperty *HorizontalAlignmentProperty;
	/* @PropertyType=char*,DefaultValue=\"en-US\",Version=2 */
	static DependencyProperty *LanguageProperty;
	/* @PropertyType=Thickness,DefaultValue=Thickness (0),Version=2 */
	static DependencyProperty *MarginProperty;
	/* @PropertyType=double,DefaultValue=INFINITY,Version=2 */
	static DependencyProperty *MaxHeightProperty;
	/* @PropertyType=double,DefaultValue=INFINITY,Version=2 */
	static DependencyProperty *MaxWidthProperty;
	/* @PropertyType=double,DefaultValue=0.0,Version=2 */
	static DependencyProperty *MinHeightProperty;
	/* @PropertyType=double,DefaultValue=0.0,Version=2 */
	static DependencyProperty *MinWidthProperty;
	/* @PropertyType=gint32,DefaultValue=VerticalAlignmentStretch,Version=2 */
	static DependencyProperty *VerticalAlignmentProperty;

	/* @GenerateCBinding */
	FrameworkElement ();
	virtual Type::Kind GetObjectType () { return Type::FRAMEWORKELEMENT; }

	virtual void ComputeBounds ();

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);

	virtual bool InsideObject (cairo_t *cr, double x, double y);

	virtual void GetSizeForBrush (cairo_t *cr, double *width, double *height);
	
	//
	// Property Accessors
	//
	void SetHeight (double height);
	double GetHeight ();
	
	void SetWidth (double width);
	double GetWidth ();
	
	//
	// 2.0 methods
	//
	
	virtual Size MeasureOverride (Size availableSize)
	{
		return Size (0, 0);
	}
	
	// overrides uielement::MeasureCore
	virtual Size MeasureCore (Size availableSize)
	{
		//
		// We proxy from UIElement.MeasureCore to FrameworkElement.MeasureOverride
		//
		// WPF docs do not shed much light into why there is a difference
		//
		return MeasureOverride (availableSize);
	}
};

G_BEGIN_DECLS

FrameworkElement *framework_element_new (void);

double	framework_element_get_height	(FrameworkElement *framework_element);
void	framework_element_set_height	(FrameworkElement *framework_element, double height);
double	framework_element_get_width	(FrameworkElement *framework_element);
void	framework_element_set_width	(FrameworkElement *framework_element, double width);

void	framework_element_init (void);

G_END_DECLS


#endif /* __MOON_FRAMEWORKELEMENT_H__ */
