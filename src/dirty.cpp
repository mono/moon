
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
#include "list.h"

static List *down_dirty = NULL;
static List *up_dirty = NULL;

class DirtyNode : public List::Node {
public:
	DirtyNode (UIElement *element) { this->element = element; }

	UIElement *element;
};

bool
is_anything_dirty ()
{
	if (!down_dirty)
		return false;

	return !down_dirty->IsEmpty() || !up_dirty->IsEmpty();
}

void
add_dirty_element (UIElement *element, DirtyType dirt)
{
	if (!down_dirty) {
		down_dirty = new List ();
		up_dirty = new List ();
	}

  // XXX this should really be here...
//  	if (element->dirty_flags & dirt)
//  		return;

	element->dirty_flags |= dirt;

	//printf ("adding element %p (%s) to the dirty list\n", element, element->GetTypeName());

	if (dirt & DownDirtyState) {
		if (element->down_dirty_node)
			return;
		element->down_dirty_node = new DirtyNode (element);
		down_dirty->Prepend (element->down_dirty_node);
	}

	if (dirt & UpDirtyState) {
		if (element->up_dirty_node)
			return;
		element->up_dirty_node = new DirtyNode (element);
		up_dirty->Prepend (element->up_dirty_node);
	}
}

void
remove_dirty_element (UIElement *element)
{
	if (!down_dirty)
		return;

	if (element->up_dirty_node) {
		up_dirty->Remove (element->up_dirty_node); // deletes the node
		element->up_dirty_node = NULL;
	}
	if (element->down_dirty_node) {
		down_dirty->Remove (element->down_dirty_node); // deletes the node
		element->down_dirty_node = NULL;
	}
}

/*
** There are 2 types of changes that need to propagate around the
** tree.
**
** 1. Those changes that need to be propagated from parent to children
**    (transformation, opacity).  We call these Downward Changes, and
**    the elements are placed in the down_dirty list.
**
** 2. Those changes that need to be propagated from children to parent
**    (bounds, invalidation).  We call these Upward Changes, and the
**    elements are placed in the up_dirty list.
**
**
** Downward Changes can result in new Upward changes (when an
** element's transform changes, usually its bounds change), so when
** processing the dirty list we push changes down the tree before
** handling the Upward Changes.
**
*/

void
process_dirty_elements ()
{
	if (!down_dirty)
		return;

	/* push down the transforms and opacity changes first */
	while (DirtyNode *node = (DirtyNode*)down_dirty->First()) {
		UIElement* el = (UIElement*)node->element;

		if (el->dirty_flags & DirtyOpacity) {
			el->dirty_flags &= ~DirtyOpacity;
			el->UpdateBounds ();

			el->Invalidate ();
			el->ComputeTotalOpacity ();
			el->Invalidate ();

			if (el->Is (Type::PANEL)) {
				Panel *p = (Panel*)el;
				VisualCollection *children = p->GetChildren();

				Collection::Node* n = (Collection::Node *) children->list->First ();
				while (n != NULL) {
					((UIElement *) n->obj)->UpdateTotalOpacity ();
					n = (Collection::Node *) n->next;
				}
			}
			else if (el->Is (Type::CONTROL)) {
				Control *c = (Control*)el;
				if (c->real_object)
					c->real_object->UpdateTotalOpacity();
			}
		}

		/*
		** since we cache N's local (from N's parent to N)
		** transform, we need to catch if we're changing
		** something about that local transform and recompute
		** it.
		** 
		** DirtyLocalTransform implies DirtyTransform, since
		** changing N's local transform requires updating the
		** transform of all descendents in the subtree rooted
		** at N.
		*/
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

				if (children != NULL) {
					Collection::Node* n = (Collection::Node *) children->list->First ();
					while (n != NULL) {
						// we don't call n->obj->UpdateTransform here
						// because that causes the element to recompute
						// its local transform as well, which isn't necessary.
						add_dirty_element ((UIElement *) n->obj, DirtyTransform);
						n = (Collection::Node *) n->next;
					}
				}
			}
			else if (el->Is (Type::CONTROL)) {
				Control *c = (Control*)el;
				if (c->real_object)
					add_dirty_element (c->real_object, DirtyTransform);
			}
		}

		if (!(el->dirty_flags & DownDirtyState)) {
			down_dirty->Remove (node);
			el->down_dirty_node = NULL;
		}
	}

	while (DirtyNode *node = (DirtyNode*)up_dirty->First()) {
		UIElement* el = (UIElement*)node->element;

//   		printf ("up processing element element %p (%s)\n", el, el->GetName());
// 		printf ("el->parent = %p\n", el->parent);

		if (el->dirty_flags & DirtyBounds) {
//   		  printf (" + bounds\n");
			el->dirty_flags &= ~DirtyBounds;

			Rect obounds = el->GetBounds ();

			el->ComputeBounds ();

// 				printf (" + + obounds = %f %f %f %f, nbounds = %f %f %f %f\n",
// 					obounds.x, obounds.y, obounds.w, obounds.h,
// 					el->GetBounds().x, el->GetBounds().y, el->GetBounds().w, el->GetBounds().h);

			if (obounds != el->GetBounds() || el->force_invalidate_of_new_bounds) {
				if (obounds != el->GetBounds()) {
					if (el->parent) {
// 						printf (" + + + calling UpdateBounds and Invalidate on parent\n");
						el->parent->UpdateBounds();
						Region oregion = Region (obounds);
						el->parent->ChildInvalidated (&oregion);
					}
				} 
				
				el->force_invalidate_of_new_bounds = false;
				el->Invalidate ();
			}
		}

		if (el->dirty_flags & DirtyInvalidate) {
//  			printf (" + invalidating %p (%s) %s, %f %f %f %f\n",
// 				el, el->GetTypeName(), el->GetName(), el->dirty_rect.x, el->dirty_rect.y, el->dirty_rect.w, el->dirty_rect.h);

			el->dirty_flags &= ~DirtyInvalidate;

			Region *dirty = el->dirty_region;
			dirty->Union (el->children_dirty_region);

			if (el->parent) {
// 			  printf (" + + invalidating parent (%f,%f,%f,%f)\n",
// 				  el->dirty_rect.x,
// 				  el->dirty_rect.y,
// 				  el->dirty_rect.w,
// 				  el->dirty_rect.h);
				el->parent->ChildInvalidated (dirty);
			}
			else if (el->Is (Type::CANVAS) &&
				 el->parent == NULL &&
				 el->GetSurface() &&
				 el->GetSurface()->IsTopLevel (el)) {
				GdkRectangle *rects;
				int count;
				dirty->GetRectangles (&rects, &count);
				Surface *surface = el->GetSurface ();
				while (count--) {
					Rect r = Rect ((double)rects [count].x,
						       (double)rects [count].y,
						       (double)rects [count].width,
						       (double)rects [count].height);
					//printf (" + + invalidating parent (%f,%f,%f,%f)\n",
					//	r.x,
					//	r.y,
					//	r.w,
					//	r.h);
					
					surface->Invalidate (r);					
				}
				g_free (rects);
			}

			delete el->dirty_region;
			el->dirty_region = new Region ();
			delete el->children_dirty_region;
			el->children_dirty_region = new Region ();
		}

		if (!(el->dirty_flags & UpDirtyState)) {
			up_dirty->Remove (node);
			el->up_dirty_node = NULL;
		}
	}

	g_assert (up_dirty->IsEmpty());
	g_assert (down_dirty->IsEmpty());
}
