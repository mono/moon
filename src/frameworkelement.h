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

/* @Namespace=System.Windows */
class FrameworkElement : public UIElement {
 protected:
	virtual ~FrameworkElement () {}

 public:
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	static DependencyProperty *HeightProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	static DependencyProperty *WidthProperty;

	/* @PropertyType=double,DefaultValue=0.0,Version=2,ManagedSetterAccess=Internal,GenerateGetter */
	static DependencyProperty *ActualHeightProperty;
	/* @PropertyType=double,DefaultValue=0.0,Version=2,ManagedSetterAccess=Internal,GenerateGetter */
	static DependencyProperty *ActualWidthProperty;
	/* @PropertyType=Managed,Version=2.0 */
	static DependencyProperty *DataContextProperty;
	/* @PropertyType=HorizontalAlignment,DefaultValue=HorizontalAlignmentStretch,Version=2.0,GenerateAccessors */
	static DependencyProperty *HorizontalAlignmentProperty;
	/* @PropertyType=string,DefaultValue=\"en-US\",Version=2.0,ManagedPropertyType=XmlLanguage */
	static DependencyProperty *LanguageProperty;
	/* @PropertyType=Thickness,DefaultValue=Thickness (0),Version=2.0,GenerateAccessors */
	static DependencyProperty *MarginProperty;
	/* @PropertyType=double,DefaultValue=INFINITY,Version=2.0,GenerateAccessors */
	static DependencyProperty *MaxHeightProperty;
	/* @PropertyType=double,DefaultValue=INFINITY,Version=2.0,GenerateAccessors */
	static DependencyProperty *MaxWidthProperty;
	/* @PropertyType=double,DefaultValue=0.0,Version=2.0,GenerateAccessors */
	static DependencyProperty *MinHeightProperty;
	/* @PropertyType=double,DefaultValue=0.0,Version=2.0,GenerateAccessors */
	static DependencyProperty *MinWidthProperty;
	/* @PropertyType=VerticalAlignment,DefaultValue=VerticalAlignmentStretch,Version=2.0,GenerateAccessors */
	static DependencyProperty *VerticalAlignmentProperty;
	/* @PropertyType=Style,Version=2.0,GenerateAccessors */
	static DependencyProperty *StyleProperty;
	
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
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

	/* @SilverlightVersion="2" */
	const static int BindingValidationErrorEvent;
	/* @SilverlightVersion="2" */
	const static int LayoutUpdatedEvent;
	/* @SilverlightVersion="2" */
	const static int SizeChangedEvent;
	// XXX 2.0 also has the Loaded event moved here from
	// UIElement.

	// Property Accessors

	double GetActualWidth ();
	double GetActualHeight ();

	Thickness* GetMargin ();
	void SetMargin (Thickness* value);

	double GetMaxHeight ();
	void SetMaxHeight (double value);

	double GetMinHeight ();
	void SetMinHeight (double value);

	double GetMaxWidth ();
	void SetMaxWidth (double value);

	double GetMinWidth ();
	void SetMinWidth (double value);

	HorizontalAlignment GetHorizontalAlignment ();
	void SetHorizontalAlignment (HorizontalAlignment value);

	Style* GetStyle();
	void SetStyle (Style* value);

	VerticalAlignment GetVerticalAlignment ();
	void SetVerticalAlignment (VerticalAlignment value);
};

#endif /* __MOON_FRAMEWORKELEMENT_H__ */
