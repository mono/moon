/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * managedtypeinfo.cpp
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>
#include <glib.h>
#include <string.h>

#include "managedtypeinfo.h"

namespace Moonlight {

ManagedTypeInfo::ManagedTypeInfo (Type::Kind kind)
{
	this->kind = kind;
}

bool
ManagedTypeInfo::operator == (const ManagedTypeInfo &v) const
{
	return kind == v.kind;
}

bool
ManagedTypeInfo::operator != (const ManagedTypeInfo &v) const
{
	return !(*this == v);
}

};
