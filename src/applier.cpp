/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include "applier.h"

typedef struct {
	DependencyObject *object;
	GList *properties_list;
} object_indexer;

typedef struct {
	DependencyProperty *property;
	GList *values_list;
} property_indexer;

typedef struct {
	int precedence;
	Value *v;
} value_indexer;

Applier::Applier ()
{
	readonly = false;
	objects = NULL;
}

Applier::~Applier ()
{
	if (objects) {
		readonly = true;
		Flush ();
	}
}

static gint
property_indexer_compare_func (property_indexer *a, property_indexer *b)
{
	if (a->property == b->property)
		return 0;
	else
		return -1;
}

static gint
value_indexer_compare_func (value_indexer *a, value_indexer *b)
{
	if (a->precedence < b->precedence)
		return -1;
	else if (a->precedence > b->precedence)
		return 1;
	else
		return 0;
}

void
Applier::AddPropertyChange (DependencyObject *object, DependencyProperty *property, Value *v, int precedence)
{
	if (readonly) {
		g_warning ("Applier::AddPropertyChange is being called during shutdown");
		delete v;
		return;
	}

	if (precedence == APPLIER_PRECEDENCE_INSTANT) {
		object->SetValue (property, *v);
		delete v;
		return;
	}

	value_indexer *v_indexer = NULL;
	property_indexer *p_indexer = NULL;
	object_indexer *o_indexer = NULL;

	if (!objects)
		objects = g_hash_table_new (g_direct_hash, g_direct_equal);
	else
		o_indexer = (object_indexer *) g_hash_table_lookup (objects, object);

	if (o_indexer == NULL) {
		o_indexer = g_new (object_indexer, 1);
		o_indexer->object = object;
		object->ref ();
		o_indexer->properties_list = NULL;
		g_hash_table_insert (objects, object, o_indexer);
	}

	GList *list_item = g_list_find_custom (o_indexer->properties_list, property, (GCompareFunc) property_indexer_compare_func);

	if (list_item != NULL)
		p_indexer = (property_indexer *) list_item->data;

	if (p_indexer == NULL) {
		p_indexer = g_new (property_indexer, 1);
		p_indexer->property = property;
		p_indexer->values_list = NULL;
		o_indexer->properties_list = g_list_append (o_indexer->properties_list, p_indexer);
	}

	v_indexer = g_new (value_indexer, 1);
	v_indexer->precedence = precedence;
	v_indexer->v = v;

	p_indexer->values_list = g_list_insert_sorted (p_indexer->values_list, v_indexer, (GCompareFunc) value_indexer_compare_func);
}

static void
destroy_value_func (value_indexer *v_indexer, gpointer unused)
{
	g_return_if_fail (v_indexer != NULL);
	g_return_if_fail (v_indexer->v != NULL);

	delete v_indexer->v;
	v_indexer->v = NULL;
	g_free (v_indexer);
}

static void
destroy_property_func (property_indexer *p_indexer, gpointer unused)
{
	g_return_if_fail (p_indexer != NULL);
	g_return_if_fail (p_indexer->values_list != NULL);

	g_list_foreach (p_indexer->values_list, (GFunc) destroy_value_func, NULL);
	g_list_free (p_indexer->values_list);
	p_indexer->property = NULL;
	p_indexer->values_list = NULL;
	g_free (p_indexer);
}

static void
destroy_object_func (DependencyObject *object, object_indexer *o_indexer, gpointer unused)
{
	g_return_if_fail (object != NULL);
	g_return_if_fail (o_indexer != NULL);

	g_list_foreach (o_indexer->properties_list, (GFunc) destroy_property_func, NULL);
	g_list_free (o_indexer->properties_list);
	object->unref ();
	g_free (o_indexer);
}

static void
apply_property_func (property_indexer *p_indexer, DependencyObject *object)
{
	g_return_if_fail (p_indexer->property != NULL);
	g_return_if_fail (p_indexer->values_list != NULL);

	value_indexer *v_indexer = (value_indexer *) p_indexer->values_list->data;
	object->SetValue (p_indexer->property, *v_indexer->v);
}

static void
apply_object_func (DependencyObject *object, object_indexer *o_indexer, gpointer unused)
{
	g_return_if_fail (o_indexer != NULL);

	g_list_foreach (o_indexer->properties_list, (GFunc) apply_property_func, object);
}

void 
Applier::Apply ()
{
	if (objects)
		g_hash_table_foreach (objects, (GHFunc) apply_object_func, NULL);
}

void
Applier::Flush ()
{
	if (objects) {
		g_hash_table_foreach (objects, (GHFunc) destroy_object_func, NULL);
		g_hash_table_destroy (objects);
		objects = NULL;
	}
}

