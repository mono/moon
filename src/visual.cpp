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

TimeManager *
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
	UIElement *item;
	
	for (guint i = 0; i < array->len; i++) {
		item = ((Value *) array->pdata[i])->AsUIElement ();
		item->SetVisualParent (NULL);
	}
	
	g_ptr_array_free (z_sorted, true);
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
	if (array->len <= 1)
		return;
	
	g_ptr_array_set_size (z_sorted, array->len);
	for (guint i = 0; i < array->len; i++)
		z_sorted->pdata[i] = ((Value *) array->pdata[i])->AsUIElement ();
	
	g_ptr_array_sort (z_sorted, UIElementZIndexComparer);
}

void
VisualCollection::AddedToCollection (Value *value)
{
	UIElement *item = value->AsUIElement ();
	
	g_ptr_array_insert_sorted (z_sorted, UIElementZIndexComparer, item);
	
	DependencyObjectCollection::AddedToCollection (value);
}

void
VisualCollection::RemovedFromCollection (Value *value)
{
	UIElement *item = value->AsUIElement ();
	
	g_ptr_array_remove (z_sorted, item);
	
	DependencyObjectCollection::RemovedFromCollection (value);
}

bool
VisualCollection::Insert (int index, Value value)
{
	if (!Collection::Insert (index, value))
		return false;
	
	// FIXME: If z_sorted was an array of structs containing both
	// the item *and* the array index, our comparer could take
	// that into consideration when sorting and so we'd never have
	// to completely re-sort on Insert()
	g_ptr_array_set_size (z_sorted, z_sorted->len + 1);
	ResortByZIndex ();
	
	return true;
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
