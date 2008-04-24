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

#include "geometry.h"
#include "panel.h"
#include "brush.h"
#include "color.h"
#include "math.h"
#include "collection.h"
#include "runtime.h"
#include "dirty.h"


DependencyProperty *Panel::BackgroundProperty;
DependencyProperty *Panel::ChildrenProperty;

Panel::Panel ()
{
	SetValue (Panel::ChildrenProperty, Value::CreateUnref (new VisualCollection ()));
	mouse_over = NULL;
}

Panel::~Panel ()
{
}

Brush *
Panel::GetBackground ()
{
	Value *value = GetValue (Panel::BackgroundProperty);
	
	return value ? value->AsBrush () : NULL;
}

void
Panel::SetBackground (Brush *background)
{
	SetValue (Panel::BackgroundProperty, Value (background));
}

VisualCollection *
Panel::GetChildren ()
{
	Value *value = GetValue (Panel::ChildrenProperty);
	
	return value ? value->AsVisualCollection () : NULL;
}

void
Panel::SetChildren (VisualCollection *children)
{
	SetValue (Panel::ChildrenProperty, children);
}

void
Panel::SetSurface (Surface *s)
{
	FrameworkElement::SetSurface (s);

	VisualCollection *children = GetChildren ();
	Collection::Node *cn;

	if (children != NULL) {
		cn = (Collection::Node *) children->list->First ();
		for ( ; cn != NULL; cn = (Collection::Node *) cn->next) {
			UIElement *item = (UIElement *) cn->obj;
			item->SetSurface (s);
		}
	}
}

void
Panel::AddChild (UIElement *item)
{
	GetChildren ()->Add (item);
}

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
Panel::ComputeBounds ()
{
	VisualCollection *children = GetChildren ();
	Collection::Node *cn;
	bool first = true;

#if DEBUG_BOUNDS
	levelb += 4;
	space (levelb);
	printf ("Panel: Enter ComputeBounds (%s)\n", GetName());
#endif
	if (children != NULL) {
		cn = (Collection::Node *) children->list->First ();
		for ( ; cn != NULL; cn = (Collection::Node *) cn->next) {
			UIElement *item = (UIElement *) cn->obj;

			// if the item isn't drawn, skip it
			if (!item->GetRenderVisible ())
				continue;

			Rect r = item->GetSubtreeBounds ();
				
			r = IntersectBoundsWithClipPath (r, true);
#if DEBUG_BOUNDS
			space (levelb + 4);
			printf ("Item (%s, %s) bounds %g %g %g %g\n", 
				dependency_object_get_name (item), item->GetTypeName(),r.x, r.y, r.w, r.h);
#endif
			if (first) {
				bounds_with_children = r;
				first = false;
			}
			else {
				bounds_with_children = bounds_with_children.Union (r);
			}
		}
		bounds_with_children = IntersectBoundsWithClipPath (bounds_with_children, true);
	} else {
		bounds_with_children = Rect (0,0,0,0);
	}

	Value *value = GetValue (Panel::BackgroundProperty);
	if (value) {
		FrameworkElement::ComputeBounds ();
		bounds_with_children = bounds_with_children.Union (bounds);
	} else
		bounds = Rect (0,0,0,0);

#if DEBUG_BOUNDS
	space (levelb);
	printf ("Panel: Leave ComputeBounds (%g %g %g %g)\n", bounds.x, bounds.y, bounds.w, bounds.h);
	space (levelb);
	printf ("Panel: Leave ComputeBounds (%g %g %g %g)\n", bounds_with_children.x, bounds_with_children.y, bounds_with_children.w, bounds_with_children.h);
	levelb -= 4;
#endif
}

//#define DEBUG_INVALIDATE 1

void
Panel::UpdateTotalRenderVisibility ()
{
	FrameworkElement::UpdateTotalRenderVisibility ();
}

void
Panel::UpdateTotalHitTestVisibility ()
{
#if 1
	// this really shouldn't need to be here, but our dirty code is broken
	VisualCollection *children = GetChildren ();
	for (guint i = 0; i < children->z_sorted->len; i++) {
		UIElement *item = (UIElement *) children->z_sorted->pdata[i];
		item->UpdateTotalHitTestVisibility ();
	}
#endif

	FrameworkElement::UpdateTotalHitTestVisibility ();
}

bool
Panel::UseBackToFront ()
{
	return GetChildren ()->list->Length() < 25;
}

