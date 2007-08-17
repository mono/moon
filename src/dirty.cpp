
#include "config.h"
#include <string.h>
#include <gtk/gtk.h>
#include <malloc.h>
#include <glib.h>
#include <stdlib.h>

#include "uielement.h"
#include "panel.h"
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
	if (element->dirty_flags & dirt)
		return;

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
  TimeSpan before = get_now ();
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

			add_dirty_element (el, DirtyBounds);

			el->ComputeTransform ();

			if (el->Is (Type::PANEL)) {
				Panel *p = (Panel*)el;
				VisualCollection *children = p->GetChildren();

				Collection::Node* n = (Collection::Node *) children->list->First ();
				while (n != NULL) {
					add_dirty_element ((UIElement *) n->obj, DirtyTransform);
					n = (Collection::Node *) n->Next ();
				}
			}
		}

		if (el->dirty_flags & DirtyOpacity) {
			el->dirty_flags &= ~DirtyOpacity;

			add_dirty_element (el, DirtyInvalidate);

			el->ComputeTotalOpacity ();

			if (el->Is (Type::PANEL)) {
				Panel *p = (Panel*)el;
				VisualCollection *children = p->GetChildren();

				Collection::Node* n = (Collection::Node *) children->list->First ();
				while (n != NULL) {
					add_dirty_element ((UIElement *) n->obj, DirtyOpacity);
					n = (Collection::Node *) n->Next ();
				}
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

		//printf ("up processing element element %p (%s)\n", el, el->GetTypeName());

		if (el->dirty_flags & DirtyBounds) {
			el->dirty_flags &= ~DirtyBounds;
			
			Rect obounds = el->GetBounds ();

			el->ComputeBounds ();

			if (obounds != el->GetBounds()) {
				if (el->GetParent ()) {
					DependencyObject *p = el->GetParent ();
					if (p->Is (Type::COLLECTION)) {
						p = ((Collection*)p)->closure;
					}

					add_dirty_element ((UIElement*)p, DirtyBounds);
				}

 				el->dirty_flags |= DirtyInvalidate;
				if (el->dirty_rect.IsEmpty()) {
					el->dirty_rect = el->GetBounds();
				}
				else {
					el->dirty_rect.Union (el->GetBounds());
				}
			}
		}

		if (el->dirty_flags & DirtyInvalidate) {
			el->dirty_flags &= ~DirtyInvalidate;

			if (el->GetParent ()) {
				DependencyObject *p = el->GetParent ();
				if (p->Is (Type::COLLECTION)) {
					p = ((Collection*)p)->closure;
				}

				UIElement *parent = (UIElement*)p;

				if (parent->dirty_flags & DirtyInvalidate) {
					parent->dirty_rect.Union (el->dirty_rect);
				}
				else {
					parent->dirty_rect = el->dirty_rect;

					add_dirty_element (parent, DirtyInvalidate);
				}
				el->dirty_rect = Rect (0,0,0,0);
			}
			else {
				if (el->GetSurface ()) {
					el->GetSurface()->Invalidate (el->dirty_rect);
				}
			}
		}


		if (!(el->dirty_flags & UpDirtyState)) {
			el->dirty_flags &= ~DirtyInUpDirtyList;
			up_dirty = g_slist_delete_link (up_dirty, link);
		}
	}

	g_assert (!up_dirty);
	g_assert (!down_dirty);

  TimeSpan after = get_now ();

  //  fprintf (stderr, "process_dirty_elements took %lld ticks\n", after - before); fflush (stderr);
}
