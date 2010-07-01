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

ManagedTypeInfo::ManagedTypeInfo (Type::Kind kind, const char *full_name)
{
	Initialize (kind, full_name);
}

ManagedTypeInfo::ManagedTypeInfo (const ManagedTypeInfo& v)
{
	Initialize (v.kind, v.full_name);
}

ManagedTypeInfo::~ManagedTypeInfo ()
{
	g_free (full_name);
}

void
ManagedTypeInfo::Initialize (Type::Kind kind, const char *full_name)
{
	this->kind = kind;
	this->full_name = g_strdup (full_name);
}

bool
ManagedTypeInfo::operator == (const ManagedTypeInfo &v)
{
	return kind == v.kind && strcmp (full_name, v.full_name) == 0;
}

bool
ManagedTypeInfo::operator != (const ManagedTypeInfo &v)
{
	return !(*this == v);
}

ManagedTypeInfo&
ManagedTypeInfo::operator = (const ManagedTypeInfo &v)
{
	if (this != &v) {
		g_free (this->full_name);
		this->kind = v.kind;
		this->full_name = g_strdup (v.full_name);
	}
	return *this;
}

void
ManagedTypeInfo::Free (ManagedTypeInfo *mti)
{
	if (mti == NULL)
		return;
	
	g_free (mti->full_name);
	g_free (mti);
}
