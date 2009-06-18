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
#include "template.h"
#include "fontfamily.h"

#define CONTROL_FONT_FAMILY  "Portable User Interface"
#define CONTROL_FONT_STRETCH FontStretchesNormal
#define CONTROL_FONT_WEIGHT  FontWeightsNormal
#define CONTROL_FONT_STYLE   FontStylesNormal

//
// Control Class
//
/* @Namespace=System.Windows.Controls */
/* @CallInitialize */
class Control : public FrameworkElement {
public:
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Protected */
	Control ();
	virtual void Dispose ();

	virtual bool CanCaptureMouse () { return GetIsEnabled (); }
	virtual bool CanFindElement () { return GetIsEnabled (); }
	virtual void FindElementsInHostCoordinates (cairo_t *cr, Point p, List *uielement_list);
	virtual void HitTest (cairo_t *cr, Point p, List *uielement_list);
	
	virtual bool IsLayoutContainer () { return true; }

	virtual bool InsideObject (cairo_t *cr, double x, double y);
	
	virtual void OnLoaded ();
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);
	virtual bool SetValueWithErrorImpl (DependencyProperty *property, Value *value, MoonError *error);
	
	virtual void ElementAdded (UIElement *item);
	virtual void ElementRemoved (UIElement *item);
	
	virtual void OnApplyTemplate ();
	virtual void SetVisualParent (UIElement *visual_parent);

	virtual bool Focus (bool recurse = true);
	
	/* @GenerateCBinding,GeneratePInvoke */
	bool ApplyTemplate ();
	void ClearTemplate ();

	/* @GenerateCBinding,GeneratePInvoke */
	DependencyObject *GetTemplateChild (const char *name);
	
	const static int TemplateAppliedEvent;

	//
	// Property Accessors
	//
	void SetBackground (Brush *bg);
	Brush *GetBackground ();
	
	void SetBorderBrush (Brush *brush);
	Brush *GetBorderBrush ();
	
	void SetBorderThickness (Thickness *thickness);
	Thickness *GetBorderThickness ();
	
	void SetFontFamily (FontFamily *family);
	FontFamily *GetFontFamily ();
	
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
	
	void SetIsEnabled (bool value);
	bool GetIsEnabled ();
	
	void SetIsTabStop (bool value);
	bool GetIsTabStop ();
	
	void SetPadding (Thickness *padding);
	Thickness *GetPadding ();
	
	void SetTabIndex (int index);
	int GetTabIndex ();
	
	void SetTabNavigation (KeyboardNavigationMode mode);
	KeyboardNavigationMode GetTabNavigation ();

	void SetTemplate (ControlTemplate *value);
	ControlTemplate* GetTemplate ();
	
	void SetVerticalContentAlignment (VerticalAlignment alignment);
	VerticalAlignment GetVerticalContentAlignment ();

	void SetDefaultStyleKey (ManagedTypeInfo *value);
	ManagedTypeInfo* GetDefaultStyleKey ();

	// Events
	const static int IsEnabledChangedEvent;
	
	/* @PropertyType=Brush,GenerateAccessors */
	const static int BackgroundProperty;
	/* @PropertyType=Brush,GenerateAccessors */
	const static int BorderBrushProperty;
	/* @PropertyType=Thickness,DefaultValue=Thickness(0.0),GenerateAccessors */
	const static int BorderThicknessProperty;
	/* @PropertyType=FontFamily,DefaultValue=FontFamily(CONTROL_FONT_FAMILY),GenerateAccessors */
	const static int FontFamilyProperty;
	/* @PropertyType=double,AutoCreator=CreateDefaultFontSize,GenerateAccessors */
	const static int FontSizeProperty;
	/* @PropertyType=FontStretch,DefaultValue=CONTROL_FONT_STRETCH,GenerateAccessors */
	const static int FontStretchProperty;
	/* @PropertyType=FontStyle,DefaultValue=CONTROL_FONT_STYLE,GenerateAccessors */
	const static int FontStyleProperty;
	/* @PropertyType=FontWeight,DefaultValue=CONTROL_FONT_WEIGHT,GenerateAccessors */
	const static int FontWeightProperty;
	/* @PropertyType=Brush,DefaultValue=new SolidColorBrush("black"),GenerateAccessors */
	const static int ForegroundProperty;
	/* @PropertyType=HorizontalAlignment,DefaultValue=HorizontalAlignmentCenter,GenerateAccessors */
	const static int HorizontalContentAlignmentProperty;
	/* @PropertyType=bool,DefaultValue=true,GenerateAccessors */
	const static int IsEnabledProperty;
	/* @PropertyType=bool,DefaultValue=true,GenerateAccessors */
	const static int IsTabStopProperty;
	/* @PropertyType=Thickness,DefaultValue=Thickness(0.0),GenerateAccessors */
	const static int PaddingProperty;
	/* @PropertyType=gint32,DefaultValue=INT_MAX,GenerateAccessors */
	const static int TabIndexProperty;
	/* @PropertyType=KeyboardNavigationMode,DefaultValue=KeyboardNavigationModeLocal,GenerateAccessors */
	const static int TabNavigationProperty;
	/* @PropertyType=ControlTemplate,GenerateAccessors,Validator=TemplateValidator */
	const static int TemplateProperty;
	/* @PropertyType=VerticalAlignment,DefaultValue=VerticalAlignmentCenter,GenerateAccessors */
	const static int VerticalContentAlignmentProperty;
	/* @PropertyType=ManagedTypeInfo,ManagedPropertyType=object,GenerateManagedDP=false,GenerateAccessors */
	const static int DefaultStyleKeyProperty;
	
protected:
	virtual ~Control ();
	UIElement *template_root;
	
private:
	ControlTemplate *applied_template;
	bool enabled_local;
	bool enabled_parent;
	void UpdateEnabled ();
};


#endif /* __CONTROL_H__ */
