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
	SetValue (Style::SettersProperty, Value::CreateUnref (new SetterBaseCollection()));
	GetSetters ()->style = this;
}

void
Style::Seal ()
{
	SetIsSealed (true);
	GetSetters ()->Seal ();
}

SetterBaseCollection::SetterBaseCollection ()
{
	this->style = NULL;
}

int
SetterBaseCollection::AddWithError (Value *value, MoonError *error)
{
	if (style && style->GetIsSealed ()) {
		MoonError::FillIn (error, MoonError::EXCEPTION, "Cannot modify a style after it is used");
		return -1;
	}
	return DependencyObjectCollection::AddWithError (value, error);	
}

int
SetterBaseCollection:: InsertWithError (int index, Value *value, MoonError *error)
{
	if (style && style->GetIsSealed ()) {
		MoonError::FillIn (error, MoonError::EXCEPTION, "Cannot modify a style after it is used");
		return -1;
	}
	return DependencyObjectCollection::InsertWithError (index, value, error);	
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
SetterBaseCollection::SetValueAtWithError (int index, Value *value, MoonError *error)
{
	if (style && style->GetIsSealed ()) {
		MoonError::FillIn (error, MoonError::EXCEPTION, "Cannot modify a style after it is used");
		return false;
	}
	return DependencyObjectCollection::SetValueAtWithError (index, value, error);	
}

SetterBase::SetterBase ()
{
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
