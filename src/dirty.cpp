
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

void
Surface::AddDirtyElement (UIElement *element, DirtyType dirt)
{
  // XXX this should really be here...
//	if (element->dirty_flags & dirt)
//		return;

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
Surface::RemoveDirtyElement (UIElement *element)
{
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

			el->Invalidate ();
			el->ComputeTotalRenderVisibility ();
			el->Invalidate ();

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

			el->ComputeTransform ();
			el->UpdateBounds ();

			PropagateDirtyFlagToChildren (el, DirtyTransform);
		}

		if (!(el->dirty_flags & DownDirtyState)) {
			down_dirty->Remove (node);
			el->down_dirty_node = NULL;
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
			up_dirty->Remove (node);
			el->up_dirty_node = NULL;
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
