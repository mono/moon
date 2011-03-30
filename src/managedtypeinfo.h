/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * managedtypeinfo.h: 
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MANAGEDTYPEINFO_H__
#define __MANAGEDTYPEINFO_H__

#include "type.h"

namespace Moonlight {

/* @IncludeInKinds */
class ManagedTypeInfo {
 public:
	char *full_name;
	Type::Kind kind;

	ManagedTypeInfo (Type::Kind kind, const char *full_name);
	ManagedTypeInfo (const ManagedTypeInfo& v);
	~ManagedTypeInfo ();

	void Initialize (Type::Kind kind, const char *full_name);

	bool operator == (const ManagedTypeInfo &v) const;
	bool operator != (const ManagedTypeInfo &v) const;
	ManagedTypeInfo& operator = (const ManagedTypeInfo &v);
	
private:
	ManagedTypeInfo (); // we don't want this one to be called
};

};
#endif /* __MANAGEDTYPEINFO_H__ */
