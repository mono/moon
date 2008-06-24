/*
 * visual.cpp
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>
#include <stdio.h>
#include <glib.h>

#include "visual.h"
#include "runtime.h"
#include "utils.h"

bool
Visual::InsideObject (cairo_t *cr, double x, double y)
{
	printf ("Visual subclass '%s' should implement ::InsideObject\n", GetTypeName ());
	return false;
}

void
visual_set_surface (Visual* visual, Surface* surface)
{
	if (visual)
		visual->SetSurface (surface);
}

TimeManager*
Visual::GetTimeManager ()
{
	Surface *surface = GetSurface ();
	return surface ? surface->GetTimeManager() : NULL;
}


VisualCollection::VisualCollection ()
{
	z_sorted = g_ptr_array_new ();
}

VisualCollection::~VisualCollection ()
{
	Collection::Node *n = (Collection::Node *) list->First ();
	
	g_ptr_array_free (z_sorted, true);
	
	while (n) {
		((UIElement *) n->obj)->SetVisualParent (NULL);
		
		n = (Collection::Node *) n->next;
	}
}

static int
UIElementZIndexComparer (gconstpointer ui1, gconstpointer ui2)
{
	int z1 = (*((UIElement **) ui1))->GetValue (UIElement::ZIndexProperty)->AsInt32 ();
	int z2 = (*((UIElement **) ui2))->GetValue (UIElement::ZIndexProperty)->AsInt32 ();
	
	return z1 - z2;
}

void
VisualCollection::ResortByZIndex ()
{
	Collection::Node *node;
	guint i = 0;
	
	if (z_sorted->len < 1)
		return;
	
	node = (Collection::Node *) list->First ();
	while (node) {
		z_sorted->pdata[i++] = node->obj;
		node = (Collection::Node *) node->next;
	}
	
	g_ptr_array_sort (z_sorted, UIElementZIndexComparer);
}

int
VisualCollection::Add (DependencyObject *data)
{
	UIElement *item = (UIElement *) data;
	
	int n = Collection::Add (item);

	if (n != -1)
		g_ptr_array_insert_sorted (z_sorted, UIElementZIndexComparer, item);
	
	return n;
}

DependencyObject *
VisualCollection::SetVal (int index, DependencyObject *data)
{
	UIElement *item = (UIElement *) data;
	
	UIElement *old = (UIElement *) Collection::SetVal (index, item);
	
	if (old) {
		g_ptr_array_remove (z_sorted, old);
		g_ptr_array_insert_sorted (z_sorted, UIElementZIndexComparer, item);
	}

	return old;
}

bool
VisualCollection::Insert (int index, DependencyObject *data)
{
	UIElement *item = (UIElement *) data;
	
	if (!Collection::Insert (index, item))
		return false;
	
	g_ptr_array_set_size (z_sorted, z_sorted->len + 1);
	ResortByZIndex ();
	
	return true;
}

bool
VisualCollection::Remove (DependencyObject *data)
{
	if (!list->Find (CollectionNodeFinder, data))
		return false;

	UIElement *item = (UIElement *) data;

	if (Collection::Remove (item)) {
		g_ptr_array_remove (z_sorted, item);
		return true;
	}
	
	return false;
}

bool
VisualCollection::RemoveAt (int index)
{
	Collection::Node *n = (Collection::Node *) list->Index (index);
	if (n == NULL)
		return false;
	
	UIElement *item = (UIElement *) n->obj;

	if (Collection::RemoveAt (index)) {
		g_ptr_array_remove (z_sorted, item);
		return true;
	}
	
	return false;
}

void
VisualCollection::Clear ()
{
	Collection::Clear ();
	g_ptr_array_set_size (z_sorted, 0);
}


VisualCollection *
visual_collection_new (void)
{
	return new VisualCollection ();
}
