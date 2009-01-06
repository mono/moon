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

bool
ValidateSetter (SetterBaseCollection *collection, Value *value, MoonError *error)
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

	if (collection->GetIsSealed ()) {
		MoonError::FillIn (error, MoonError::EXCEPTION, "Cannot add a setter to a sealed style");
		return false;
	}

	return true;
}

Style::Style ()
{
	SetValue (Style::SettersProperty, Value::CreateUnref (new SetterBaseCollection()));
	GetSetters ()->style = this;
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
	this->style = NULL;
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
	if (!value || !ValidateSetter (this, value, error))
		return false;

	SetterBase *setter = value->AsSetterBase ();
	setter->SetAttached (true);

	if (style->GetIsSealed ())
		setter->Seal ();

	return true;
}

void
SetterBaseCollection::RemovedFromCollection (Value *value)
{
	value->AsSetterBase ()->SetAttached (false);
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
	if (GetIsSealed ()) {
		MoonError::FillIn (error, MoonError::UNAUTHORIZED_ACCESS, "Cannot modify a setter after it is used");
		return false;
	}
	
	return DependencyObject::SetValueWithErrorImpl (property, value, error);
}

Setter::Setter ()
{
}
