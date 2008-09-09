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



static void
delete_list (gpointer p)
{
	delete ((List*)p);
}

FrameworkTemplate::FrameworkTemplate ()
{
	xaml_bindings = g_hash_table_new_full (g_direct_hash,
					       g_direct_equal,
					       NULL,
					       delete_list);
	visual_tree = NULL;
}

FrameworkTemplate::~FrameworkTemplate ()
{
	g_hash_table_destroy (xaml_bindings);
	if (visual_tree)
		visual_tree->unref();
}

void
FrameworkTemplate::SetVisualTree (FrameworkElement *value)
{
	visual_tree = value;
	visual_tree->ref ();
}

void
FrameworkTemplate::AddXamlBinding (XamlTemplateBinding *binding)
{
	if (binding == NULL) {
		g_warning("AddXamlBinding passed NULL binding");
		return;
	}
		
	List *l = (List*)g_hash_table_lookup (xaml_bindings, binding->GetTarget());
	if (!l) {
		l = new List();
		g_hash_table_insert (xaml_bindings, binding->GetTarget(), l);
	}
	XamlTemplateBindingNode *node = new XamlTemplateBindingNode (binding);
	l->Append (node);
}


ControlTemplate::ControlTemplate ()
{
}

struct duplicate_value_closure {
	ControlTemplate *t;
	Control *source;
	DependencyObject *dob;
	List *bindings;
};

void
ControlTemplate::duplicate_value (DependencyProperty *key,
				  Value *value,
				  gpointer data)
{
	duplicate_value_closure *closure = (duplicate_value_closure*)data;
	ControlTemplate *t = closure->t;
	Control *source = closure->source;
	DependencyObject *dob = closure->dob;
	List *bindings = closure->bindings;

	if (value->Is (Type::DEPENDENCY_OBJECT))
		dob->SetValue (key, Value (t->DuplicateObject (source, value->AsDependencyObject(), bindings)));
	else
		dob->SetValue (key, new Value (*value));
}


DependencyObject*
ControlTemplate::DuplicateObject (Control *source, DependencyObject *dob, List* bindings)
{
	DependencyObject *new_dob = dob->GetType()->CreateInstance ();

	/* iterate over all of dob's values, and copy them over */
	duplicate_value_closure closure;

	closure.t = this;
	closure.source = source;
	closure.dob = new_dob;
	closure.bindings = bindings;

	g_hash_table_foreach (dob->GetCurrentValues(), (GHFunc)duplicate_value, &closure);

	if (dob->Is (Type::COLLECTION)) {
		Collection *c = (Collection*)dob;
		Collection *new_c = (Collection*)new_dob;

		if (Type::Find(c->GetElementType())->IsSubclassOf(Type::DEPENDENCY_OBJECT)) {
			for (int i = 0; i < c->GetCount(); i ++)
				new_c->Add(Value (DuplicateObject(source, c->GetValueAt(i)->AsDependencyObject(), bindings)));
		}
		else {
			for (int i = 0; i < c->GetCount(); i ++)
				new_c->Add(c->GetValueAt(i));
		}
	}
	else if (dob->Is (Type::FRAMEWORKTEMPLATE)) {
		FrameworkTemplate *t = (FrameworkTemplate*)dob;
		FrameworkTemplate *new_t = (FrameworkTemplate*)new_dob;
		new_t->SetVisualTree ((FrameworkElement*)DuplicateObject (source, t->GetVisualTree(), bindings));
	}

	/* check if dob exists in the xaml binding hash. */
	List *l = (List*)g_hash_table_lookup (xaml_bindings, dob);
	if (l) {
		/* and if it does, iterate over the list of
		   XamlTemplateBindings, creating TemplateBindings and
		   adding them to the bindings list above */
		List::Node *node;
		for (node = l->First(); node; node = node->next) {
			XamlTemplateBindingNode *x = (XamlTemplateBindingNode*)node;
			TemplateBinding *b = x->GetBinding()->Attach (source, (FrameworkElement*)new_dob);
			if (b) {
				bindings->Append (new TemplateBindingNode (b));
				b->unref();
			}
		}
	}

	return new_dob;
}

FrameworkElement *
ControlTemplate::Apply (Control *control, List *bindings)
{
	if (!visual_tree)
		return NULL;

	DependencyObject *instantiated_tree = DuplicateObject (control, visual_tree, bindings);

	return (FrameworkElement *)instantiated_tree;
}

XamlTemplateBinding::XamlTemplateBinding (FrameworkElement *target,
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
XamlTemplateBinding::Attach (Control *source, FrameworkElement *target)
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

	return new TemplateBinding (source, sourceProperty,
				    target, targetProperty);
}

TemplateBinding::TemplateBinding (Control *source,
				  DependencyProperty *sourceProperty,
				  FrameworkElement *target,
				  DependencyProperty *targetProperty)
{
	this->source = source;
	this->sourceProperty = sourceProperty;
	this->target = target;
	this->targetProperty = targetProperty;

	source->AddPropertyChangeListener (this, NULL);

	// maybe this first step should be done elsewhere?
	target->SetValue (targetProperty, source->GetValue (sourceProperty));
}

TemplateBinding::~TemplateBinding ()
{
	source->RemovePropertyChangeListener (this, NULL);
}

void
TemplateBinding::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj,
				       PropertyChangedEventArgs *subobj_args)
{
	if (subobj_args->property != sourceProperty)
		return;
	
	target->SetValue (targetProperty, subobj_args->new_value);
}
