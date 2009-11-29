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

/* @IncludeInKinds */
struct ManagedTypeInfo {
	char *assembly_name;
	char *full_name;
	
	ManagedTypeInfo (const ManagedTypeInfo& v);
	~ManagedTypeInfo ();

	void Initialize (const char *assembly_name, const char *full_name);

	bool operator == (const ManagedTypeInfo &v);
	bool operator != (const ManagedTypeInfo &v);
	ManagedTypeInfo& operator = (const ManagedTypeInfo &v);
	
	static void Free (ManagedTypeInfo *mti);
	
private:
	ManagedTypeInfo (); // we don't want this one to be called
};

#endif /* __MANAGEDTYPEINFO_H__ */
