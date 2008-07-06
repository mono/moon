
#include "config.h"
#include <glib.h>

#include "uielement.h"
#include "panel.h"
#include "control.h"
#include "visual.h"
#include "runtime.h"
#include "clock.h"
#include "dirty.h"
#include "list.h"

class DirtyNode : public List::Node {
public:
	DirtyNode (UIElement *element) 
	{
		this->element = element;
	}
	UIElement *element;
};

bool
Surface::IsAnythingDirty ()
{
	return !down_dirty->IsEmpty() || !up_dirty->IsEmpty();
}

/*
** We keep the dirty lists partially sorted, with the following
** invariants:
**
** 1. In the down dirty list, ancestors always appear before
**    descendents.
** 2. In the up dirty list, descendents always appear before
**    ancestors.
**
** No ordering guarantee is made between siblings, regardless of their
** zindex.
**
** To make the previous invariants hold, inserting nodes into the list
** is complicated a little.
**
** When inserting a node in the down dirty list:
**
**
** 1. we walk up the tree looking for the first element that is
**    in the list (down_dirty_node != NULL).  If we find one, we
**    insert the new node directly after the ancestor.
** 
** 2. If an ancestor has not been inserted, we prepend the node to the
**    start of the list.
**
**
** This 2 step process proceeds analogously in the up dirty case,
** except we insert the new nodes before parents.
**
*/
static List::Node*
find_down_pred (UIElement *element)
{
	UIElement *parent = element->GetVisualParent();
	for (; parent; parent = parent->GetVisualParent())
		if (parent->down_dirty_node)
			return parent->down_dirty_node;

	return NULL;
}

static List::Node*
find_up_succ (UIElement *element)
{
	UIElement *parent = element->GetVisualParent();
	for (; parent; parent = parent->GetVisualParent())
		if (parent->up_dirty_node)
			return parent->up_dirty_node;

	return NULL;
}

void
Surface::AddDirtyElement (UIElement *element, DirtyType dirt)
{
	// XXX this should really be here...
// 	if (element->dirty_flags & dirt)
// 		return;

	element->dirty_flags |= dirt;

	//printf ("adding element %p (%s) to the dirty list\n", element, element->GetTypeName());

	if (dirt & DownDirtyState) {
		if (element->down_dirty_node)
			return;
		element->down_dirty_node = new DirtyNode (element);

		List::Node *pred = find_down_pred (element);
		if (pred) {
			// an ancestor already exists in the
			// tree, we need to insert ourselves
			// after it
			down_dirty->InsertBefore (element->down_dirty_node, pred->next);
		}
		else {
			// no ancestor exists in the tree.  insert
			// ourselves at the start of the list.
			down_dirty->Prepend (element->down_dirty_node);
		}
	}

	if (dirt & UpDirtyState) {
		if (element->up_dirty_node)
			return;
		element->up_dirty_node = new DirtyNode (element);

		List::Node *succ = find_up_succ (element);
		if (succ) {
			// an ancestor already exists in the
			// tree, we need to insert ourselves
			// before it
			up_dirty->InsertBefore (element->up_dirty_node, succ);
		}
		else {
			// no ancestor exists in the tree.  insert
			// ourselves at the end of the list.
			up_dirty->Append (element->up_dirty_node);
		}
	}
}

/*
**
** Removing nodes is also a little complicated, in that we need to
** update pred/succ pointers if they refered to the node we're
** deleting.  So each removal method walks up the tree splicing the
** element's succ/pred into the ancestor's dirty info.
**
** It may not be immediately apparent, but these methods guarantee
** that once the list has been completely drained, the pred/succ links
** are NULL for all elements.
*/
static void
remove_up_dirty_node (List* up_dirty, UIElement *element)
{
	up_dirty->Remove (element->up_dirty_node); // deletes the node
	element->up_dirty_node = NULL;
}

static void
remove_down_dirty_node (List* down_dirty, UIElement *element)
{
	down_dirty->Remove (element->down_dirty_node); // deletes the node
	element->down_dirty_node = NULL;
}

