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
