/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * control.cpp:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>
#include "rect.h"
#include "runtime.h"
#include "control.h"
#include "canvas.h"

#define CONTROL_FONT_FAMILY  "Portable User Interface"
#define CONTROL_FONT_STRETCH FontStretchesNormal
#define CONTROL_FONT_WEIGHT  FontWeightsNormal
#define CONTROL_FONT_STYLE   FontStylesNormal
#define CONTROL_FONT_SIZE    14.666666984558105

DependencyProperty *Control::BackgroundProperty;
DependencyProperty *Control::BorderBrushProperty;
DependencyProperty *Control::BorderThicknessProperty;
DependencyProperty *Control::FontFamilyProperty;
DependencyProperty *Control::FontSizeProperty;
DependencyProperty *Control::FontStretchProperty;
DependencyProperty *Control::FontStyleProperty;
DependencyProperty *Control::FontWeightProperty;
DependencyProperty *Control::ForegroundProperty;
DependencyProperty *Control::HorizontalContentAlignmentProperty;
DependencyProperty *Control::IsTabStopProperty;
DependencyProperty *Control::PaddingProperty;
DependencyProperty *Control::TabIndexProperty;
DependencyProperty *Control::TabNavigationProperty;
DependencyProperty *Control::VerticalContentAlignmentProperty;

Control::Control ()
{
	real_object = NULL;
	emitting_loaded = false;
}

Control::~Control ()
{
	if (real_object)
		real_object->unref ();
}

void 
Control::Render (cairo_t *cr, Region *region)
{
	if (real_object)
		real_object->DoRender (cr, region);
}

void
Control::FrontToBack (Region *surface_region, List *render_list)
{
	if (real_object)
		return real_object->FrontToBack (surface_region, render_list);
}

void 
Control::ComputeBounds ()
{
	if (real_object) {
		bounds = real_object->GetBounds ();
		bounds_with_children = real_object->GetSubtreeBounds ();
	} else {
		bounds = Rect (0, 0, 0, 0);
		bounds_with_children = Rect (0, 0, 0, 0);
	}
	
	double x1, x2, y1, y2;
	
	x1 = y1 = 0.0;
	x2 = GetWidth ();
	y2 = GetHeight ();
	
	if (x2 != 0.0 && y2 != 0.0) {
		Rect fw_rect = Rect (x1,y1,x2,y2);
		
		if (real_object)
			bounds = bounds.Union (fw_rect);
		else
			bounds = fw_rect;
	}
	
	bounds = IntersectBoundsWithClipPath (bounds, false).Transform (&absolute_xform);
	bounds_with_children = IntersectBoundsWithClipPath (bounds_with_children.Union (bounds), false);
}

void
Control::SetSurface (Surface *s)
{
	FrameworkElement::SetSurface (s);
	
	if (real_object)
		real_object->SetSurface (s);
}

void
Control::UnregisterAllNamesRootedAt (NameScope *from_ns)
{
	FrameworkElement::UnregisterAllNamesRootedAt (from_ns);

	if (real_object)
		real_object->UnregisterAllNamesRootedAt (from_ns);
}

void
Control::RegisterAllNamesRootedAt (NameScope *to_ns)
{
	FrameworkElement::RegisterAllNamesRootedAt (to_ns);

	if (real_object)
		real_object->RegisterAllNamesRootedAt (to_ns);
}

void
Control::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	if (subobj_args->property == Canvas::TopProperty || subobj_args->property == Canvas::LeftProperty)
		real_object->UpdateTransform ();
	
	UIElement::OnSubPropertyChanged (prop, obj, subobj_args);
}

void
Control::GetTransformFor (UIElement *item, cairo_matrix_t *result)
{
	double left = Canvas::GetLeft (item);
	double top = Canvas::GetTop (item);
	
	cairo_matrix_init_translate (result, left, top);
}

bool 
Control::InsideObject (cairo_t *cr, double x, double y)
{
	if (real_object)
		return real_object->InsideObject (cr, x, y);
	else
		return false;
}

