/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * control.h:
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_CONTROL_H__
#define __MOON_CONTROL_H__

#include <glib.h>

#include "frameworkelement.h"
#include "thickness.h"
#include "brush.h"
#include "enums.h"
#include "xaml.h"

//
// Control Class
//
/* @ContentProperty="Content" */
/* @SilverlightVersion="2" */
class Control : public FrameworkElement {
	bool emitting_loaded;
	
 protected:
	virtual ~Control ();
	
 public:
	static DependencyProperty *BackgroundProperty;
	static DependencyProperty *BorderBrushProperty;
	static DependencyProperty *BorderThicknessProperty;
	static DependencyProperty *FontFamilyProperty;
	static DependencyProperty *FontSizeProperty;
	static DependencyProperty *FontStretchProperty;
	static DependencyProperty *FontStyleProperty;
	static DependencyProperty *FontWeightProperty;
	static DependencyProperty *ForegroundProperty;
	static DependencyProperty *HorizontalContentAlignmentProperty;
	static DependencyProperty *IsTabStopProperty;
	static DependencyProperty *PaddingProperty;
	static DependencyProperty *TabIndexProperty;
	static DependencyProperty *TabNavigationProperty;
	static DependencyProperty *VerticalContentAlignmentProperty;
	
	FrameworkElement *real_object;
	Rect bounds_with_children;
	
	/* @GenerateCBinding */
	Control ();
	
	virtual Type::Kind GetObjectType () { return Type::CONTROL; }
	
	virtual void SetSurface (Surface *s);
	
	virtual void UnregisterAllNamesRootedAt (NameScope *from_ns);
	virtual void RegisterAllNamesRootedAt (NameScope *to_ns);
	
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
	
	virtual void OnLoaded ();
	
	void SetContent (UIElement *element, Surface *surface);
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


G_BEGIN_DECLS

void control_init (void);


Control *control_new (void);
UIElement *control_initialize_from_xaml (Control *control, const char *xaml,
					 Type::Kind *element_type);
UIElement *control_initialize_from_xaml_callbacks (Control *control, const char *xaml, 
						   Type::Kind *element_type, XamlLoader *loader);

void control_set_background (Control *control, Brush *bg);
Brush *control_get_background (Control *control);

void control_set_border_brush (Control *control, Brush *brush);
Brush *control_get_border_brush (Control *control);

void control_set_border_thickness (Control *control, Thickness *thickness);
Thickness *control_get_border_thickness (Control *control);

void control_set_font_family (Control *control, const char *family);
const char *control_get_font_family (Control *control);

void control_set_font_size (Control *control, double size);
double control_get_font_size (Control *control);

void control_set_font_stretch (Control *control, FontStretches stretch);
FontStretches control_get_font_stretch (Control *control);

void control_set_font_style (Control *control, FontStyles style);
FontStyles control_get_font_style (Control *control);

void control_set_font_weight (Control *control, FontWeights weight);
FontWeights control_get_font_weight (Control *control);

void control_set_foreground (Control *control, Brush *fg);
Brush *control_get_foreground (Control *control);

void control_set_horizontal_content_alignment (Control *control, HorizontalAlignment alignment);
HorizontalAlignment control_get_horizontal_content_alignment (Control *control);

void control_set_is_tab_stop (Control *control, bool value);
bool control_get_is_tab_stop (Control *control);

void control_set_padding (Control *control, Thickness *padding);
Thickness *control_get_padding (Control *control);

void control_set_tab_index (Control *control, int index);
int control_get_tab_index (Control *control);

void control_set_tab_navigation (Control *control, KeyboardNavigationMode mode);
KeyboardNavigationMode control_get_tab_navigation (Control *control);

void control_set_vertical_content_alignment (Control *control, VerticalAlignment alignment);
VerticalAlignment control_get_vertical_content_alignment (Control *control);

G_END_DECLS

#endif /* __MOON_CONTROL_H__ */
