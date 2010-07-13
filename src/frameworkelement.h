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
#include "enums.h"

namespace Moonlight {

/* @CBindingRequisite */
typedef Size (*MeasureOverrideCallback)(Size availableSize, MoonError *error);
/* @CBindingRequisite */
typedef Size (*ArrangeOverrideCallback)(Size finalSize, MoonError *error);
/* @CBindingRequisite */
typedef UIElement *(*GetDefaultTemplateCallback)(FrameworkElement *element);
/* @CBindingRequisite */
typedef void (*LoadedCallback)(FrameworkElement *element);

/* @Namespace=System.Windows */
/* @CallInitialize */
class FrameworkElement : public UIElement {
public:
	/* @PropertyType=double,DefaultValue=NAN,GenerateAccessors */
	const static int HeightProperty;
	/* @PropertyType=double,DefaultValue=NAN,GenerateAccessors */
	const static int WidthProperty;

	/* @PropertyType=double,DefaultValue=0.0,Version=2,ManagedSetterAccess=Internal,GenerateAccessors,ReadOnly */
	const static int ActualHeightProperty;
	/* @PropertyType=double,DefaultValue=0.0,Version=2,ManagedSetterAccess=Internal,GenerateAccessors,ReadOnly */
	const static int ActualWidthProperty;
	/* @PropertyType=object,Version=2.0,IsCustom=true */
	const static int DataContextProperty;
	/* @PropertyType=HorizontalAlignment,DefaultValue=HorizontalAlignmentStretch,Version=2.0,GenerateAccessors */
	const static int HorizontalAlignmentProperty;
	/* @PropertyType=string,DefaultValue=\"en-US\",Version=2.0,ManagedPropertyType=XmlLanguage,Validator=LanguageValidator,GenerateAccessors */
	const static int LanguageProperty;
	/* @PropertyType=Thickness,DefaultValue=Thickness (0),Version=2.0,GenerateAccessors */
	const static int MarginProperty;
	/* @PropertyType=double,DefaultValue=INFINITY,Version=2.0,GenerateAccessors */
	const static int MaxHeightProperty;
	/* @PropertyType=double,DefaultValue=INFINITY,Version=2.0,GenerateAccessors */
	const static int MaxWidthProperty;
	/* @PropertyType=double,DefaultValue=0.0,Version=2.0,GenerateAccessors */
	const static int MinHeightProperty;
	/* @PropertyType=double,DefaultValue=0.0,Version=2.0,GenerateAccessors */
	const static int MinWidthProperty;
	/* @PropertyType=VerticalAlignment,DefaultValue=VerticalAlignmentStretch,Version=2.0,GenerateAccessors */
	const static int VerticalAlignmentProperty;
	/* @PropertyType=Style,Version=2.0,GenerateAccessors,Validator=StyleValidator */
	const static int StyleProperty;
	/* @PropertyType=FlowDirection,DefaultValue=FlowDirectionLeftToRight,GenerateAccessors */
	const static int FlowDirectionProperty;
	
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Protected */
	FrameworkElement ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	bool ApplyTemplateWithError (MoonError *error);
	virtual bool DoApplyTemplateWithError (MoonError *error);
	virtual UIElement *GetDefaultTemplate ();
	virtual void OnApplyTemplate ();
	
	virtual void ElementRemoved (UIElement *obj);
	
	virtual void ComputeBounds ();
	virtual void ComputeGlobalBounds ();
	virtual void ComputeSurfaceBounds ();
	virtual Rect GetLocalBounds ();
	virtual Rect GetGlobalBounds ();
	virtual Rect GetSubtreeBounds ();
	virtual void Dispose ();
	virtual void HitTest (cairo_t *cr, Point p, List *uielement_list);
	virtual void FindElementsInHostCoordinates (cairo_t *cr, Point P, List *uielement_list);
	virtual void FindElementsInHostCoordinates (cairo_t *cr, Rect r, List *uielement_list);
	
	//virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);

	virtual bool InsideObject (cairo_t *cr, double x, double y);
	bool InsideLayoutClip (double x, double y);
	void RenderLayoutClip (cairo_t *cr);

	virtual void GetSizeForBrush (cairo_t *cr, double *width, double *height);
	virtual Point GetTransformOrigin ();

	virtual void SetVisualParent (UIElement *visual_parent);

	/* @GenerateCBinding,GeneratePInvoke */
	void SetLogicalParent (DependencyObject *logical_parent, MoonError *error);
	/* @GenerateCBinding,GeneratePInvoke */
	DependencyObject *GetLogicalParent () { return logical_parent; }

	//
	// Property Accessors
	//
	void SetHeight (double height);
	double GetHeight ();
	
	void SetWidth (double width);
	double GetWidth ();

	void SetLanguage (const char *value);
	const char *GetLanguage ();

	//
	// 2.0 methods
	//
	// Layout
	virtual void MeasureWithError (Size availableSize, MoonError *error);
	virtual void ArrangeWithError (Rect finalRect, MoonError *error);

	/* @GeneratePInvoke,GenerateCBinding */
	void RegisterManagedOverrides (MeasureOverrideCallback measure_cb, ArrangeOverrideCallback arrange_cb,
				       GetDefaultTemplateCallback get_default_template_cb, LoadedCallback loaded_cb);

	// These two methods call into managed land using the
	// delegates registered in RegisterManagedOverrides.  If
	// classes want to implement fully unmanaged layout, they
	// should override these two methods.

	/* @GenerateCBinding,GeneratePInvoke */
	virtual Size MeasureOverrideWithError (Size availableSize, MoonError *error);
	/* @GenerateCBinding,GeneratePInvoke */
	virtual Size ArrangeOverrideWithError (Size finalSize, MoonError *error);
	virtual Size ComputeActualSize ();

	
	// Apply specific constraint values to the given size
	Size ApplySizeConstraints (const Size &size);
	
	virtual void UpdateLayoutWithError (MoonError *error);

	virtual void OnLoaded ();

	/* @DelegateType=SizeChangedEventHandler */
	const static int SizeChangedEvent;
	/* @GenerateManagedEvent=false */
	const static int TemplateAppliedEvent;
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

	FlowDirection GetFlowDirection ();
	void SetFlowDirection (FlowDirection value);

protected:
	GetDefaultTemplateCallback get_default_template_cb;
	Rect bounds_with_children;
	Rect global_bounds_with_children;
	Rect surface_bounds_with_children;
	GHashTable *styles;

	virtual ~FrameworkElement ();
	
private:
	MeasureOverrideCallback measure_cb;
	ArrangeOverrideCallback arrange_cb;
	LoadedCallback loaded_cb;

	DependencyObject  *logical_parent;
	UIElement *default_template;
};

class FrameworkElementProvider : public PropertyValueProvider {
	Value *actual_height_value;
	Value *actual_width_value;
	Size last;

 public:
	FrameworkElementProvider (DependencyObject *obj, PropertyPrecedence precedence);
	virtual ~FrameworkElementProvider ();
	virtual Value *GetPropertyValue (DependencyProperty *property);
};


};
#endif /* __FRAMEWORKELEMENT_H__ */
