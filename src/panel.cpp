/*
 * panel.cpp
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include <gtk/gtk.h>

#include "panel.h"
#include "brush.h"
#include "collection.h"

VisualCollection*
Panel::GetChildren ()
{
	return GetValue (Panel::ChildrenProperty)->AsVisualCollection();
}

void
Panel::SetChildren (VisualCollection *col)
{
	SetValue (Panel::ChildrenProperty, col);
}

void 
panel_child_add (Panel *panel, UIElement *child)
{
	panel->GetChildren()->Add (child);
}

Brush *
panel_get_background (Panel *panel)
{
	Value *value = panel->GetValue (Panel::BackgroundProperty);
	
	return value ? (Brush *) value->AsBrush () : NULL;
}

Panel*
panel_new (void)
{
	return new Panel ();
}

Panel::Panel ()
{
	this->SetValue (Panel::ChildrenProperty, Value (new VisualCollection ()));
	background = NULL;
}

Panel::~Panel ()
{
	if (background != NULL) {
		background->Detach (NULL, this);
		background->unref ();
	}
}

//
// Intercept any changes to the children property and mirror that into our
// own variable
//
void
Panel::OnPropertyChanged (DependencyProperty *prop)
{
	FrameworkElement::OnPropertyChanged (prop);

	if (prop == Panel::ChildrenProperty) {
		VisualCollection *newcol = GetValue (prop)->AsVisualCollection();
		
		if (newcol) {
			if (newcol->closure)
				printf ("Warning we attached a property that was already attached\n");
			newcol->closure = this;
		}
	} else if (prop == BackgroundProperty) {
		if (background != NULL) {
			background->Detach (NULL, this);
			background->unref ();
		}
		
		if ((background = panel_get_background (this)) != NULL) {
			background->Attach (NULL, this);
			background->ref ();
		}
		
		Invalidate ();
	}
}

void
Panel::OnSubPropertyChanged (DependencyProperty *prop, DependencyProperty *subprop)
{
	if (prop == Panel::BackgroundProperty) {
		Invalidate ();
	}
	else {
		FrameworkElement::OnSubPropertyChanged (prop, subprop);
	}
}

void
Panel::OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, DependencyProperty *prop)
{
	// if a child changes its ZIndex property we need to resort our Children
	if (prop == UIElement::ZIndexProperty) {
		GetChildren()->ResortByZIndex ();
	}
}


void
Panel::OnLoaded ()
{
	VisualCollection *children = GetChildren ();
	Collection::Node *cn;

	cn = (Collection::Node *) children->list->First ();
	for ( ; cn != NULL; cn = (Collection::Node *) cn->Next ()) {
		UIElement *item = (UIElement *) cn->obj;

		item->OnLoaded ();
	}

	FrameworkElement::OnLoaded ();
}


DependencyProperty* Panel::ChildrenProperty;
DependencyProperty* Panel::BackgroundProperty;

void 
panel_init (void)
{
	Panel::ChildrenProperty = DependencyObject::Register (Type::PANEL, "Children", Type::VISUAL_COLLECTION);
	Panel::BackgroundProperty = DependencyObject::Register (Type::PANEL, "Background", Type::BRUSH);
}

