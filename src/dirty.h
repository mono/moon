/*
 * dirty.h: 
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

enum DirtyType {
	// DirtyTransform
	//
	// This node needs to communicate its transform to its
	// children.  this can happen either when the parent's
	// transform changes or when this node's transform changes.
	DirtyTransform      = 0x01,

	// DirtyLocalTransform
	//
	// This node needs to update its local transform (relative to
	// its parent).  This implies DirtyTransform.
	DirtyLocalTransform = 0x02,

	// DirtyBounds
	//
	// The bounds of this element need to be recomputed.  If
	// they're found to be different, invalidate the node and set
	// DirtyBounds on the parent.
	DirtyBounds         = 0x04,

	DirtyRenderVisibility  = 0x08,
	DirtyHitTestVisibility = 0x10,

	// DirtyInvalidate
	//
	// element->dirty_rect contains the area needing repaint.  If
	// we're the toplevel surface, we generate an expose event on
	// the surface.  Otherwise we pass the rect up to our parent
	// (and union it in with the parent's dirty_rect), and set
	// DirtyInvalidate on the parent.
	DirtyInvalidate     = 0x20,

	DownDirtyState      = DirtyLocalTransform | DirtyTransform | DirtyRenderVisibility | DirtyHitTestVisibility,
	UpDirtyState        = DirtyBounds | DirtyInvalidate,

	DirtyState          = DownDirtyState | UpDirtyState,

	DirtyInUpDirtyList  = 0x80,
	DirtyInDownDirtyList = 0x100
};

void add_dirty_element (UIElement *element, DirtyType dirt);
void remove_dirty_element (UIElement *element);
void process_dirty_elements ();
bool is_anything_dirty ();
