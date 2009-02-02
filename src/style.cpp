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

Style::Style ()
{
	SetObjectType (Type::STYLE);
	SetValue (Style::SettersProperty, Value::CreateUnref (new SetterBaseCollection()));
	GetSetters ()->SetStyle (this);
}

Style::~Style ()
{
}

void
Style::Seal ()
{
	SetIsSealed (true);
	SetterBaseCollection *c = GetSetters ();
	if (c->GetCount () > 0)
		c->Seal ();
}

SetterBaseCollection::SetterBaseCollection ()
{
	SetObjectType (Type::SETTERBASE_COLLECTION);
	this->style = NULL;
}

void
SetterBaseCollection::SetStyle (Style *style)
{
	this->style = style;
}

void
SetterBaseCollection::Seal ()
{
	SetIsSealed (true);
	CollectionIterator *iter = GetIterator ();

	int error = 0;
	Value *current;
	while (iter->Next () && (current = iter->GetCurrent (&error))) {
		SetterBase *setter = current->AsSetterBase ();
		setter->Seal ();
	}
}

bool
SetterBaseCollection::AddedToCollection (Value *value, MoonError *error)
{ 
	if (!value || !ValidateSetter (value, error))
		return false;

	SetterBase *setter = value->AsSetterBase ();
	setter->SetAttached (true);
	setter->Seal ();

	return DependencyObjectCollection::AddedToCollection (value, error);
}

void
SetterBaseCollection::RemovedFromCollection (Value *value)
{
	value->AsSetterBase ()->SetAttached (false);
	DependencyObjectCollection::RemovedFromCollection (value);
}

bool
SetterBaseCollection::ValidateSetter (Value *value, MoonError *error)
{
	if (value->Is(Type::SETTER)) {
		Setter *s = value->AsSetter ();
		if (!s->GetValue (Setter::PropertyProperty)) {
			MoonError::FillIn (error, MoonError::EXCEPTION, "Cannot have a null target property");
			return false;	
		}
	}
	
	if (value->Is (Type::SETTERBASE)) {
		SetterBase *s = value->AsSetterBase ();
		if (s->GetAttached ()) {
			MoonError::FillIn (error, MoonError::INVALID_OPERATION, "Setter is currently attached to another style");
			return false;
		}
		if (s->GetIsSealed ()) {
			MoonError::FillIn (error, MoonError::EXCEPTION, "Cannot reuse a setter after it has been sealed");
			return false;
		}
	}

	if (GetIsSealed ()) {
		MoonError::FillIn (error, MoonError::EXCEPTION, "Cannot add a setter to a sealed style");
		return false;
	}

	return true;
}

bool 
SetterBase::GetAttached ()
{
	return this->attached;	
}
void
SetterBase::SetAttached (bool value)
{
	this->attached = value;
}

SetterBase::SetterBase ()
{
	SetObjectType (Type::SETTERBASE);
	this->attached = false;
}

void
SetterBase::Seal ()
{
	if (GetIsSealed ())
		return;
	SetIsSealed (true);	
}

bool
SetterBase::SetValueWithErrorImpl (DependencyProperty *property, Value *value, MoonError *error)
{
	if (GetIsSealed () && property != Setter::ConvertedValueProperty) {
		MoonError::FillIn (error, MoonError::UNAUTHORIZED_ACCESS, "Cannot modify a setter after it is used");
		return false;
	}
	
	return DependencyObject::SetValueWithErrorImpl (property, value, error);
}

Setter::Setter ()
{
	SetObjectType (Type::SETTER);
}

Setter::~Setter ()
{
}