void
Control::HitTest (cairo_t *cr, Point p, List *uielement_list)
{
	if (InsideObject (cr, p.x, p.y)) {
		uielement_list->Prepend (new UIElementNode (this));
		real_object->HitTest (cr, p, uielement_list);
	}
}

void
Control::HitTest (cairo_t *cr, Rect r, List *uielement_list)
{
}

void
Control::OnLoaded ()
{
	if (emitting_loaded)
		return;

	emitting_loaded = true;

	flags |= UIElement::IS_LOADED;

	if (real_object)
		real_object->OnLoaded ();

	FrameworkElement::OnLoaded ();

	emitting_loaded = false;
}

void
Control::SetContent (UIElement *element, Surface *surface)
{
	if (real_object){
		real_object->SetVisualParent (NULL);
		real_object->SetSurface (NULL);
		real_object->unref ();
	}

	real_object = (FrameworkElement *) element;
	real_object->SetVisualParent (this);

	//
	// It is not clear that we even need to do this, as attaching
	// will do this internally, but am keeping this to not break
	// things.   Also the @surface parameter could go away if we
	// determine its not needed
	//
	if (surface != NULL)
		SetSurface (surface);
	
	real_object->AddPropertyChangeListener (this);
	real_object->UpdateTotalRenderVisibility ();
	real_object->UpdateTransform ();
	UpdateBounds ();

}

UIElement *
Control::InitializeFromXaml (const char *xaml, Type::Kind *element_type, XamlLoader *loader)
{
	// No callback, figure out how this will work in the plugin to satisfy deps
	UIElement *element = (UIElement*)xaml_create_from_str (loader, xaml, false, element_type);
	if (element == NULL)
		return NULL;

	Surface *surface = loader ? loader->GetSurface () : NULL;

	SetContent (element, surface);
	
	return element;
}

void
Control::SetBackground (Brush *bg)
{
	SetValue (Control::BackgroundProperty, Value (bg));
}

Brush *
Control::GetBackground ()
{
	Value *value = GetValue (Control::BackgroundProperty);
	
	return value ? value->AsBrush () : NULL;
}

void
Control::SetBorderBrush (Brush *brush)
{
	SetValue (Control::BorderBrushProperty, Value (brush));
}

Brush *
Control::GetBorderBrush ()
{
	Value *value = GetValue (Control::BorderBrushProperty);
	
	return value ? value->AsBrush () : NULL;
}

void
Control::SetBorderThickness (Thickness *thickness)
{
	SetValue (Control::BorderThicknessProperty, Value (*thickness));
}

Thickness *
Control::GetBorderThickness ()
{
	return GetValue (Control::BorderThicknessProperty)->AsThickness ();
}

void
Control::SetFontFamily (const char *family)
{
	SetValue (Control::FontFamilyProperty, Value (family));
}

const char *
Control::GetFontFamily ()
{
	Value *value = GetValue (Control::FontFamilyProperty);
	
	return value ? value->AsString () : NULL;
}

void
Control::SetFontSize (double size)
{
	SetValue (Control::FontSizeProperty, Value (size));
}

double
Control::GetFontSize ()
{
	return GetValue (Control::FontSizeProperty)->AsDouble ();
}

void
Control::SetFontStretch (FontStretches stretch)
{
	SetValue (Control::FontStretchProperty, Value (stretch));
}

FontStretches
Control::GetFontStretch ()
{
	return (FontStretches) GetValue (Control::FontStretchProperty)->AsInt32 ();
}

void
Control::SetFontStyle (FontStyles style)
{
	SetValue (Control::FontStyleProperty, Value (style));
}

FontStyles
Control::GetFontStyle ()
{
	return (FontStyles) GetValue (Control::FontStyleProperty)->AsInt32 ();
}

void
Control::SetFontWeight (FontWeights weight)
{
	SetValue (Control::FontWeightProperty, Value (weight));
}

FontWeights
Control::GetFontWeight ()
{
	return (FontWeights) GetValue (Control::FontWeightProperty)->AsInt32 ();
}

