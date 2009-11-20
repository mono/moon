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

ManagedTypeInfo::ManagedTypeInfo (const ManagedTypeInfo& v)
{
	Initialize (v.assembly_name, v.full_name);
}

ManagedTypeInfo::~ManagedTypeInfo ()
{
	g_free (assembly_name);
	g_free (full_name);
}

void
ManagedTypeInfo::Initialize (const char *assembly_name, const char *full_name)
{
	this->assembly_name = g_strdup (assembly_name);
	this->full_name = g_strdup (full_name);
}

bool
ManagedTypeInfo::operator == (const ManagedTypeInfo &v)
{
	return strcmp (assembly_name, v.assembly_name) == 0 && strcmp (full_name, v.full_name) == 0;
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
		g_free (this->assembly_name);
		g_free (this->full_name);
		this->assembly_name = g_strdup (v.assembly_name);
		this->full_name = g_strdup (v.full_name);
	}
	return *this;
}

void
ManagedTypeInfo::Free (ManagedTypeInfo *mti)
{
	if (mti == NULL)
		return;
	
	g_free (mti->assembly_name);
	g_free (mti->full_name);
	g_free (mti);
}
