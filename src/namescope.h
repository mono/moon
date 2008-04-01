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
#include "list.h"

class NameScope : public DependencyObject {
 protected:
	virtual ~NameScope ();

 public:
	NameScope ();

	virtual Type::Kind GetObjectType () { return Type::NAMESCOPE; }

	void RegisterName (const char *name, DependencyObject *object);
	void UnregisterName (const char *name);

	DependencyObject* FindName (const char *name);

	void SetTemporary (bool flag) { temporary = flag; }
	bool GetTemporary () { return temporary; }

	void MergeTemporaryScope (NameScope *scope);

	static NameScope* GetNameScope (DependencyObject *obj);
	static void SetNameScope (DependencyObject *obj, NameScope *scope);

	static DependencyProperty *NameScopeProperty;
	
 private:
	static void ObjectDestroyedEvent (EventObject *sender, EventArgs *args, gpointer closure);

	static gboolean remove_handler (gpointer key, gpointer value, gpointer data);

	static void merge_name (gpointer key, gpointer value, gpointer user_data);

	GHashTable *names;

	bool temporary;
};

G_BEGIN_DECLS

void namescope_init (void);

G_END_DECLS

#endif /* __MOON_NAMESCOPE_H__ */
