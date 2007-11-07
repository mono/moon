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

VisualCollection *
Panel::GetChildren ()
{
	Value* children = GetValue (Panel::ChildrenProperty);
	if (children)
		return children->AsVisualCollection ();
	return NULL;
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

Panel *
panel_new (void)
{
	return new Panel ();
}

Panel::Panel ()
{
	this->SetValue (Panel::ChildrenProperty, Value::CreateUnref (new VisualCollection ()));
	background = NULL;
	mouse_over = NULL;
}

Panel::~Panel ()
{
	if (background != NULL) {
		background->Detach (NULL, this);
		background->unref ();
	}
}

#define DEBUG_BOUNDS 0
#define CAIRO_CLIP 0

static void space (int n)
{
	for (int i = 0; i < n; i++)
		putchar (' ');
}
static int levelb = 0;

void
Panel::ComputeBounds ()
{
	VisualCollection *children = GetChildren ();
	Collection::Node *cn;
	bool first = true;

#if DEBUG_BOUNDS
	levelb += 4;
	space (levelb);
	printf ("Panel: Enter ComputeBounds\n");
#endif
	if (children != NULL) {
		cn = (Collection::Node *) children->list->First ();
		for ( ; cn != NULL; cn = (Collection::Node *) cn->next) {
			UIElement *item = (UIElement *) cn->obj;

			// if the item doesn't take part in layout
			// calculations, skip it
			if (!item->GetVisible ())
				continue;

			Rect r = item->GetBounds ();
			r = IntersectBoundsWithClipPath (r, true);
#if DEBUG_BOUNDS
			space (levelb + 4);
			printf ("Item (%s, %s) bounds %g %g %g %g\n", 
				dependency_object_get_name (item), item->GetTypeName(),r.x, r.y, r.w, r.h);
#endif
			if (first) {
				bounds = r;
				first = false;
			}
			else {
				bounds = bounds.Union (r);
			}
		}
	}

	// If we found nothing.
	double x1, x2, y1, y2;
	
	x1 = y1 = 0.0;
	x2 = framework_element_get_width (this);
	y2 = framework_element_get_height (this);

	Value *value = GetValue (Panel::BackgroundProperty);
	if (value && x2 != 0.0 && y2 != 0.0) {
		Rect fw_rect = bounding_rect_for_transformed_rect (&absolute_xform,
								   IntersectBoundsWithClipPath (Rect (x1,y1,x2,y2), false));


		if (first)
			bounds = fw_rect;
		else
			bounds = bounds.Union (fw_rect);
	}

	/* standard "grow the rectangle by enough to cover our
	   asses because of cairo's floating point rendering"
	   thing */
// nop-op	bounds.GrowBy (1);

#if DEBUG_BOUNDS
	space (levelb);
	printf ("Panel: Leave ComputeBounds (%g %g %g %g)\n", bounds.x, bounds.y, bounds.w, bounds.h);
	levelb -= 4;
#endif
}


static int level = 0;

//#define DEBUG_INVALIDATE 1
#define USE_STARTING_ELEMENT 1



int
Panel::FindStartingElement (Region *region)
{
#if USE_STARTING_ELEMENT
	VisualCollection *children = GetChildren ();
	Rect clip = region->ClipBox ().RoundOut ();
	
	gint i = children->z_sorted->len;
	if (i < 2)
		return -1;

	if (!clip.IntersectsWith (GetBounds().RoundOut())
	    || !GetVisible () 
	    || GetValue (UIElement::ClipProperty) != NULL
	    || uielement_get_opacity_mask (this) != NULL
	    || GetTotalOpacity () < .990
	    )
		return -1;

	for (i --; i >= 0; i --) {
		UIElement *item = (UIElement *) children->z_sorted->pdata[i];
		// if the exposed rectangle is completely within the bounds
		// of a child that has opacity == 1.0 and lacks an opacity
		// mask, we start rendering from there.
		if (item->GetTotalOpacity () > GetTotalOpacity ())
			g_warning ("Moonlight bug: child opacity greater than parent");

		if (item->GetVisible ()
		    && item->GetValue (UIElement::ClipProperty) == NULL
		    && (item->absolute_xform.yx == 0 && item->absolute_xform.xy == 0) /* no skew */
		    && item->GetTotalOpacity () >= .990
		    && uielement_get_opacity_mask (item) == NULL
		    && clip.IntersectsWith (item->GetBounds ().RoundOut ())
		    && region->RectIn (item->GetBounds().RoundOut()) == GDK_OVERLAP_RECTANGLE_IN
		    ) {
			// there are actually some more type
			// specific checks required.  we need
			// to fulsrther limit it to elements
			// which are truly rectangular to
			// begin with (images, panels,
			// mediaelements), and which aren't
			// rotated/skewed.  also, make sure
			// panel backgrounds are
			// non-translucent.
			Type::Kind type = item->GetObjectType();

			if (type == Type::PANEL || type == Type::CANVAS || type == Type::INKPRESENTER) {
				bool panel_works = false;
#if 1
				Brush *bg = panel_get_background ((Panel*)item);
				if (bg && bg->GetObjectType() == Type::SOLIDCOLORBRUSH) {
					Color *c = solid_color_brush_get_color ((SolidColorBrush*)bg);
					if (c && c->a == 1.0) {
						/* we're good */
						panel_works = true;
					}
				}
#endif

				if (!panel_works) {
					/* look one level down and see if
					** there's a child of this the child
					** panel that completely encompasses
					** the rectangle.
					*/
					if (-1 == ((Panel*)item)->FindStartingElement (region))
						continue;
				}
			}
			else if (type != Type::MEDIAELEMENT) {
				continue;
			}
			return i;
		}
	}
#endif

	return -1;
}

void
Panel::UpdateTotalOpacity ()
{
#if 1
	VisualCollection *children = GetChildren ();
	for (guint i = 0; i < children->z_sorted->len; i++) {
		UIElement *item = (UIElement *) children->z_sorted->pdata[i];
		item->UpdateTotalOpacity ();
	}
#endif

	add_dirty_element (this, DirtyOpacity);
}

void
Panel::Render (cairo_t *cr, Region *region)
{
	//if (!GetVisible () || GetTotalOpacity () <= 0.0)
	//	return;

	cairo_save (cr);  // for UIElement::ClipProperty

	cairo_set_matrix (cr, &absolute_xform);
	RenderClipPath (cr);

	Value *value = GetValue (Panel::BackgroundProperty);
	if (value) {
		double fwidth = framework_element_get_width (this);
		double fheight = framework_element_get_height (this);

		if (fwidth > 0 && fheight > 0){


			Brush *background = value->AsBrush ();
			background->SetupBrush (cr, this);

			// FIXME - UIElement::Opacity may play a role here
			cairo_rectangle (cr, 0, 0, fwidth, fheight);
			cairo_fill (cr);
		}
	}

	RenderChildren (cr, region);

	cairo_restore (cr);
}

void
Panel::RenderChildren (cairo_t *cr, Region *parent_region)
{

	VisualCollection *children = GetChildren ();

	Region *clipped_region = new Region (GetBounds ());
	clipped_region->Intersect (parent_region);
	gint start_element = FindStartingElement (clipped_region);
	
	if (start_element <= 0)
		start_element = 0;
	else
		printf ("start_element = %d\n", start_element);

	cairo_identity_matrix (cr);
	for (guint i = start_element; i < children->z_sorted->len; i++) {
		UIElement *item = (UIElement *) children->z_sorted->pdata[i];
		
		Region *region = new Region (item->GetBounds ());
		region->Intersect (clipped_region);

		if (!item->GetVisible() 
		    || item->GetTotalOpacity () == 0.0 
		    || gdk_region_empty (region->gdkregion)
		    ) {
#ifdef DEBUG_INVALIDATE
			printf ("skipping invisible object %s: %p (%s)\n", item->GetName (), item, item->GetTypeName());
#endif
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
		
		runtime_cairo_region (cr,region->gdkregion);
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
		

		//Type::Kind type = item->GetObjectType();
		//if (type == Type::PANEL || type == Type::CANVAS)
			item->DoRender (cr, region);
			//else {
			//cairo_save (cr);
			////Region zone = Region (region->ClipBox ());
			//runtime_cairo_region (cr,region->gdkregion);
			//cairo_clip (cr);
			//item->DoRender (cr, region);
			//cairo_restore (cr);
			//}
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

bool
Panel::InsideObject (cairo_t *cr, double x, double y)
{
	/* if we have explicitly set width/height, we check them */
	if (FrameworkElement::InsideObject (cr, x, y)) {
		/* we're inside, check if we're actually painting any background,
		   or, we're just transparent due to no painting. */
		if (panel_get_background (this) != NULL)
			return true;
	}

	bool is_inside_clip = InsideClip (cr, x, y);
	if (!is_inside_clip)
		return false;
	
	UIElement* mouseover = FindMouseOver (cr, x, y);
	
	return mouseover != NULL;
}

bool
Panel::CheckOver (cairo_t *cr, UIElement *item, double x, double y)
{
	// if the item isn't visible, it's really easy
	if (!item->GetVisible ())
		return false;

	// if the item doesn't take part in hit testing, it's also easy
	if (!item->GetHitTestVisible ())
		return false;

	// first a quick bounds check
	if (!item->GetBounds().PointInside (x, y))
		return false;

	// then, if that passes, a more tailored shape check
	if (!item->InsideObject (cr, x, y))
		return false;

	return true;
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
			//printf (" mouse_over = %s\n", item->GetName());
			return item;
		}
	}

	return NULL;
}

void
Panel::HandleMotion (cairo_t *cr, int state, double x, double y, MouseCursor *cursor)
{
	Surface *s = GetSurface ();
	UIElement *new_over;
	bool captured = false;

	if (s != NULL && s->GetCapturedElement () == this) {
		captured = true;
		new_over = this;
	} else {
		new_over = FindMouseOver (cr, x, y);
	}


	if (new_over != mouse_over) {
		if (mouse_over)
			mouse_over->Leave ();

		mouse_over = new_over;

		if (mouse_over)
			mouse_over->Enter (cr, state, x, y);
	}

	if (cursor && *cursor == MouseCursorDefault)
		*cursor = (MouseCursor)GetValue (UIElement::CursorProperty)->AsInt32();

	if (mouse_over)
		mouse_over->HandleMotion (cr, state, x, y, cursor);

	if (mouse_over || captured || InsideObject (cr, x, y))
		FrameworkElement::HandleMotion (cr, state, x, y, NULL);
}

void
Panel::HandleButtonPress (cairo_t *cr, int state, double x, double y)
{
	// not sure if this is correct, but we don't bother updating
	// the current mouse_over here (and along with that, emitting
	// enter/leave events).
	if (mouse_over) {
		if (mouse_over->GetBounds().PointInside (x, y)
		    && mouse_over->InsideObject (cr, x, y)) {

			mouse_over->HandleButtonPress (cr, state, x, y);
		}
	}

	FrameworkElement::HandleButtonPress (cr, state, x, y);
}

void
Panel::HandleButtonRelease (cairo_t *cr, int state, double x, double y)
{
	// not sure if this is correct, but we don't bother updating
	// the current mouse_over here (and along with that, emitting
	// enter/leave events).
	if (mouse_over) {
		if (mouse_over->GetBounds().PointInside (x, y)
		    && mouse_over->InsideObject (cr, x, y)) {

			mouse_over->HandleButtonRelease (cr, state, x, y);
		}
	}

	FrameworkElement::HandleButtonRelease (cr, state, x, y);
}

void
Panel::Enter (cairo_t *cr, int state, double x, double y)
{
	mouse_over = FindMouseOver (cr, x, y);

	if (mouse_over)
		mouse_over->Enter (cr, state, x, y);
	  
	FrameworkElement::Enter (cr, state, x, y);
}

void
Panel::Leave ()
{
	if (mouse_over) {
	       mouse_over->Leave ();
	       mouse_over = NULL;
	}

	FrameworkElement::Leave ();
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
Panel::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, DependencyProperty *subprop)
{
	if (prop == Panel::BackgroundProperty) {
		Invalidate ();
	}
	else {
		FrameworkElement::OnSubPropertyChanged (prop, obj, subprop);
	}
}

void
Panel::OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, DependencyProperty *prop)
{
	switch (type) {
	case CollectionChangeTypeItemAdded:
		// we could do some optimization here
	case CollectionChangeTypeItemRemoved:
	case CollectionChangeTypeChanged:
		UpdateBounds (true);
		break;
	case CollectionChangeTypeItemChanged:
		// if a child changes its ZIndex property we need to resort our Children
		if (prop == UIElement::ZIndexProperty) {
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


DependencyProperty* Panel::ChildrenProperty;
DependencyProperty* Panel::BackgroundProperty;

void 
panel_init (void)
{
	Panel::ChildrenProperty = DependencyObject::Register (Type::PANEL, "Children", Type::VISUAL_COLLECTION);
	Panel::BackgroundProperty = DependencyObject::Register (Type::PANEL, "Background", Type::BRUSH);
}

