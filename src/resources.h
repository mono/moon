/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * resources.h: Resource dictionaries.
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_RESOURCES_H__
#define __MOON_RESOURCES_H__

#include <glib.h>

#include "collection.h"
#include "value.h"

/* @Namespace=System.Windows */
class ResourceDictionary : public Collection {
public:
	/* @GenerateCBinding,GeneratePInvoke */
	ResourceDictionary ();

	virtual Type::Kind GetObjectType () { return Type::RESOURCE_DICTIONARY; }

	/* just to provide an implementation.  our CanAdd always returns true. */
	virtual Type::Kind GetElementType () { return Type::INVALID;}

	bool Add (char* key, Value *value);

	/* @GenerateCBinding,GeneratePInvoke */
	void AddWithError (char* key, Value *value, MoonError *error);

	/* @GenerateCBinding,GeneratePInvoke */
	bool Clear ();

	/* @GenerateCBinding,GeneratePInvoke */
	bool ContainsKey (char *key);

	/* @GenerateCBinding,GeneratePInvoke */
	bool Remove (char *key);

	/* @GenerateCBinding,GeneratePInvoke */
	bool Set (char *key, Value *value);

	/* @GenerateCBinding,GeneratePInvoke */
	Value* Get (char *key, bool *exists);

	virtual void SetSurface (Surface *surface);
	virtual void UnregisterAllNamesRootedAt (NameScope *from_ns);
	virtual void RegisterAllNamesRootedAt (NameScope *to_ns);

protected:
	virtual ~ResourceDictionary ();

	virtual bool CanAdd (Value *value);

	virtual void AddedToCollection (Value *value);
	virtual void RemovedFromCollection (Value *value);

	void MergeNames (DependencyObject *new_obj);

private:
	GHashTable *hash;
};

#endif /* __MOON_RESOURCES_H__ */
