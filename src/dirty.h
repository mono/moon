
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

	// DirtyOpacity
	//
	// The opacity of the element needs ot be recomputed.
	// Recomputes the opacity on this node and sets DirtyOpacity
	// on all children.
	DirtyOpacity        = 0x08,

	// DirtyInvalidate
	//
	// element->dirty_rect contains the area needing repaint.  If
	// we're the toplevel surface, we generate an expose event on
	// the surface.  Otherwise we pass the rect up to our parent
	// (and union it in with the parent's dirty_rect), and set
	// DirtyInvalidate on the parent.
	DirtyInvalidate     = 0x10,

	DownDirtyState      = DirtyOpacity | DirtyLocalTransform | DirtyTransform,
	UpDirtyState        = DirtyBounds | DirtyInvalidate,

	DirtyState          = DownDirtyState | UpDirtyState,

	DirtyInUpDirtyList  = 0x40,
	DirtyInDownDirtyList = 0x80
};

void add_dirty_element (UIElement *element, DirtyType dirt);
void process_dirty_elements ();
bool is_anything_dirty ();
