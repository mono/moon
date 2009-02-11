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
#define CONTROL_FONT_SIZE    11.0

//
// Control Class
//
/* @SilverlightVersion="2" */
/* @Namespace=System.Windows.Controls */
/* @CallInitialize */
class Control : public FrameworkElement {
public:
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Protected */
	Control ();

	virtual bool IsLayoutContainer () { return true; }

	virtual void HitTest (cairo_t *cr, Rect r, List *uielement_list);
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	
	virtual void ElementAdded (UIElement *item);
	virtual void ElementRemoved (UIElement *item);
	
	virtual void OnLoaded ();

	virtual void OnApplyTemplate ();

	/* @GenerateCBinding,GeneratePInvoke */
	bool ApplyTemplate ();

	/* @GenerateCBinding,GeneratePInvoke */
	DependencyObject *GetTemplateChild (const char *name);
	
	/* @SilverlightVersion="2" */
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

 	/* @PropertyType=Brush,GenerateAccessors */
	static int BackgroundProperty;
 	/* @PropertyType=Brush,GenerateAccessors */
	static int BorderBrushProperty;
 	/* @PropertyType=Thickness,DefaultValue=Thickness(0.0),GenerateAccessors */
	static int BorderThicknessProperty;
 	/* @PropertyType=FontFamily,DefaultValue=FontFamily(CONTROL_FONT_FAMILY),GenerateAccessors */
	static int FontFamilyProperty;
 	/* @PropertyType=double,DefaultValue=CONTROL_FONT_SIZE,GenerateAccessors */
	static int FontSizeProperty;
 	/* @PropertyType=FontStretch,DefaultValue=CONTROL_FONT_STRETCH,GenerateAccessors */
	static int FontStretchProperty;
 	/* @PropertyType=FontStyle,DefaultValue=CONTROL_FONT_STYLE,GenerateAccessors */
	static int FontStyleProperty;
 	/* @PropertyType=FontWeight,DefaultValue=CONTROL_FONT_WEIGHT,GenerateAccessors */
	static int FontWeightProperty;
 	/* @PropertyType=Brush,GenerateAccessors */
	static int ForegroundProperty;
 	/* @PropertyType=HorizontalAlignment,DefaultValue=HorizontalAlignmentCenter,GenerateAccessors */
	static int HorizontalContentAlignmentProperty;
 	/* @PropertyType=bool,DefaultValue=true,GenerateAccessors */
	static int IsTabStopProperty;
 	/* @PropertyType=Thickness,DefaultValue=Thickness(0.0),GenerateAccessors */
	static int PaddingProperty;
 	/* @PropertyType=gint32,DefaultValue=INT_MAX,GenerateAccessors */
	static int TabIndexProperty;
 	/* @PropertyType=KeyboardNavigationMode,DefaultValue=KeyboardNavigationModeLocal,GenerateAccessors */
	static int TabNavigationProperty;
 	/* @PropertyType=ControlTemplate,GenerateAccessors,Validator=TemplateValidator */
	static int TemplateProperty;
 	/* @PropertyType=VerticalAlignment,DefaultValue=VerticalAlignmentCenter,GenerateAccessors */
	static int VerticalContentAlignmentProperty;
	/* @PropertyType=ManagedTypeInfo,ManagedPropertyType=object,GenerateManagedDP=false,GenerateAccessors */
	static int DefaultStyleKeyProperty;
	
protected:
	virtual ~Control ();
	UIElement *template_root;
	
private:
	ControlTemplate *applied_template;
	
	List *bindings;
};


#endif /* __CONTROL_H__ */