void
Panel::Render (cairo_t *cr, Region *region)
{
	Brush *background;
	
	cairo_set_matrix (cr, &absolute_xform);
	
	if ((background = GetBackground ())) {
		double fheight = GetHeight ();
		double fwidth = GetWidth ();
		
		if (fwidth > 0 && fheight > 0) {
			background->SetupBrush (cr, this);
			
			// FIXME - UIElement::Opacity may play a role here
			cairo_new_path (cr);
			cairo_rectangle (cr, 0, 0, fwidth, fheight);
			cairo_fill (cr);
		}
	}
}

void
Panel::PostRender (cairo_t *cr, Region *region, bool front_to_back)
{
	// if we didn't render front to back, then render the children here
	if (!front_to_back || !UseBackToFront ()) {
		RenderChildren (cr, region);
	}

	UIElement::PostRender (cr, region, front_to_back);
}


void
Panel::RenderChildren (cairo_t *cr, Region *parent_region)
{
	VisualCollection *children = GetChildren ();

	Region *clipped_region = new Region (bounds_with_children);
	clipped_region->Intersect (parent_region);

	cairo_identity_matrix (cr);
	for (guint i = 0; i < children->z_sorted->len; i++) {
		UIElement *item = (UIElement *) children->z_sorted->pdata[i];
		
		Region *region = new Region (item->GetSubtreeBounds());
		region->Intersect (clipped_region);

		if (!item->GetRenderVisible ()
		    || region->IsEmpty()) {
#ifdef DEBUG_INVALIDATE
			printf ("skipping offscreen object %s: %p (%s)\n", item->GetName (), item, item->GetTypeName());
#endif
			delete region;
			continue;
		}
		
#if CAIRO_CLIP
#if TIME_CLIP
		STARTTIMER(clip, "cairo clip setup");
#endif
		cairo_save (cr);
		
		//printf ("Clipping to %g %g %g %g\n", inter.x, inter.y, inter.w, inter.h);
		// at the very least we need to clip based on the expose area.
		// there's also a UIElement::ClipProperty
		
		region->Draw (cr);
		cairo_clip (cr);
#if TIME_CLIP
		ENDTIMER(clip, "cairo clip setup");
#endif
#endif
		// 			space (levelb);
		// 			printf ("%p %s (%s), bounds = %g %g %g %g, inter = %g %g %g %g\n",
		// 				item, item->GetTypeName(), item->GetName(),
		// 				item->GetBounds().x, item->GetBounds().y, item->GetBounds().w, item->GetBounds().h,
		// 				inter.x, inter.y, inter.w, inter.h);
		
		item->DoRender (cr, region);

#if CAIRO_CLIP
#if TIME_CLIP
		STARTTIMER(endclip, "cairo clip teardown");
#endif			
		cairo_restore (cr);
		
#if TIME_CLIP
		ENDTIMER(endclip, "cairo clip teardown");
#endif
#endif
		delete region;
	}

	delete clipped_region;
}

