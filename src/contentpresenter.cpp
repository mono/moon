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
			ApplyTemplate ();
	} else if (args->property == ContentPresenter::ContentProperty) {
		if (IsLoaded ())
			ApplyTemplate ();
	}
	
	NotifyListenersOfPropertyChange (args);
}
