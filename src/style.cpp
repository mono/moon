/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * style.cpp:
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
#include "style.h"
#include "error.h"
#include "deployment.h"

namespace Moonlight {

//
// Style
//

Style::Style ()
{
	SetObjectType (Type::STYLE);
}

Style::~Style ()
{
}

void
Style::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType() != Type::STYLE) {
		DependencyObject::OnPropertyChanged (args, error);
		return;
	}

	if (args->GetId () == Style::BasedOnProperty) {
		if (GetIsSealed ()) {
			MoonError::FillIn (error, MoonError::INVALID_OPERATION, "Cannot change BasedOn on a sealed style");
			return; 
		}
	}
	
	NotifyListenersOfPropertyChange (args, error);
}

void
Style::Seal ()
{
	if (GetIsSealed ())
		return;

	Application::GetCurrent ()->ConvertSetterValues (this);
	SetIsSealed (true);
	GetSetters ()->Seal ();

	Style *s = GetBasedOn ();
	if (s)
		s->Seal ();
}

//
// SetterBaseCollection
//

SetterBaseCollection::SetterBaseCollection ()
{
	SetObjectType (Type::SETTERBASE_COLLECTION);
}

void
SetterBaseCollection::Seal ()
{
	SetIsSealed (true);
	
	CollectionIterator *iter = GetIterator ();
	MoonError err;
	
	Value *current;
	Types *types = Deployment::GetCurrent ()->GetTypes ();
	
	while (iter->Next (&err) && (current = iter->GetCurrent (&err))) {
		SetterBase *setter = current->AsSetterBase (types);
		setter->Seal ();
	}

	delete iter;
}

bool
SetterBaseCollection::AddedToCollection (Value *value, MoonError *error)
{ 
	if (!value || !ValidateSetter (value, error))
		return false;

	SetterBase *setter = value->AsSetterBase ();
	if (setter) {
		setter->SetAttached (true);
		setter->Seal ();
	}

	return DependencyObjectCollection::AddedToCollection (value, error);
}

void
SetterBaseCollection::RemovedFromCollection (Value *value)
{
	SetterBase *setter = value->AsSetterBase ();
	if (setter)
		setter->SetAttached (false);
	DependencyObjectCollection::RemovedFromCollection (value);
}

bool
SetterBaseCollection::ValidateSetter (Value *value, MoonError *error)
{
	Types *types = Deployment::GetCurrent ()->GetTypes ();

	if (types->IsSubclassOf (value->GetKind (), Type::SETTER)) {
		Setter *s = value->AsSetter (types);
		if (!s->GetValue (Setter::PropertyProperty)) {// || !s->GetValue (Setter::ValueProperty)) {
			MoonError::FillIn (error, MoonError::EXCEPTION, "Cannot have a null target property");
			return false;	
		}
	}
	
	if (types->IsSubclassOf (value->GetKind (), Type::SETTERBASE)) {
		SetterBase *s = value->AsSetterBase (types);
		if (s->GetAttached ()) {
			MoonError::FillIn (error, MoonError::INVALID_OPERATION, "Setter is currently attached to another style");
			return false;
		}
	}

	if (GetIsSealed ()) {
		MoonError::FillIn (error, MoonError::EXCEPTION, "Cannot add a setter to a sealed style");
		return false;
	}

	return true;
}


//
// SetterBase
//

SetterBase::SetterBase ()
{
	SetObjectType (Type::SETTERBASE);
	attached = false;
}

void
SetterBase::Seal ()
{
	if (GetIsSealed ())
		return;
	SetIsSealed (true);	
}

bool 
SetterBase::GetAttached ()
{
	return attached;	
}

void
SetterBase::SetAttached (bool value)
{
	attached = value;
}


//
// Setter
//

Setter::Setter ()
{
	SetObjectType (Type::SETTER);
}

Setter::~Setter ()
{
}

class StyleNode : public List::Node {
 public:
	StyleNode (Style *style) {
		this->style = style;
	}
	Style *style;
};

DeepStyleWalker::DeepStyleWalker (Style *style, Types *types)
{
	// Create a list of all Setters in the style sorted by their DP.
	// Use the hashtable to ensure that we only take the first setter
	// declared for each DP (i.e. if the BasedOn style and main style
	// have setters for the same DP, we ignore the BasedOn one
	
	// NOTE: This can be pre-computed and cached as once a style is
	// sealed it can never be changed.

	this->offset = 0;
	this->types = types || !style ? types : style->GetDeployment ()->GetTypes ();
	this->setter_list = g_ptr_array_new ();
	GHashTable *dps = g_hash_table_new (g_direct_hash, g_direct_equal);

	while (style != NULL) {
		SetterBaseCollection *setters = style->GetSetters ();
		int count = setters ? setters->GetCount () : 0;
		for (int i = 0; i < count; i++) {
			Value *v = setters->GetValueAt (i);
			if (Value::IsNull (v) || !types->IsSubclassOf (v->GetKind (), Type::SETTER))
				continue;

			Setter *setter = v->AsSetter ();
			Value* dpVal = setter->GetValue (Setter::PropertyProperty);
			if (Value::IsNull (dpVal))
				continue;

			DependencyProperty *prop = dpVal->AsDependencyProperty ();
			if (!g_hash_table_lookup_extended (dps, prop, NULL, NULL)) {
				g_hash_table_insert (dps, prop, setter);
				g_ptr_array_add (setter_list, setter);
			}
		}
		style = style->GetBasedOn ();
	}
	
	g_hash_table_destroy (dps);
	g_ptr_array_sort (setter_list, SetterComparer);
}

gint
DeepStyleWalker::SetterComparer (gconstpointer left, gconstpointer right)
{
	Setter *l = *(Setter **)left;
	Setter *r = *(Setter **)right;
	
	DependencyProperty *lprop = l->GetValue (Setter::PropertyProperty)->AsDependencyProperty ();
	DependencyProperty *rprop = r->GetValue (Setter::PropertyProperty)->AsDependencyProperty ();
	
	if (lprop == rprop)
		return 0;
	return lprop > rprop ? 1 : -1;
}

DeepStyleWalker::~DeepStyleWalker ()
{
	g_ptr_array_free (setter_list, true);
}

Setter *
DeepStyleWalker::Step ()
{
	if (offset < (int) setter_list->len)
		return (Setter *) setter_list->pdata [offset ++];
	return NULL;
}

};
