/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * contentcontrol.cpp:
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include "contentcontrol.h"


DependencyProperty *ContentControl::ContentProperty;
DependencyProperty *ContentControl::ContentTemplateProperty;
DependencyProperty *ContentControl::IsEnabledProperty;
DependencyProperty *ContentControl::TextAlignmentProperty;
DependencyProperty *ContentControl::TextDecorationsProperty;
DependencyProperty *ContentControl::TextWrappingProperty;


void
ContentControl::SetContent (DependencyObject *content)
{
	SetValue (ContentControl::ContentProperty, Value (content));
}

DependencyObject *
ContentControl::GetContent ()
{
	Value *value = GetValue (ContentControl::ContentProperty);
	
	return value ? value->AsDependencyObject () : NULL;
}

#if 0
void
ContentControl::SetContentTemplate (DataTemplate *t)
{
	SetValue (ContentControl::ContentTemplateProperty, Value (t));
}

DataTemplate *
ContentControl::GetContentTemplate ()
{
	Value *value = GetValue (ContentControl::ContentTemplateProperty);
	
	return value ? value->AsDataTemplate () : NULL;
}
#endif

void
ContentControl::SetIsEnabled (bool value)
{
	SetValue (ContentControl::IsEnabledProperty, Value (value));
}

bool
ContentControl::GetIsEnabled ()
{
	return GetValue (ContentControl::IsEnabledProperty)->AsBool ();
}

void
ContentControl::SetTextAlignment (TextAlignment alignment)
{
	SetValue (ContentControl::TextAlignmentProperty, Value (alignment));
}

TextAlignment
ContentControl::GetTextAlignment ()
{
	return (TextAlignment) GetValue (ContentControl::TextAlignmentProperty)->AsInt32 ();
}

void
ContentControl::SetTextDecorations (TextDecorations decorations)
{
	SetValue (ContentControl::TextDecorationsProperty, Value (decorations));
}

TextDecorations
ContentControl::GetTextDecorations ()
{
	return (TextDecorations) GetValue (ContentControl::TextDecorationsProperty)->AsInt32 ();
}

void
ContentControl::SetTextWrapping (TextWrapping wrapping)
{
	SetValue (ContentControl::TextWrappingProperty, Value (wrapping));
}

TextWrapping
ContentControl::GetTextWrapping ()
{
	return (TextWrapping) GetValue (ContentControl::TextWrappingProperty)->AsInt32 ();
}


ContentControl *
content_control_new (void)
{
	return new ContentControl ();
}

void
content_control_set_content (ContentControl *content_control, DependencyObject *content)
{
	content_control->SetContent (content);
}

DependencyObject *
content_control_get_content (ContentControl *content_control)
{
	return content_control->GetContent ();
}

#if 0
void
content_control_set_content_template (ContentControl *content_control, DataTemplate *t)
{
	content_control->SetContentTemplate (t);
}

DataTemplate *
content_control_get_content_template (ContentControl *content_control)
{
	return content_control->GetContentTemplate ();
}
#endif

void
content_control_set_is_enabled (ContentControl *content_control, bool value)
{
	content_control->SetIsEnabled (value);
}

bool
content_control_get_is_enabled (ContentControl *content_control)
{
	return content_control->GetIsEnabled ();
}

void
content_control_set_text_alignment (ContentControl *content_control, TextAlignment alignment)
{
	content_control->SetTextAlignment (alignment);
}

TextAlignment
content_control_get_text_alignment (ContentControl *content_control)
{
	return content_control->GetTextAlignment ();
}

void
content_control_set_text_decorations (ContentControl *content_control, TextDecorations decorations)
{
	content_control->SetTextDecorations (decorations);
}

TextDecorations
content_control_get_text_decorations (ContentControl *content_control)
{
	return content_control->GetTextDecorations ();
}

void
content_control_set_text_wrapping (ContentControl *content_control, TextWrapping wrapping)
{
	content_control->SetTextWrapping (wrapping);
}

TextWrapping
content_control_get_text_wrapping (ContentControl *content_control)
{
	return content_control->GetTextWrapping ();
}


void
content_control_init (void)
{
	ContentControl::ContentProperty = DependencyProperty::Register (Type::CONTENTCONTROL, "Content", Type::DEPENDENCY_OBJECT);
	//ContentControl::ContentTemplateProperty = DependencyProperty::Register (Type::CONTENTCONTROL, "ContentTemplate", Type::DEPENDENCYOBJECT);
	ContentControl::IsEnabledProperty = DependencyProperty::Register (Type::CONTENTCONTROL, "IsEnabled", new Value (true));
	ContentControl::TextAlignmentProperty = DependencyProperty::Register (Type::CONTENTCONTROL, "TextAlignment", new Value (TextAlignmentLeft));
	ContentControl::TextDecorationsProperty = DependencyProperty::Register (Type::CONTENTCONTROL, "TextDecorations", new Value (TextDecorationsNone));
	ContentControl::TextWrappingProperty = DependencyProperty::Register (Type::CONTENTCONTROL, "TextWrapping", new Value (TextWrappingNoWrap));
}