void
Panel::FrontToBack (Region *surface_region, List *render_list)
{
	double local_opacity = GetValue (OpacityProperty)->AsDouble();

	if (surface_region->RectIn (bounds_with_children.RoundOut()) == GDK_OVERLAP_RECTANGLE_OUT)
		return;

	if (!GetRenderVisible ()
	    || IS_INVISIBLE (local_opacity))
		return;

	if (!UseBackToFront ()) {
		Region *self_region = new Region (surface_region);
		self_region->Intersect (bounds_with_children.RoundOut());
		// we need to include our children in this one, since
		// we'll be rendering them in the PostRender method.
		if (!self_region->IsEmpty())
			render_list->Prepend (new RenderNode (this, self_region, !self_region->IsEmpty(),
							      UIElement::CallPreRender, UIElement::CallPostRender));
		// don't remove the region from surface_region because
		// there are likely holes in it
		return;
	}

	Region *region;
	bool delete_region;
	bool can_subtract_self;
	Value *value;
	
	if ((GetValue (UIElement::ClipProperty) == NULL)
	    && (!(value = GetValue (UIElement::OpacityMaskProperty)) || value->AsBrush () == NULL)
	    && !IS_TRANSLUCENT (GetValue (UIElement::OpacityProperty)->AsDouble ())) {
		region = surface_region;
		delete_region = false;
		can_subtract_self = true;
	}
	else {
		region = new Region (surface_region);
		delete_region = true;
		can_subtract_self = false;
	}

	RenderNode *panel_cleanup_node = new RenderNode (this, NULL, false, NULL, UIElement::CallPostRender);
	
	render_list->Prepend (panel_cleanup_node);

	VisualCollection *children = GetChildren ();
	for (guint i = children->z_sorted->len; i > 0; i--) {
		UIElement *item = (UIElement *) children->z_sorted->pdata[i - 1];

		item->FrontToBack (region, render_list);
	}

	Region *self_region = new Region (region);
	self_region->Intersect (GetRenderBounds().RoundOut ()); // note the RoundOut

	if (self_region->IsEmpty() && render_list->First() == panel_cleanup_node) {
		/* we don't intersect the surface region, and none of
		   our children did either, remove the cleanup node */
		render_list->Remove (render_list->First());
		delete self_region;
		if (delete_region)
			delete region;
		return;
	}

	render_list->Prepend (new RenderNode (this, self_region, !self_region->IsEmpty(), UIElement::CallPreRender, NULL));

	if (!self_region->IsEmpty()) {
		bool subtract = ((absolute_xform.yx == 0 && absolute_xform.xy == 0) /* no skew/rotation */
				 && can_subtract_self);

		if (subtract) {
			Brush *background = GetBackground ();
			
			if (background)
				subtract = background->IsOpaque ();
			else
				subtract = false;
		}

 		if (subtract)
			region->Subtract (bounds);
	}

	if (delete_region)
		delete region;
}

void
Panel::CacheInvalidateHint (void)
{
	VisualCollection *children = GetChildren ();
	Collection::Node *cn;

	if (children != NULL) {
		cn = (Collection::Node *) children->list->First ();
		for ( ; cn != NULL; cn = (Collection::Node *) cn->next) {
			UIElement *item = (UIElement *) cn->obj;
			item->CacheInvalidateHint ();
		}
	}
}

bool
Panel::InsideObject (cairo_t *cr, double x, double y)
{
	bool is_inside_clip = InsideClip (cr, x, y);
	if (!is_inside_clip)
		return false;
	
	/* if we have explicitly set width/height, we check them */
	if (FrameworkElement::InsideObject (cr, x, y)) {
		/* we're inside, check if we're actually painting any background,
		   or, we're just transparent due to no painting. */
		if (GetBackground ())
			return true;
	}
	
	UIElement *mouseover = FindMouseOver (cr, x, y);

	return mouseover != NULL;
}

bool
Panel::CheckOver (cairo_t *cr, UIElement *item, double x, double y)
{
	// if the item isn't visible, it's really easy
	if (!item->GetRenderVisible ())
		return false;

	// if the item doesn't take part in hit testing, it's also easy
	if (!item->GetHitTestVisible ())
		return false;

	// first a quick bounds check
	if (!item->GetSubtreeBounds().PointInside (x, y))
		return false;

	// then, if that passes, a more tailored shape check
	return item->InsideObject (cr, x, y);
}

UIElement *
Panel::FindMouseOver (cairo_t *cr, double x, double y)
{
	VisualCollection *children = GetChildren ();
	
	// Walk the list in reverse order, since it's sorted in ascending z-index order
	//
	for (guint i = children->z_sorted->len; i > 0; i--) {
		UIElement *item = (UIElement *) children->z_sorted->pdata[i - 1];

		if (CheckOver (cr, item, x, y)) {
			return item;
		}
	}

	return NULL;
}

void
Panel::HitTest (cairo_t *cr, double x, double y, List *uielement_list)
{
	/* in the interests of not calling FindMouseOver twice, this method
	   cut & pastes from the bodies of both Panel::InsideObject and
	   Panel::FindMouseOver */

	UIElement *mouseover = FindMouseOver (cr, x, y);

	if (mouseover) {
		uielement_list->Prepend (new UIElementNode (this));
		mouseover->HitTest (cr, x, y, uielement_list);
	}
	else {
		bool is_inside_clip = InsideClip (cr, x, y);
		if (!is_inside_clip)
			return;
	
		/* if we have explicitly set width/height, we check them */
		if (FrameworkElement::InsideObject (cr, x, y)) {
			/* we're inside, check if we're actually painting any background,
			   or, we're just transparent due to no painting. */
			if (GetBackground ())
				uielement_list->Prepend (new UIElementNode (this));
		}
	}
}

