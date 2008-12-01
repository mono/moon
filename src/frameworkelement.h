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
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	static DependencyProperty *HeightProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	static DependencyProperty *WidthProperty;

	/* @PropertyType=double,DefaultValue=0.0,Version=2,ManagedSetterAccess=Internal,GenerateAccessors */
	static DependencyProperty *ActualHeightProperty;
	/* @PropertyType=double,DefaultValue=0.0,Version=2,ManagedSetterAccess=Internal,GenerateAccessors */
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
	
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Protected */
	FrameworkElement ();
	
	virtual Type::Kind GetObjectType () { return Type::FRAMEWORKELEMENT; }

	virtual void ComputeBounds ();
	
	//virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
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

	void SetTemplateNameScope (NameScope *namescope);

	virtual Value *GetLocalValue (DependencyProperty *property);
	virtual void ClearValue (DependencyProperty *property, bool notify_listeners = true);

protected:
	NameScope *template_namescope;
	GHashTable *bindings;
	GHashTable *styles;
	
	// Methods for accessing a binding expression on a property
	void SetBindingExpression (DependencyProperty *property, BindingExpressionBase *expr);
	BindingExpressionBase *GetBindingExpression (DependencyProperty *property);
	void ClearBindingExpression (DependencyProperty *property, BindingExpressionBase *expr);
	
	void BoundPropertyChanged (DependencyObject *sender, PropertyChangedEventArgs *args, BindingExpressionBase *expr);
	static void bound_property_changed (DependencyObject *sender, PropertyChangedEventArgs *args, gpointer user_data);
	
	virtual bool IsValueValid (Types *additional_types, DependencyProperty *property, Value *value, MoonError *error);
	virtual bool SetValueWithErrorImpl (DependencyProperty *property, Value *value, MoonError *error);
	
	virtual ~FrameworkElement ();
	
private:
	MeasureOverrideCallback measure_cb;
	ArrangeOverrideCallback arrange_cb;
	void UpdateFromStyle (Style *style);
};

#endif /* __FRAMEWORKELEMENT_H__ */
