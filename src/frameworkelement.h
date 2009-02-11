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
#include <math.h>

#include "uielement.h"

typedef Size (*MeasureOverrideCallback)(Size availableSize);
typedef Size (*ArrangeOverrideCallback)(Size finalSize);

/* @Namespace=System.Windows */
/* @CallInitialize */
class FrameworkElement : public UIElement {
public:
	/* @PropertyType=double,DefaultValue=NAN,GenerateAccessors */
	static int HeightProperty;
	/* @PropertyType=double,DefaultValue=NAN,GenerateAccessors */
	static int WidthProperty;

	/* @PropertyType=double,DefaultValue=0.0,Version=2,ManagedSetterAccess=Internal,GenerateAccessors */
	static int ActualHeightProperty;
	/* @PropertyType=double,DefaultValue=0.0,Version=2,ManagedSetterAccess=Internal,GenerateAccessors */
	static int ActualWidthProperty;
	/* @PropertyType=object,Version=2.0 */
	static int DataContextProperty;
	/* @PropertyType=HorizontalAlignment,DefaultValue=HorizontalAlignmentStretch,Version=2.0,GenerateAccessors */
	static int HorizontalAlignmentProperty;
	/* @PropertyType=string,DefaultValue=\"en-US\",Version=2.0,ManagedPropertyType=XmlLanguage,Validator=NonNullStringValidator */
	static int LanguageProperty;
	/* @PropertyType=Thickness,DefaultValue=Thickness (0),Version=2.0,GenerateAccessors */
	static int MarginProperty;
	/* @PropertyType=double,DefaultValue=INFINITY,Version=2.0,GenerateAccessors */
	static int MaxHeightProperty;
	/* @PropertyType=double,DefaultValue=INFINITY,Version=2.0,GenerateAccessors */
	static int MaxWidthProperty;
	/* @PropertyType=double,DefaultValue=0.0,Version=2.0,GenerateAccessors */
	static int MinHeightProperty;
	/* @PropertyType=double,DefaultValue=0.0,Version=2.0,GenerateAccessors */
	static int MinWidthProperty;
	/* @PropertyType=VerticalAlignment,DefaultValue=VerticalAlignmentStretch,Version=2.0,GenerateAccessors */
	static int VerticalAlignmentProperty;
	/* @PropertyType=Style,Version=2.0,GenerateAccessors,Validator=StyleValidator */
	static int StyleProperty;
	
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Protected */
	FrameworkElement ();
	
	virtual void ComputeBounds ();
	virtual Rect GetSubtreeBounds ();

	virtual void HitTest (cairo_t *cr, Point p, List *uielement_list);
	virtual void FindElementsInHostCoordinates (cairo_t *cr, Point P, List *uielement_list);
	
	//virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);

	virtual bool InsideObject (cairo_t *cr, double x, double y);

	virtual void GetSizeForBrush (cairo_t *cr, double *width, double *height);
	virtual Point GetTransformOrigin ();
	
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
	// Layout
	virtual void Measure (Size availableSize);
	virtual void Arrange (Rect finalRect);

	/* @GeneratePInvoke,GenerateCBinding */
	void RegisterManagedOverrides (MeasureOverrideCallback measure_cb, ArrangeOverrideCallback arrange_cb);

	// These two methods call into managed land using the
	// delegates registered in RegisterManagedOverrides.  If
	// classes want to implement fully unmanaged layout, they
	// should override these two methods.

	/* @GenerateCBinding,GeneratePInvoke */
	virtual Size MeasureOverride (Size availableSize);
	/* @GenerateCBinding,GeneratePInvoke */
	virtual Size ArrangeOverride (Size finalSize);

	virtual bool UpdateLayout ();
	
	/* @SilverlightVersion="2" */
	const static int BindingValidationErrorEvent;
	/* @SilverlightVersion="2" */
	const static int LayoutUpdatedEvent;
	/* @SilverlightVersion="2" */
	const static int SizeChangedEvent;
	// XXX 2.0 also has the Loaded event moved here from
	// UIElement.
	
	//
	// Property Accessors (2.0)
	//

	void SetActualWidth (double width);
	double GetActualWidth ();
	
	void SetActualHeight (double width);
	double GetActualHeight ();
	
	Thickness *GetMargin ();
	void SetMargin (Thickness *value);

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

	Style *GetStyle ();
	void SetStyle (Style *value);

	VerticalAlignment GetVerticalAlignment ();
	void SetVerticalAlignment (VerticalAlignment value);

protected:
	Rect bounds_with_children;
	GHashTable *styles;
	
	// Methods for accessing a binding expression on a property
	void SetBindingExpression (DependencyProperty *property, BindingExpressionBase *expr);
	BindingExpressionBase *GetBindingExpression (DependencyProperty *property);
	void ClearBindingExpression (DependencyProperty *property, BindingExpressionBase *expr);
	
	virtual bool SetValueWithErrorImpl (DependencyProperty *property, Value *value, MoonError *error);

	virtual void ElementAdded (UIElement *item);
	
	virtual ~FrameworkElement ();
	
private:
	MeasureOverrideCallback measure_cb;
	ArrangeOverrideCallback arrange_cb;
};

#endif /* __FRAMEWORKELEMENT_H__ */
