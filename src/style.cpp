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

Style::Style ()
{
	SetValue (Style::SettersProperty, Value::CreateUnref (new SetterBaseCollection()));
}

SetterBaseCollection::SetterBaseCollection ()
{
}

SetterBase::SetterBase ()
{
}

Setter::Setter ()
{
}
