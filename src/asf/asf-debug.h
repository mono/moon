/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * asf-debug.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef _ASF_DEBUG_MOONLIGHT_H
#define _ASF_DEBUG_MOONLIGHT_H

#if DEBUG

#include <glib.h>
#include <stdio.h>

class ObjectTracker {
public:
	ObjectTracker (const char* tn)
	{
		this->tn = tn;
		id = GetNextID (tn);
		Add ();	
		printf ("ObjectTracer::ObjectTracker (%s): id = %i\n", tn, id);
	}
	
	~ObjectTracker ()
	{
		printf ("ObjectTracker::~ObjectTracker (%s): id = %i\n", tn, id);
		Remove ();
	}
	
	
	static bool PrintStatus (const char* tn)
	{
		GHashTable* list = NULL;
		
		if (current_objects != NULL) {
			list = (GHashTable*)g_hash_table_lookup (current_objects, tn);
		}
		
		if (list != NULL) {
			guint count = g_hash_table_size (list);
			if (count == 0) {
				printf ("ObjectTracking::PrintStatus (%s): No unfreed objects.\n", tn);
			} else {
				printf ("ObjectTracking::PrintStatus (%s): %u unfreed objects:\n", tn, count);
				int max_items = 10;
				g_hash_table_foreach (list, print, &max_items);
			}
			return count == 0;
		} else {
			printf ("ObjectTracking::PrintStatus (%s): No objects tracked.\n", tn);
		}
		
		return true;
	}
	
	static void print (gpointer key, gpointer value, gpointer user_data)
	{
		ObjectTracker* obj = (ObjectTracker*) value;
		(*(int*)user_data)--;
		int items_left = *(int*)user_data;
		if (items_left >= 0) {
			printf (" ObjectTracking::Print (%s): #%i is still alive.\n", obj->tn, obj->id);
		}
	}
	
private:
	int GetNextID (const char* tn)
	{
		int result = 0;
		
		if (current_ids == NULL)
			current_ids = g_hash_table_new (g_str_hash, g_str_equal);
			
		gpointer idp = NULL;
		int idn = 0;
		
		if (g_hash_table_lookup_extended (current_ids, tn, NULL, &idp)) {
			idn = GPOINTER_TO_INT (idp);
		}
		
		result = ++idn;
		
		printf ("ObjectTracer::GetNextID (%s): %i\n", tn, result);
		
		g_hash_table_insert (current_ids, (gpointer) tn, GINT_TO_POINTER (idn));
		
		return result;
	}
	
	void Add ()
	{
		GHashTable* list = NULL;
		
		if (current_objects == NULL)
			current_objects = g_hash_table_new (g_str_hash, g_str_equal);
			
		list = (GHashTable*) g_hash_table_lookup (current_objects, tn);
		
		if (list == NULL) {
			list = g_hash_table_new (g_direct_hash, g_direct_equal);
			g_hash_table_insert (current_objects, (gpointer) tn, list);
		}
		g_hash_table_insert (list, GINT_TO_POINTER (id), this);
		
	}
	void Remove ()
	{
		GHashTable* list = NULL;
		
		if (current_objects == NULL) {
			printf ("ObjectTracker::Remove (): the hashtable where I should be is NULL.\n");
			return;
		}
		
		list = (GHashTable*) g_hash_table_lookup (current_objects, tn);
		
		if (list == NULL) {
			printf ("ObjectTracker::Remove (): the list where the I should be is NULL.\n");
			return;
		}
		
		g_hash_table_remove (list, GINT_TO_POINTER (id));
	}
	
	// No automagic C++ stuff.
	ObjectTracker ();
	ObjectTracker (const ObjectTracker&);
	ObjectTracker operator= (const ObjectTracker&);
	
	int id;
	const char* tn;
	static GHashTable* current_ids;
	static GHashTable* current_objects;
};

#endif

#endif
