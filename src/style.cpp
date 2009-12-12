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
Style::Seal ()
{
	SetIsSealed (true);
	GetSetters ()->Seal ();
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
	Types *types = Deployment::GetCurrent ()->GetTypes ();

	if (types->IsSubclassOf (value->GetKind (), Type::SETTER)) {
		Setter *s = value->AsSetter (types);
		if (!s->GetValue (Setter::PropertyProperty)) {
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