//
// Intercept any changes to the children property and mirror that into our
// own variable
//
void
Panel::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property->type != Type::PANEL) {
		FrameworkElement::OnPropertyChanged (args);
		return;
	}

	if (args->property == Panel::BackgroundProperty)
		Invalidate ();

	if (args->property == Panel::ChildrenProperty) {
		Collection::Node *n;
		if (args->old_value) {
			for (n = (Collection::Node *) args->old_value->AsCollection()->list->First (); n; n = (Collection::Node *) n->next) 
				ChildRemoved ((Visual*)n->obj);
		}

		if (args->new_value) {
			for (n = (Collection::Node *) args->new_value->AsCollection()->list->First (); n; n = (Collection::Node *) n->next) 
				ChildAdded ((Visual*)n->obj);
		}

		UpdateBounds();
	}

	NotifyListenersOfPropertyChange (args);
}

void
Panel::ChildAdded (Visual *child)
{
	UIElement *item = (UIElement *) child;

	item->SetVisualParent (this);
	item->UpdateTransform ();
	item->UpdateTotalRenderVisibility ();
	item->UpdateTotalHitTestVisibility ();
	item->Invalidate ();
}

void
Panel::ChildRemoved (Visual *child)
{
	UIElement *item = (UIElement *) child;

	Invalidate (item->GetSubtreeBounds());

	item->CacheInvalidateHint ();
	item->SetVisualParent (NULL);
	item->flags &= ~UIElement::IS_LOADED;
}

void
Panel::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	if (prop == Panel::BackgroundProperty) {
		Invalidate ();
	}
	else {
		FrameworkElement::OnSubPropertyChanged (prop, obj, subobj_args);
	}
}

void
Panel::OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, PropertyChangedEventArgs *element_args)
{
	if (col == GetValue (Panel::ChildrenProperty)->AsCollection()) {
		switch (type) {
		case CollectionChangeTypeItemAdded:
			ChildAdded ((Visual*)obj);
			UpdateBounds (true);
			break;
		case CollectionChangeTypeItemRemoved:
			ChildRemoved ((Visual*)obj);
			UpdateBounds (true);
			break;
		case CollectionChangeTypeChanged:
			Collection::Node *n;
			for (n = (Collection::Node *) col->list->First (); n; n = (Collection::Node *) n->next) 
				ChildAdded ((Visual*)n->obj);
			break;
		case CollectionChangeTypeChanging: {
			Collection::Node *n;
			for (n = (Collection::Node *) col->list->First (); n; n = (Collection::Node *) n->next) 
				ChildRemoved ((Visual*)n->obj);
			break;
		}
		case CollectionChangeTypeItemChanged:
			// if a child changes its ZIndex property we need to resort our Children
			if (element_args->property == UIElement::ZIndexProperty) {
				// FIXME: it would probably be faster to remove the
				// changed item and then re-add it using
				// g_ptr_array_insert_sorted() because
				// g_ptr_array_sort() uses QuickSort which has poor
				// performance on nearly-sorted input.
				GetChildren()->ResortByZIndex ();
				((UIElement*)obj)->Invalidate();
			}
			break;
		}
	}
}

void
Panel::OnLoaded ()
{
	VisualCollection *children = GetChildren ();
	Collection::Node *cn;

	cn = (Collection::Node *) children->list->First ();
	for ( ; cn != NULL; cn = (Collection::Node *) cn->next) {
		UIElement *item = (UIElement *) cn->obj;

		item->OnLoaded ();
	}

	FrameworkElement::OnLoaded ();
}



Panel *
panel_new (void)
{
	return new Panel ();
}

void
panel_set_background (Panel *panel, Brush *background)
{
	panel->SetBackground (background);
}

Brush *
panel_get_background (Panel *panel)
{
	return panel->GetBackground ();
}

void
panel_set_children (Panel *panel, VisualCollection *children)
{
	panel->SetChildren (children);
}

VisualCollection *
panel_get_children (Panel *panel)
{
	return panel->GetChildren ();
}

void 
panel_init (void)
{
	Panel::ChildrenProperty = DependencyObject::Register (Type::PANEL, "Children", Type::VISUAL_COLLECTION);
	Panel::BackgroundProperty = DependencyObject::Register (Type::PANEL, "Background", Type::BRUSH);
}