void
Surface::RemoveDirtyElement (UIElement *element)
{
	if (element->up_dirty_node)
		remove_up_dirty_node (up_dirty, element);
	if (element->down_dirty_node)
		remove_down_dirty_node (down_dirty, element);
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
Surface::PropagateDirtyFlagToChildren (UIElement *el, DirtyType flags)
{
	if (el->Is (Type::PANEL)) {
		Panel *p = (Panel*)el;
		VisualCollection *children = p->GetChildren();

		Collection::Node* n = (Collection::Node *) children->list->First ();
		while (n != NULL) {
			AddDirtyElement ((UIElement *)n->obj, flags);
			n = (Collection::Node *) n->next;
		}
	}
	else if (el->Is (Type::CONTROL)) {
		Control *c = (Control*)el;
		if (c->real_object)
			AddDirtyElement (c->real_object, flags);
	}
}

void
Surface::ProcessDownDirtyElements ()
{
	/* push down the transforms opacity, and visibility changes first */
	while (DirtyNode *node = (DirtyNode*)down_dirty->First()) {
		UIElement* el = (UIElement*)node->element;

		if (el->dirty_flags & DirtyRenderVisibility) {
			el->dirty_flags &= ~DirtyRenderVisibility;

			el->UpdateBounds ();
			// Since we are not included in our parents subtree when we
			// are collapsed we need to notify our parent that things may
			// have changed
			if (el->GetVisualParent ())
				el->GetVisualParent ()->UpdateBounds ();

			el->ComputeTotalRenderVisibility ();
			el->dirty_flags |= DirtyNewBounds;

			PropagateDirtyFlagToChildren (el, DirtyRenderVisibility);
		}

		if (el->dirty_flags & DirtyHitTestVisibility) {
			el->dirty_flags &= ~DirtyHitTestVisibility;
			
			el->ComputeTotalHitTestVisibility ();

			PropagateDirtyFlagToChildren (el, DirtyHitTestVisibility);
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

			el->Invalidate ();
			el->ComputeTransform ();
			el->UpdateBounds ();
			el->dirty_flags |= DirtyNewBounds;

			PropagateDirtyFlagToChildren (el, DirtyTransform);
		}

		if (el->dirty_flags & DirtyLocalClip) {
			el->dirty_flags &= ~DirtyLocalClip;
			el->dirty_flags |= DirtyClip;

			// XXX el->ComputeLocalClip ();
		}

		if (el->dirty_flags & DirtyClip) {
			el->dirty_flags &= ~DirtyTransform;

			// XXX el->ComputeClip ();
			// XXX el->UpdateBounds ();

			PropagateDirtyFlagToChildren (el, DirtyClip);
		}

		if (el->dirty_flags & DirtyChildrenZIndices) {
			el->dirty_flags &= ~DirtyChildrenZIndices;
			if (!el->Is(Type::PANEL)) { 
				g_warning ("DirtyChildrenZIndices is only applicable to Panel subclasses");
			}
			else {
				((Panel*)el)->GetChildren ()->ResortByZIndex();
			}
			    
		}

		if (el->dirty_flags & DirtyPosition) {
			el->dirty_flags &= ~DirtyPosition;

			Rect obounds = el->GetBounds ();

			el->Invalidate ();
			el->ComputePosition ();
 			if (obounds != el->GetBounds() && el->GetVisualParent ())
				el->GetVisualParent()->UpdateBounds();
			el->dirty_flags |= DirtyNewBounds;

 			PropagateDirtyFlagToChildren (el, DirtyPosition);
		}

		if (!(el->dirty_flags & DownDirtyState)) {
			remove_down_dirty_node (down_dirty, el);
		}
	}
	
	if (!down_dirty->IsEmpty())
		g_warning ("after down dirty pass, down dirty list is not empty");
}

/*
** Note that since this calls GDK invalidation functions 
** it's a good idea to call it with a GDK lock held (all gtk callbacks
** are automatically protected except for timeouts and idle)
*/
void
Surface::ProcessUpDirtyElements ()
{
	while (DirtyNode *node = (DirtyNode*)up_dirty->First()) {
		UIElement* el = (UIElement*)node->element;

//   		printf ("up processing element element %p (%s)\n", el, el->GetName());
// 		printf ("el->parent = %p\n", el->parent);

		if (el->dirty_flags & DirtyBounds) {
//			printf (" + bounds\n");
			el->dirty_flags &= ~DirtyBounds;

			Rect obounds = el->GetBounds ();
			Rect osubtreebounds = el->GetSubtreeBounds ();
			bool parent_bounds_updated = false;

			el->ComputeBounds ();

// 				printf (" + + obounds = %f %f %f %f, nbounds = %f %f %f %f\n",
// 					obounds.x, obounds.y, obounds.w, obounds.h,
// 					el->GetBounds().x, el->GetBounds().y, el->GetBounds().w, el->GetBounds().h);

			if (osubtreebounds != el->GetSubtreeBounds ()) {
				if (el->GetVisualParent ()) {
					el->GetVisualParent ()->UpdateBounds ();
					parent_bounds_updated = true;
				}
			}

			if (obounds != el->GetBounds()) {
				if (el->GetVisualParent ()) {
// 						printf (" + + + calling UpdateBounds and Invalidate on parent\n");
					if (!parent_bounds_updated)
						el->GetVisualParent ()->UpdateBounds();

					Region oregion = Region (obounds);
					el->GetVisualParent ()->Invalidate (&oregion);
				}
				el->Invalidate ();
			}
				
			if (el->force_invalidate_of_new_bounds) {
				el->force_invalidate_of_new_bounds = false;
				// Invalidate everything including the
				// visible area of our children.
				el->Invalidate (el->GetSubtreeBounds ());
			}
		}
		if (el->dirty_flags & DirtyNewBounds) {
			el->Invalidate ();
			el->dirty_flags &= ~DirtyNewBounds;
		}
		if (el->dirty_flags & DirtyInvalidate) {
//  			printf (" + invalidating %p (%s) %s, %f %f %f %f\n",
// 				el, el->GetTypeName(), el->GetName(), el->dirty_rect.x, el->dirty_rect.y, el->dirty_rect.w, el->dirty_rect.h);

			el->dirty_flags &= ~DirtyInvalidate;

			Region *dirty = el->dirty_region;

			GdkRectangle *rects;
			int count;
			dirty->GetRectangles (&rects, &count);
			Surface *surface = el->GetSurface ();
			if (surface) {
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
		}

		if (!(el->dirty_flags & UpDirtyState)) {
			remove_up_dirty_node (up_dirty, el);
		}
	}
	
	if (!up_dirty->IsEmpty())
		g_warning ("after up dirty pass, up dirty list is not empty");
}

void
Surface::ProcessDirtyElements ()
{
	ProcessDownDirtyElements ();
	ProcessUpDirtyElements ();
}
