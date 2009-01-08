/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * stackpanel.cpp:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>
#include "rect.h"
#include "runtime.h"
#include "transform.h"
#include "stackpanel.h"

#define DEBUG_BOUNDS 0
#define CAIRO_CLIP 0

#if DEBUG_BOUNDS
static void space (int n)
{
	for (int i = 0; i < n; i++)
		putchar (' ');
}
static int levelb = 0;
#endif


void
StackPanel::GetTransformFor (UIElement *item, cairo_matrix_t *result)
{
	UIElementCollection *children = GetChildren ();
	int idx = children->IndexOf (Value::CreateUnrefPtr (item));
	int top = 0;
	for (int i = 0; i < idx; i ++) {
		UIElement *child = children->GetValueAt(i)->AsUIElement ();
		child->Measure (Size (INFINITY, INFINITY));
		Size size = child->GetDesiredSize ();
		top += size.height;
	}

	cairo_matrix_init_identity (result);
	cairo_matrix_init_translate (result, 0, top);
}

void
StackPanel::OnLoaded ()
{
	UIElement::OnLoaded ();

	if (GetSurface ()) {
		// queue a resort based on ZIndex
		GetSurface ()->AddDirtyElement (this, DirtyChildrenZIndices);
	}
}

void
StackPanel::ElementAdded (UIElement *item)
{
	Panel::ElementAdded (item);
	if (GetSurface ()) {
		// queue a resort based on ZIndex
		GetSurface ()->AddDirtyElement (this, DirtyChildrenZIndices);
	}
}

void
StackPanel::ElementRemoved (UIElement *item)
{
	Panel::ElementRemoved (item);
	
	if (GetSurface ()) {
		// queue a resort based on ZIndex
		GetSurface ()->AddDirtyElement (this, DirtyChildrenZIndices);
	}
}