void
Control::SetForeground (Brush *fg)
{
	SetValue (Control::ForegroundProperty, Value (fg));
}

Brush *
Control::GetForeground ()
{
	Value *value = GetValue (Control::ForegroundProperty);
	
	return value ? value->AsBrush () : NULL;
}

void
Control::SetHorizontalContentAlignment (HorizontalAlignment alignment)
{
	SetValue (Control::HorizontalContentAlignmentProperty, Value (alignment));
}

HorizontalAlignment
Control::GetHorizontalContentAlignment ()
{
	return (HorizontalAlignment) GetValue (Control::HorizontalContentAlignmentProperty)->AsInt32 ();
}

void
Control::SetIsTabStop (bool value)
{
	SetValue (Control::IsTabStopProperty, Value (value));
}

bool
Control::GetIsTabStop ()
{
	return GetValue (Control::IsTabStopProperty)->AsBool ();
}

void
Control::SetPadding (Thickness *padding)
{
	SetValue (Control::PaddingProperty, Value (*padding));
}

Thickness *
Control::GetPadding ()
{
	return GetValue (Control::PaddingProperty)->AsThickness ();
}

void
Control::SetTabIndex (int index)
{
	SetValue (Control::TabIndexProperty, Value (index));
}

int
Control::GetTabIndex ()
{
	return (int) GetValue (Control::TabIndexProperty)->AsInt32 ();
}

void
Control::SetTabNavigation (KeyboardNavigationMode mode)
{
	SetValue (Control::TabNavigationProperty, Value (mode));
}

KeyboardNavigationMode
Control::GetTabNavigation ()
{
	return (KeyboardNavigationMode) GetValue (Control::TabNavigationProperty)->AsInt32 ();
}

void
Control::SetVerticalContentAlignment (VerticalAlignment alignment)
{
	SetValue (Control::VerticalContentAlignmentProperty, Value (alignment));
}

VerticalAlignment
Control::GetVerticalContentAlignment ()
{
	return (VerticalAlignment) GetValue (Control::VerticalContentAlignmentProperty)->AsInt32 ();
}



UIElement *
control_initialize_from_xaml (Control *control, const char *xaml,
			      Type::Kind *element_type)
{
	return control->InitializeFromXaml (xaml, element_type, NULL);
}

UIElement *
control_initialize_from_xaml_callbacks (Control *control, const char *xaml,
					Type::Kind *element_type, XamlLoader *loader)
{
	return control->InitializeFromXaml (xaml, element_type, loader);
}

void
control_set_background (Control *control, Brush *bg)
{
	control->SetBackground (bg);
}

Brush *
control_get_background (Control *control)
{
	return control->GetBackground ();
}

void
control_set_border_brush (Control *control, Brush *brush)
{
	control->SetBorderBrush (brush);
}

Brush *
control_get_border_brush (Control *control)
{
	return control->GetBorderBrush ();
}

void
control_set_border_thickness (Control *control, Thickness *thickness)
{
	control->SetBorderThickness (thickness);
}

Thickness *
control_get_border_thickness (Control *control)
{
	return control->GetBorderThickness ();
}

void
control_set_font_family (Control *control, const char *family)
{
	control->SetFontFamily (family);
}

const char *
control_get_font_family (Control *control)
{
	return control->GetFontFamily ();
}

void
control_set_font_size (Control *control, double size)
{
	control->SetFontSize (size);
}

double
control_get_font_size (Control *control)
{
	return control->GetFontSize ();
}

void
control_set_font_stretch (Control *control, FontStretches stretch)
{
	control->SetFontStretch (stretch);
}

FontStretches
control_get_font_stretch (Control *control)
{
	return control->GetFontStretch ();
}

void
control_set_font_style (Control *control, FontStyles style)
{
	control->SetFontStyle (style);
}

FontStyles
control_get_font_style (Control *control)
{
	return control->GetFontStyle ();
}

