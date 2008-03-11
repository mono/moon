/*
 * namescope.cpp
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>
#include <glib.h>

#include "namescope.h"

class NameScopeNode : public List::Node {
public:
	NameScopeNode (NameScope *ns)
	{
		this->ns = ns;
		this->ns->ref ();
	}

	virtual ~NameScopeNode ()
	{
		this->ns->unref ();
	}
	
	NameScope *ns;
};

NameScope::NameScope ()
{
	names = g_hash_table_new_full (g_str_hash, g_str_equal,
				       (GDestroyNotify)g_free,
				       (GDestroyNotify)base_unref);
	merged_child_namescopes = new List ();
	temporary = false;
	merged = false;
}

NameScope::~NameScope ()
{
	merged_child_namescopes->Clear (true);
	delete merged_child_namescopes;
	g_hash_table_destroy (names);
}

void
NameScope::RegisterName (const char *name, DependencyObject *object)
{
	object->ref();
	g_hash_table_insert (names, g_strdup (name), object);
}

void
NameScope::UnregisterName (const char *name)
{
	g_hash_table_remove (names, name);
}

DependencyObject*
NameScope::FindName (const char *name)
{
	if (name == NULL) {
		g_warning ("NULL passed to FindName");
		return NULL;
	}

	DependencyObject *o = (DependencyObject*)g_hash_table_lookup (names, name);
	if (o)
		return o;

	for (NameScopeNode *n = (NameScopeNode *) merged_child_namescopes->First (); n; n = (NameScopeNode *) n->next) {
		o = n->ns->FindName (name);
		if (o)
			return o;
	}

	return NULL;
}

NameScope*
NameScope::GetNameScope (DependencyObject *obj)
{
	Value *v = obj->GetValue (NameScope::NameScopeProperty);
	return v == NULL ? NULL : v->AsNameScope();
}

void
NameScope::SetNameScope (DependencyObject *obj, NameScope *scope)
{
	obj->SetValue (NameScope::NameScopeProperty, scope);
}

void
NameScope::MergeTemporaryScope (NameScope *temp)
{
	merged_child_namescopes->Append (new NameScopeNode (temp));
	temp->SetMerged (true);
}

static bool
NameScopeNodeFinder (List::Node *node, void *data)
{
	NameScopeNode *n = (NameScopeNode*)node;
	return (n->ns == data);
}

void
NameScope::UnmergeTemporaryScope (NameScope *temp)
{
	merged_child_namescopes->Remove (NameScopeNodeFinder, temp);

	// XXX this line is commented out to duplicate microsoft's
	// (IMO) broken behavior when it comes to temporary namescopes
	// on objects which are then removed from the tree.  In MS's
	// implementation, the namescope is gone for good, and removing
	// the element from the tree doesn't bring it back.

	// temp->SetMerged (false);
}

DependencyProperty *NameScope::NameScopeProperty;

void
namescope_init (void)
{
	NameScope::NameScopeProperty = DependencyObject::Register (Type::NAMESCOPE, "NameScope", Type::NAMESCOPE);
}
