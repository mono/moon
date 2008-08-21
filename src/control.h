/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * control.h:
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __CONTROL_H__
#define __CONTROL_H__

#include <glib.h>

#include "frameworkelement.h"
#include "thickness.h"
#include "brush.h"
#include "enums.h"
#include "xaml.h"

#define CONTROL_FONT_FAMILY  "Portable User Interface"
#define CONTROL_FONT_STRETCH FontStretchesNormal
#define CONTROL_FONT_WEIGHT  FontWeightsNormal
#define CONTROL_FONT_STYLE   FontStylesNormal
#define CONTROL_FONT_SIZE    14.666666984558105

//
// Control Class
//
/* @SilverlightVersion="2" */
/* @Namespace=System.Windows.Controls */
class Control : public FrameworkElement {
 protected:
	virtual ~Control ();
	
 public:
 	/* @PropertyType=Brush,GenerateAccessors */
	static DependencyProperty *BackgroundProperty;
 	/* @PropertyType=Brush,GenerateAccessors */
	static DependencyProperty *BorderBrushProperty;
 	/* @PropertyType=Thickness,DefaultValue=Thickness(0.0) */
	static DependencyProperty *BorderThicknessProperty;
 	/* @PropertyType=string,DefaultValue=CONTROL_FONT_FAMILY,ManagedPropertyType=FontFamily,GenerateAccessors */
	static DependencyProperty *FontFamilyProperty;
 	/* @PropertyType=double,DefaultValue=CONTROL_FONT_SIZE,GenerateAccessors */
	static DependencyProperty *FontSizeProperty;
 	/* @PropertyType=FontStretch,DefaultValue=CONTROL_FONT_STRETCH,GenerateAccessors */
	static DependencyProperty *FontStretchProperty;
 	/* @PropertyType=FontStyle,DefaultValue=CONTROL_FONT_STYLE,GenerateAccessors */
	static DependencyProperty *FontStyleProperty;
 	/* @PropertyType=FontWeight,DefaultValue=CONTROL_FONT_WEIGHT,GenerateAccessors */
	static DependencyProperty *FontWeightProperty;
 	/* @PropertyType=Brush,GenerateAccessors */
	static DependencyProperty *ForegroundProperty;
 	/* @PropertyType=HorizontalAlignment,DefaultValue=HorizontalAlignmentCenter,GenerateAccessors */
	static DependencyProperty *HorizontalContentAlignmentProperty;
 	/* @PropertyType=bool,DefaultValue=true,GenerateAccessors */
	static DependencyProperty *IsTabStopProperty;
 	/* @PropertyType=Thickness,DefaultValue=Thickness(0.0) */
	static DependencyProperty *PaddingProperty;
 	/* @PropertyType=gint32,DefaultValue=INT_MAX,GenerateAccessors */
	static DependencyProperty *TabIndexProperty;
 	/* @PropertyType=KeyboardNavigationMode,DefaultValue=KeyboardNavigationModeLocal,GenerateAccessors */
	static DependencyProperty *TabNavigationProperty;
 	/* @PropertyType=ControlTemplate */
	static DependencyProperty *TemplateProperty;
 	/* @PropertyType=VerticalAlignment,DefaultValue=VerticalAlignmentCenter,GenerateAccessors */
	static DependencyProperty *VerticalContentAlignmentProperty;
 	/* @PropertyType=Style,ManagedFieldAccess=Internal */
	static DependencyProperty *StyleProperty;
	
	FrameworkElement *real_object;
	Rect bounds_with_children;
	
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Protected */
	Control ();
	
	virtual Type::Kind GetObjectType () { return Type::CONTROL; }
	
	virtual void Render (cairo_t *cr, Region *region);
	virtual void FrontToBack (Region *surface_region, List *render_list);
	virtual void ComputeBounds ();
	virtual Rect GetSubtreeBounds () { return bounds_with_children; }
	virtual void GetTransformFor (UIElement *item, cairo_matrix_t *result);
	
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
	
	virtual bool InsideObject (cairo_t *cr, double x, double y);
	
	virtual void HitTest (cairo_t *cr, Point p, List *uielement_list);
	virtual void HitTest (cairo_t *cr, Rect r, List *uielement_list);
	
	virtual bool GetRenderVisible () { return real_object && real_object->GetRenderVisible(); }
	virtual bool GetHitTestVisible () { return real_object && real_object->GetHitTestVisible(); }

	void SetContent (UIElement *element, Surface *surface);

	/* @GenerateCBinding,GeneratePInvoke */
	UIElement *InitializeFromXaml (const char *xaml, Type::Kind *element_type, XamlLoader *loader);
	
	//
	// Property Accessors
	//
	void SetBackground (Brush *bg);
	Brush *GetBackground ();
	
	void SetBorderBrush (Brush *brush);
	Brush *GetBorderBrush ();
	
	void SetBorderThickness (Thickness *thickness);
	Thickness *GetBorderThickness ();
	
	void SetFontFamily (const char *family);
	const char *GetFontFamily ();
	
	void SetFontSize (double size);
	double GetFontSize ();
	
	void SetFontStretch (FontStretches stretch);
	FontStretches GetFontStretch ();
	
	void SetFontStyle (FontStyles style);
	FontStyles GetFontStyle ();
	
	void SetFontWeight (FontWeights weight);
	FontWeights GetFontWeight ();
	
	void SetForeground (Brush *fg);
	Brush *GetForeground ();
	
	void SetHorizontalContentAlignment (HorizontalAlignment alignment);
	HorizontalAlignment GetHorizontalContentAlignment ();
	
	void SetIsTabStop (bool value);
	bool GetIsTabStop ();
	
	void SetPadding (Thickness *padding);
	Thickness *GetPadding ();
	
	void SetTabIndex (int index);
	int GetTabIndex ();
	
	void SetTabNavigation (KeyboardNavigationMode mode);
	KeyboardNavigationMode GetTabNavigation ();
	
	void SetVerticalContentAlignment (VerticalAlignment alignment);
	VerticalAlignment GetVerticalContentAlignment ();
};


#endif /* __CONTROL_H__ */
