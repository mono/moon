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
	Type::Kind kind;

	/* @GeneratePInvoke */
	ManagedTypeInfo (Type::Kind kind);

	bool operator == (const ManagedTypeInfo &v) const;
	bool operator != (const ManagedTypeInfo &v) const;

private:
	ManagedTypeInfo (); // we don't want this one to be called
};

};
#endif /* __MANAGEDTYPEINFO_H__ */
