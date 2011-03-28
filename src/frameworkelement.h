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
typedef Size (*MeasureOverrideCallback)(FrameworkElement *fwe, Size availableSize, MoonError *error);
/* @CBindingRequisite */
typedef Size (*ArrangeOverrideCallback)(FrameworkElement *fwe, Size finalSize, MoonError *error);
/* @CBindingRequisite */
typedef UIElement *(*GetDefaultTemplateCallback)(FrameworkElement *element);
/* @CBindingRequisite */
typedef void (*LoadedCallback)(FrameworkElement *element);
/* @CBindingRequisite */
typedef void (*StyleResourceChangedCallback)(FrameworkElement *element, const char *key, Style *value);

/* @Namespace=System.Windows */
/* @CallInitialize */
class FrameworkElement : public UIElement {
public:
	/* @PropertyType=double,DefaultValue=NAN,GenerateAccessors,Validator=DoubleNotNegativeValidator */
	const static int HeightProperty;
	/* @PropertyType=double,DefaultValue=NAN,GenerateAccessors,Validator=DoubleNotNegativeValidator */
	const static int WidthProperty;

	/* @PropertyType=double,DefaultValue=0.0,ManagedSetterAccess=Internal,GenerateAccessors,ReadOnly */
	const static int ActualHeightProperty;
	/* @PropertyType=double,DefaultValue=0.0,ManagedSetterAccess=Internal,GenerateAccessors,ReadOnly */
	const static int ActualWidthProperty;
	/* @PropertyType=object,IsCustom=true */
	const static int DataContextProperty;
	/* @PropertyType=HorizontalAlignment,DefaultValue=HorizontalAlignmentStretch,GenerateAccessors */
	const static int HorizontalAlignmentProperty;
	/* @PropertyType=XmlLanguage,DefaultValue=\"en-US\",Validator=LanguageValidator,GenerateAccessors */
	const static int LanguageProperty;
	/* @PropertyType=Thickness,DefaultValue=Thickness (0),GenerateAccessors */
	const static int MarginProperty;
	/* @PropertyType=double,DefaultValue=INFINITY,GenerateAccessors,Validator=DoubleNotNegativeValidator */
	const static int MaxHeightProperty;
	/* @PropertyType=double,DefaultValue=INFINITY,GenerateAccessors,Validator=DoubleNotNegativeValidator */
	const static int MaxWidthProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors,Validator=DoubleNotNegativeValidator */
	const static int MinHeightProperty;
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors,Validator=DoubleNotNegativeValidator */
	const static int MinWidthProperty;
	/* @PropertyType=VerticalAlignment,DefaultValue=VerticalAlignmentStretch,GenerateAccessors */
	const static int VerticalAlignmentProperty;
	/* @PropertyType=Style,GenerateAccessors,Validator=StyleValidator */
	const static int StyleProperty;
	/* @PropertyType=FlowDirection,DefaultValue=FlowDirectionLeftToRight,GenerateAccessors */
	const static int FlowDirectionProperty;
	
	/* @GeneratePInvoke,ManagedAccess=Protected */
	FrameworkElement ();
	
	/* @GeneratePInvoke */
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
	virtual Rect GetSubtreeExtents ();
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

	/* @GeneratePInvoke */
	void SetLogicalParent (DependencyObject *value, MoonError *error);
	/* @GeneratePInvoke */
	DependencyObject *GetLogicalParent () { return logical_parent; }
	virtual void OnLogicalParentChanged (DependencyObject *old_parent, DependencyObject *new_parent);

	/* @GeneratePInvoke */
	void SetImplicitStyles (ImplicitStylePropertyValueProvider::StyleMask style_mask, Style** styles = NULL);

	/* @GeneratePInvoke */
	void ClearImplicitStyles (ImplicitStylePropertyValueProvider::StyleMask style_mask);

	//
	// Property Accessors
	//
	void SetHeight (double height);
	double GetHeight ();
	
	void SetWidth (double width);
	double GetWidth ();

	void SetLanguage (const char *value);
	const char *GetLanguage ();

	void StyleResourceChanged (const char *key, Style *value);

	//
	// 2.0 methods
	//
	// Layout
	virtual void MeasureWithError (Size availableSize, MoonError *error);
	virtual void ArrangeWithError (Rect finalRect, MoonError *error);

	/* @GeneratePInvoke */
	void RegisterManagedOverrides (MeasureOverrideCallback measure_cb, ArrangeOverrideCallback arrange_cb,
				       GetDefaultTemplateCallback get_default_template_cb, LoadedCallback loaded_cb,
				       StyleResourceChangedCallback style_resource_changed_cb);

	// These two methods call into managed land using the
	// delegates registered in RegisterManagedOverrides.  If
	// classes want to implement fully unmanaged layout, they
	// should override these two methods.

	/* @GeneratePInvoke */
	virtual Size MeasureOverrideWithError (Size availableSize, MoonError *error);
	/* @GeneratePInvoke */
	virtual Size ArrangeOverrideWithError (Size finalSize, MoonError *error);
	virtual Size ComputeActualSize ();

	
	// Apply specific constraint values to the given size
	Size ApplySizeConstraints (const Size &size);
	
	virtual void UpdateLayoutWithError (MoonError *error);
	virtual void UpdateLayer (LayoutPass *pass, MoonError *error);

	virtual void OnIsAttachedChanged (bool attached);
	virtual void OnIsLoadedChanged (bool loaded);

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

	const static void *LogicalParentWeakRef;

protected:
	GetDefaultTemplateCallback get_default_template_cb;
	Rect bounds_with_children;
	Rect global_bounds_with_children;
	Rect surface_bounds_with_children;
	Rect extents_with_children;
	GHashTable *styles;

	virtual ~FrameworkElement ();
	/* @SkipFactories */
	FrameworkElement (Type::Kind object_type);
	
private:
	MeasureOverrideCallback measure_cb;
	ArrangeOverrideCallback arrange_cb;
	LoadedCallback loaded_cb;
	StyleResourceChangedCallback style_resource_changed_cb;

	WeakRef<DependencyObject> logical_parent;

	void Init ();
};

class FrameworkElementProvider : public PropertyValueProvider {
	Value *actual_height_value;
	Value *actual_width_value;
	Size last;

 public:
	FrameworkElementProvider (DependencyObject *obj, PropertyPrecedence precedence, int flags = 0);
	virtual ~FrameworkElementProvider ();
	virtual Value *GetPropertyValue (DependencyProperty *property);
};


};
#endif /* __FRAMEWORKELEMENT_H__ */
