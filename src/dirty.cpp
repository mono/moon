
#include "config.h"
#include <string.h>
#include <gtk/gtk.h>
#include <malloc.h>
#include <glib.h>
#include <stdlib.h>

#include "uielement.h"
#include "panel.h"
#include "control.h"
#include "collection.h"
#include "runtime.h"
#include "clock.h"
#include "dirty.h"

GSList *down_dirty = NULL;
GSList *up_dirty = NULL;

bool
is_anything_dirty ()
{
	return down_dirty != NULL || up_dirty != NULL;
}

void
add_dirty_element (UIElement *element, DirtyType dirt)
{
  // XXX this should really be here...
//  	if (element->dirty_flags & dirt)
//  		return;

	element->dirty_flags |= dirt;

	//printf ("adding element %p (%s) to the dirty list\n", element, element->GetTypeName());

	if (element->dirty_flags & DownDirtyState) {
		if (element->dirty_flags & DirtyInDownDirtyList)
			return;
		element->dirty_flags |= DirtyInDownDirtyList;
		down_dirty = g_slist_prepend (down_dirty, element);
	}
	else {
		if (element->dirty_flags & DirtyInUpDirtyList)
			return;
		element->dirty_flags |= DirtyInUpDirtyList;
		up_dirty = g_slist_prepend (up_dirty, element);
	}
}

void
process_dirty_elements ()
{
	/* push down the transforms and opacity changes first */
	while (down_dirty) {
		GSList *link = down_dirty;
		UIElement* el = (UIElement*)link->data;

		if (el->dirty_flags & DirtyLocalTransform) {
			el->dirty_flags &= ~DirtyLocalTransform;

			el->dirty_flags |= DirtyTransform;

			el->ComputeLocalTransform ();
		}

		if (el->dirty_flags & DirtyTransform) {
			el->dirty_flags &= ~DirtyTransform;

			el->ComputeTransform ();
			el->UpdateBounds ();

			if (el->Is (Type::PANEL)) {
				Panel *p = (Panel*)el;
				VisualCollection *children = p->GetChildren();

				Collection::Node* n = (Collection::Node *) children->list->First ();
				while (n != NULL) {
					// we don't call n->obj->UpdateTransform here
					// because that causes the element to recompute
					// its local transform as well, which isn't necessary.
					add_dirty_element ((UIElement *) n->obj, DirtyTransform);
					n = (Collection::Node *) n->Next ();
				}
			}
			else if (el->Is (Type::CONTROL)) {
				Control *c = (Control*)el;
				if (c->real_object)
					add_dirty_element (c->real_object, DirtyTransform);
			}
		}

		if (el->dirty_flags & DirtyOpacity) {
			el->dirty_flags &= ~DirtyOpacity;

			el->ComputeTotalOpacity ();
			el->Invalidate ();

			if (el->Is (Type::PANEL)) {
				Panel *p = (Panel*)el;
				VisualCollection *children = p->GetChildren();

				Collection::Node* n = (Collection::Node *) children->list->First ();
				while (n != NULL) {
					((UIElement *) n->obj)->UpdateTotalOpacity ();
					n = (Collection::Node *) n->Next ();
				}
			}
			else if (el->Is (Type::CONTROL)) {
				Control *c = (Control*)el;
				if (c->real_object)
					c->real_object->UpdateTotalOpacity();
			}
		}

		if (!(el->dirty_flags & DownDirtyState)) {
			el->dirty_flags &= ~DirtyInDownDirtyList;
			down_dirty = g_slist_delete_link (down_dirty, link);
		}
	}

	while (up_dirty) {
		GSList *link = up_dirty;
		UIElement* el = (UIElement*)link->data;

//   		printf ("up processing element element %p (%s)\n", el, el->GetName());
// 		printf ("el->parent = %p\n", el->parent);

		if (el->dirty_flags & DirtyBounds) {
//   		  printf (" + bounds\n");
			el->dirty_flags &= ~DirtyBounds;

			if (!el->Is(Type::CANVAS)
			    || el->parent
			    || !el->GetSurface()
			    || el->GetSurface()->GetToplevel() != el) {

				Rect obounds = el->GetBounds ();

				el->ComputeBounds ();

// 				printf (" + + obounds = %f %f %f %f, nbounds = %f %f %f %f\n",
// 					obounds.x, obounds.y, obounds.w, obounds.h,
// 					el->GetBounds().x, el->GetBounds().y, el->GetBounds().w, el->GetBounds().h);

 				if (obounds != el->GetBounds()) {
					if (el->parent) {
// 						printf (" + + + calling UpdateBounds and Invalidate on parent\n");
						el->parent->UpdateBounds();
						el->parent->Invalidate(obounds);
					}

					el->Invalidate ();
 				}
			}
		}

		if (el->dirty_flags & DirtyInvalidate) {
// 		  printf (" + invalidate\n");

			el->dirty_flags &= ~DirtyInvalidate;

			if (el->parent) {
// 			  printf (" + + invalidating parent (%f,%f,%f,%f)\n",
// 				  el->dirty_rect.x,
// 				  el->dirty_rect.y,
// 				  el->dirty_rect.w,
// 				  el->dirty_rect.h);
				el->parent->Invalidate (el->dirty_rect.GrowBy(1));
			}
			else if (el->Is (Type::CANVAS) &&
				 el->parent == NULL &&
				 el->GetSurface() &&
				 el->GetSurface()->GetToplevel() == el) {

				el->GetSurface()->Invalidate (el->dirty_rect.GrowBy(1));
			}

			el->dirty_rect = Rect (0,0,0,0);
		}


		if (!(el->dirty_flags & UpDirtyState)) {
			el->dirty_flags &= ~DirtyInUpDirtyList;
			up_dirty = g_slist_delete_link (up_dirty, link);
		}
	}

	g_assert (!up_dirty);
	g_assert (!down_dirty);
}
