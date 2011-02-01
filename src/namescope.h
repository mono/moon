/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * namescope.h
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_NAMESCOPE_H__
#define __MOON_NAMESCOPE_H__

#include <glib.h>
#include "dependencyobject.h"
#include "error.h"
#include "list.h"

/* @Namespace=None */
/* @ManagedDependencyProperties=None */
/* @ManagedEvents=None */
class MOON_API NameScope : public DependencyObject {
	GHashTable *names;
	bool is_locked;
	bool temporary;
	
	static void ObjectDestroyedEvent (EventObject *sender, EventArgs *args, gpointer closure);
	static gboolean remove_handler (gpointer key, gpointer value, gpointer data);
	static void merge_name (gpointer key, gpointer value, gpointer user_data);
	
 protected:
	virtual ~NameScope ();

	virtual void CloneCore (Types *types, DependencyObject *fromObj);

 public:
 	/* @PropertyType=NameScope,Attached,GenerateAccessors */
	const static int NameScopeProperty;
	
	/* @GenerateCBinding*/
	NameScope ();

	virtual void Dispose ();
	
	void RegisterName (const char *name, DependencyObject *object);
	void UnregisterName (const char *name);
	
	DependencyObject *FindName (const char *name);
	
	bool GetIsLocked () { return is_locked; }
	void Lock () { is_locked = true; }
	
	void SetTemporary (bool flag) { temporary = flag; }
	bool GetTemporary () { return temporary; }
	
	void MergeTemporaryScope (NameScope *scope, MoonError *error);
	
	static NameScope *GetNameScope (DependencyObject *obj);
	static void SetNameScope (DependencyObject *obj, NameScope *scope);

	void Dump();

	GHashTable *GetNames () { return names; }
};

#endif /* __MOON_NAMESCOPE_H__ */
