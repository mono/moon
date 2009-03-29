/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * template.cpp:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */


#include <config.h>
#include "template.h"
#include "namescope.h"

class XamlTemplateBindingNode : public List::Node {
public:
	XamlTemplateBindingNode (XamlTemplateBinding *binding)
	{
		this->binding = binding;
		binding->ref();
	}

	virtual ~XamlTemplateBindingNode () { binding->unref (); }

	XamlTemplateBinding* GetBinding () { return binding; }
private:
	XamlTemplateBinding *binding;
};

class TemplateBindingNode : public List::Node {
public:
	TemplateBindingNode (TemplateBinding *binding)
	{
		this->binding = binding;
		binding->ref();
	}

	virtual ~TemplateBindingNode () { binding->unref (); }

	TemplateBinding* GetBinding () { return binding; }
private:
	TemplateBinding *binding;
};


FrameworkTemplate::FrameworkTemplate ()
{
	SetObjectType (Type::FRAMEWORKTEMPLATE);

	xaml_buffer = NULL;
	xaml_context = NULL;
}

FrameworkTemplate::~FrameworkTemplate ()
{
	if (xaml_buffer) {
		g_free (xaml_buffer);
		xaml_buffer = NULL;
	}
	delete xaml_context;
	xaml_context = NULL;
}

void
FrameworkTemplate::SetXamlBuffer (XamlContext *xaml_context, const char *xaml_buffer)
{
//	printf ("%p setting xaml buffer to %s\n", this, xaml_buffer);
	this->xaml_buffer = g_strdup (xaml_buffer);
	this->xaml_context = xaml_context;
}

DependencyObject*
FrameworkTemplate::GetVisualTree (FrameworkElement *templateBindingSource, List *templateBindings)
{
	if (xaml_buffer) {
		XamlLoader *loader = new XamlLoader (NULL, xaml_buffer, GetSurface(), xaml_context);
		Type::Kind dummy;

		this->templateBindingSource = templateBindingSource;
		if (templateBindingSource)
			templateBindingSource->ref ();
		this->templateBindings = templateBindings;
		DependencyObject *result = loader->CreateFromString (xaml_buffer, true, &dummy);

		if (templateBindingSource)
			templateBindingSource->unref ();
		this->templateBindingSource = NULL;
		this->templateBindings = NULL;

		delete loader;

		return result;
	}

	return NULL;
}

void
FrameworkTemplate::AddXamlBinding (XamlTemplateBinding *binding)
{
	if (templateBindingSource && templateBindings) {
		TemplateBinding *b = binding->Attach (templateBindingSource);
		if (b) {
			templateBindings->Append (new TemplateBindingNode (b));
			b->unref();
		}
	}
}

void
FrameworkTemplate::AddXamlBinding (FrameworkElement *target, const char *target_prop_name, const char *source_prop_name)
{
	XamlTemplateBinding *binding = new XamlTemplateBinding (target, target_prop_name, source_prop_name);
	AddXamlBinding (binding);

	binding->unref ();
}

ControlTemplate::ControlTemplate ()
{
	SetObjectType (Type::CONTROLTEMPLATE);
}

FrameworkElement *
ControlTemplate::Apply (Control *control, List *bindings)
{
	return (FrameworkElement*)GetVisualTree (control, bindings);
}

DataTemplate::DataTemplate ()
{
	SetObjectType (Type::DATATEMPLATE);
}

DependencyObject*
DataTemplate::LoadContentWithError (MoonError *error)
{
	printf ("%p: LoadContentWithError (buffer = %s)\n", this, xaml_buffer);

#if 0
	// this isn't the best way to do this, perhaps...
	if (g_hash_table_size (xaml_bindings) > 0) {
		// there are TemplateBinding elements inside this
		// DataTemplate.  those are illegal in this context.
		// Oddly enough, the error isn't given at parse time
		// but at LoadContent time, with a XamlParseException.
		MoonError::FillIn (error, MoonError::XAML_PARSE_EXCEPTION, 4004, "Invalid use of {TemplateBinding} markup extension in DataTemplate");
		return NULL;
	}
#endif

	return GetVisualTree (NULL, NULL);
}

XamlTemplateBinding::XamlTemplateBinding (DependencyObject *target,
					  const char *targetPropertyName,
					  const char *sourcePropertyName)
{
	this->target = target;
	this->targetPropertyName = g_strdup (targetPropertyName);
	this->sourcePropertyName = g_strdup (sourcePropertyName);
}

XamlTemplateBinding::~XamlTemplateBinding ()
{
	g_free (targetPropertyName);
	g_free (sourcePropertyName);
}

TemplateBinding*
XamlTemplateBinding::Attach (DependencyObject *source)
{
	if (source == NULL) {
		g_warning ("Attaching templatebinding to null control");
		return NULL;
	}

	DependencyProperty *sourceProperty = source->GetDependencyProperty (sourcePropertyName);

	if (sourceProperty == NULL) {
		g_warning ("non-existant source property '%s' on control", sourcePropertyName);
		return NULL;
	}

	DependencyProperty *targetProperty = target->GetDependencyProperty (targetPropertyName);

	if (targetProperty == NULL) {
		g_warning ("non-existant target property '%s'", targetPropertyName);
		return NULL;
	}

// 	printf ("TemplateBinding attaching %s.%s -> %s.%s\n",
// 		source->GetTypeName(), sourceProperty->GetName(),
// 		target->GetTypeName(), targetProperty->GetName());

	return new TemplateBinding (source, sourceProperty,
				    target, targetProperty);
}

TemplateBinding::TemplateBinding (DependencyObject *source,
				  DependencyProperty *sourceProperty,
				  DependencyObject *target,
				  DependencyProperty *targetProperty)
{
	this->source = source;
	this->sourceProperty = sourceProperty;
	this->target = target;
	this->targetProperty = targetProperty;

	source->AddPropertyChangeHandler (sourceProperty, SourcePropertyChangedCallback, this);

	// maybe this first step should be done elsewhere?
	target->SetValue (targetProperty, source->GetValue (sourceProperty));
}

TemplateBinding::~TemplateBinding ()
{
	source->RemovePropertyChangeHandler (sourceProperty, SourcePropertyChangedCallback);
}

void
TemplateBinding::OnSourcePropertyChanged (DependencyObject *sender, PropertyChangedEventArgs *args)
{
	target->SetValue (targetProperty, args->GetNewValue());
}

void
TemplateBinding::SourcePropertyChangedCallback (DependencyObject *sender, PropertyChangedEventArgs *args, gpointer closure)
{
	TemplateBinding *binding = (TemplateBinding*) closure;
	binding->OnSourcePropertyChanged (sender, args);
}