void
control_set_font_weight (Control *control, FontWeights weight)
{
	control->SetFontWeight (weight);
}

FontWeights
control_get_font_weight (Control *control)
{
	return control->GetFontWeight ();
}

void
control_set_foreground (Control *control, Brush *fg)
{
	control->SetForeground (fg);
}

Brush *
control_get_foreground (Control *control)
{
	return control->GetForeground ();
}

void
control_set_horizontal_content_alignment (Control *control, HorizontalAlignment alignment)
{
	control->SetHorizontalContentAlignment (alignment);
}

HorizontalAlignment
control_get_horizontal_content_alignment (Control *control)
{
	return control->GetHorizontalContentAlignment ();
}

void
control_set_is_tab_stop (Control *control, bool value)
{
	control->SetIsTabStop (value);
}

bool
control_get_is_tab_stop (Control *control)
{
	return control->GetIsTabStop ();
}

void
control_set_padding (Control *control, Thickness *padding)
{
	control->SetPadding (padding);
}

Thickness *
control_get_padding (Control *control)
{
	return control->GetPadding ();
}

void
control_set_tab_index (Control *control, int index)
{
	control->SetTabIndex (index);
}

int
control_get_tab_index (Control *control)
{
	return control->GetTabIndex ();
}

void
control_set_tab_navigation (Control *control, KeyboardNavigationMode mode)
{
	control->SetTabNavigation (mode);
}

KeyboardNavigationMode
control_get_tab_navigation (Control *control)
{
	return control->GetTabNavigation ();
}

void
control_set_vertical_content_alignment (Control *control, VerticalAlignment alignment)
{
	control->SetVerticalContentAlignment (alignment);
}

VerticalAlignment
control_get_vertical_content_alignment (Control *control)
{
	return control->GetVerticalContentAlignment ();
}


void
control_init (void)
{
	Control::BackgroundProperty = DependencyProperty::Register (Type::CONTROL, "Background", Type::BRUSH);
	Control::BorderBrushProperty = DependencyProperty::Register (Type::CONTROL, "BorderBrush", Type::BRUSH);
	Control::BorderThicknessProperty = DependencyProperty::Register (Type::CONTROL, "BorderThickness", new Value (Thickness (0.0)));
	Control::FontFamilyProperty = DependencyProperty::Register (Type::CONTROL, "FontFamily", new Value (CONTROL_FONT_FAMILY));
	Control::FontSizeProperty = DependencyProperty::Register (Type::CONTROL, "FontSize", new Value (CONTROL_FONT_SIZE));
	Control::FontStretchProperty = DependencyProperty::Register (Type::CONTROL, "FontStretch", new Value (CONTROL_FONT_STRETCH));
	Control::FontStyleProperty = DependencyProperty::Register (Type::CONTROL, "FontStyle", new Value (CONTROL_FONT_STYLE));
	Control::FontWeightProperty = DependencyProperty::Register (Type::CONTROL, "FontWeight", new Value (CONTROL_FONT_WEIGHT));
	Control::ForegroundProperty = DependencyProperty::Register (Type::CONTROL, "Foreground", Type::BRUSH);
	Control::HorizontalContentAlignmentProperty = DependencyProperty::Register (Type::CONTROL, "HorizontalContentAlignment", new Value (HorizontalAlignmentCenter));
	Control::IsTabStopProperty = DependencyProperty::Register (Type::CONTROL, "IsTabStop", new Value (true));
	Control::PaddingProperty = DependencyProperty::Register (Type::CONTROL, "Padding", new Value (Thickness (0.0)));
	Control::TabIndexProperty = DependencyProperty::Register (Type::CONTROL, "TabIndex", new Value (INT_MAX));
	Control::TabNavigationProperty = DependencyProperty::Register (Type::CONTROL, "TabNavigation", new Value (KeyboardNavigationModeLocal));
	Control::VerticalContentAlignmentProperty = DependencyProperty::Register (Type::CONTROL, "VerticalContentAlignment", new Value (VerticalAlignmentCenter));
}
