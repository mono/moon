/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * contentpresenter.cpp:
 *
 * Copyright 2007-2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "contentpresenter.h"


#define DEFAULT_TEMPLATE						\
	"<ControlTemplate"						\
	" xmlns=\"http://schemas.microsoft.com/client/2007\""		\
	" xmlns:x=\"http://schemas.microsoft.com/winfx/2006/xaml\""	\
	" xmlns:controls=\"clr-namespace:System.Windows.Controls;assembly=System.Windows.Controls\"" \
	" xmlns:controls=\"clr-namespace:System.Windows.Controls;assembly=System.Windows\"" \
	" TargetType=\"controls:ContentPresenter\">"			\
	"    <Grid x:Name=\"RootElement\""				\
	"     Background=\"{TemplateBinding Background}\""		\
	"     Cursor=\"{TemplateBinding Cursor}\">"			\
	"        <TextBlock x:Name=\"TextElement\""			\
	"         FontFamily=\"{TemplateBinding FontFamily}\""		\
	"         FontSize=\"{TemplateBinding FontSize}\""		\
	"         FontStretch=\"{TemplateBinding FontStretch}\""	\
	"         FontStyle=\"{TemplateBinding FontStyle}\""		\
	"         FontWeight=\"{TemplateBinding FontWeight}\""		\
	"         Foreground=\"{TemplateBinding Foreground}\""		\
	"         HorizontalAlignment=\"{TemplateBinding HorizontalContentAlignment}\"" \
	"         Padding=\"{TemplateBinding Padding}\""		\
	"         TextAlignment=\"{TemplateBinding TextAlignment}\""	\
	"         TextDecorations=\"{TemplateBinding TextDecorations}\"" \
	"         TextWrapping=\"{TemplateBinding TextWrapping}\""		\
	"         VerticalAlignment=\"{TemplateBinding VerticalContentAlignment}\"" \
	"         Visibility=\"Collapsed\"/>"				\
	"     </Grid>"							\
	"</ControlTemplate>"

ContentPresenter::ContentPresenter ()
{
	ManagedTypeInfo *type_info = new ManagedTypeInfo ();
	type_info->assembly_name = g_strdup ("System.Windows");
	type_info->full_name = g_strdup ("System.Windows.Controls.ContentPresenter");
	
	SetDefaultStyleKey (type_info);
	
	content = NULL;
	text = NULL;
	root = NULL;
}

void
ContentPresenter::OnLoaded ()
{
	DependencyObject *t;
	XamlLoader *loader;
	
	if (!(t = GetTemplate ())) {
		loader = new XamlLoader (NULL, DEFAULT_TEMPLATE, GetSurface ());
		t = loader->CreateFromString (DEFAULT_TEMPLATE, true, NULL);
		delete loader;
		
		SetTemplate ((ControlTemplate *) t);
		t->unref ();
	}
	
	Control::OnLoaded ();
	
	PrepareContentPresenter ();
}

void
ContentPresenter::OnApplyTemplate ()
{
	text = (TextBlock *) GetTemplateChild ("TextElement");
	root = (Grid *) GetTemplateChild ("RootElement");
}

void
ContentPresenter::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property->GetOwnerType () != Type::CONTENTPRESENTER) {
		Control::OnPropertyChanged (args);
		return;
	}
	
	if (args->property == ContentPresenter::ContentTemplateProperty) {
		if (IsLoaded ())
			PrepareContentPresenter ();
	} else if (args->property == ContentPresenter::ContentProperty) {
		if (IsLoaded ())
			PrepareContentPresenter ();
	}
	
	NotifyListenersOfPropertyChange (args);
}

void
ContentPresenter::PrepareContentPresenter ()
{
	UIElementCollection *children = root->GetChildren ();
	DependencyObject *obj;
	DataTemplate *t;
	Value *value;
	
	// Remove the old content 
	if (content != text)
		children->Remove (content); 
	else
		text->SetText (NULL);
	
	content = NULL;
	
	// Expand the ContentTemplate if it exists
	if ((t = GetContentTemplate ())) {
		obj = t->LoadContentWithError (NULL);
		if (obj && obj->Is (Type::UIELEMENT))
			content = (UIElement *) obj;
		
		value = NULL;
	} else {
		value = GetValue (ContentPresenter::ContentProperty);
		if (value && value->Is (Type::UIELEMENT))
			content = value->AsUIElement ();
	}
	
	if (content) {
		// display the uielement content
		children->Add (content);
		
		// collapse the text element
		text->SetVisibility (VisibilityCollapsed);
	} else if (value) {
		// display the text element and set the content to the stringified content
		text->SetVisibility (VisibilityVisible);
		switch (value->GetKind ()) {
		case Type::STRING:
			text->SetText (value->AsString ());
			break;
		default:
			// FIXME: implement me properly
			text->SetText (NULL);
			break;
		}
	} else {
		// no content
		text->SetVisibility (VisibilityCollapsed);
	}
}
