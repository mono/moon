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
	/* @PropertyType=ResourceDictionaryCollection,AutoCreateValue,ManagedFieldAccess=Internal,ManagedSetterAccess=Internal,GenerateAccessors */
	const static int MergedDictionariesProperty;

	/* @GenerateCBinding,GeneratePInvoke */
	ResourceDictionary ();

	/* just to provide an implementation.  our CanAdd always returns true. */
	virtual Type::Kind GetElementType () { return Type::INVALID;}

	bool Add (const char* key, Value *value);

	/* @GenerateCBinding,GeneratePInvoke */
	bool AddWithError (const char* key, Value *value, MoonError *error);

	/* @GenerateCBinding,GeneratePInvoke */
	bool Clear ();

	/* @GenerateCBinding,GeneratePInvoke */
	bool ContainsKey (const char *key);

	/* @GenerateCBinding,GeneratePInvoke */
	bool Remove (const char *key);

	/* @GenerateCBinding,GeneratePInvoke */
	bool Set (const char *key, Value *value);

	/* @GenerateCBinding,GeneratePInvoke */
	Value* Get (const char *key, bool *exists);
	Value* GetFromMergedDictionaries (const char *key, bool *exists);

	ResourceDictionaryCollection *GetMergedDictionaries ();
	void SetMergedDictionaries (ResourceDictionaryCollection* value);

	virtual void SetIsAttached (bool value);
	virtual void UnregisterAllNamesRootedAt (NameScope *from_ns);
	virtual void RegisterAllNamesRootedAt (NameScope *to_ns, MoonError *error);

protected:
	virtual ~ResourceDictionary ();

	virtual bool CanAdd (Value *value);

	virtual bool AddedToCollection (Value *value, MoonError *error);
	virtual void RemovedFromCollection (Value *value);

private:
	GHashTable *hash;
	bool from_resource_dictionary_api;
};

#endif /* __MOON_RESOURCES_H__ */
