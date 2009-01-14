/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * dirty.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __DIRTY_H__
#define __DIRTY_H__

class DirtyList;

class DirtyLists {
public:
	DirtyLists (bool ascending);
	~DirtyLists ();

	DirtyList* GetList (int level, bool create = false);
	void RemoveList (int level);

	void AddDirtyNode (int level, List::Node *node);
	void RemoveDirtyNode (int level, List::Node *node);

	List::Node *GetFirst ();

	bool IsEmpty ();

	void Clear (bool freeNodes);

private:
	bool ascending;
	List *lists;
};

enum DirtyType {
	// DirtyTransform
	//
	// This node needs to communicate its transform to its
	// children.  this can happen either when the parent's
	// transform changes or when this node's transform changes.
	DirtyTransform         = 0x00000001,

	// DirtyLocalTransform
	//
	// This node needs to update its local transform (relative to
	// its parent).  This implies DirtyTransform.
	DirtyLocalTransform    = 0x00000002,

	// DirtyClip
	//
	// This node needs to communicate its clip to its children.
	// this can happen either when the parent's clip changes or
	// when this node's clip changes.
	DirtyClip              = 0x00000004,

	// DirtyLocalClip
	//
	// This node needs to update its local clip (relative to
	// its parent).  This implies DirtyClip.
	DirtyLocalClip         = 0x00000008,

	// Dirty*Visibility
	//
	// The visibility (either render or hit-test) of this node has
	// changed, and we need to communicate this change to all its
	// children.
	DirtyRenderVisibility  = 0x00000010,
	DirtyHitTestVisibility = 0x00000020,

	// DirtyMeasure
	//
	// InvalidateMeasure was called on this element.
	DirtyMeasure           = 0x00000040,

	// DirtyMeasure
	//
	// InvalidateMeasure was called on this element.
	DirtyArrange           = 0x00000080,
	
	// DirtyChildrenZIndices
	//
	// This isn't really a downward pass, as it doesn't propagate
	// anything to children.  It's just so we can delay resorting
	// by ZIndex until the dirty passes run.
	//
	DirtyChildrenZIndices  = 0x00000100,

	DownDirtyState         = (DirtyLocalTransform |
				  DirtyTransform |
				  DirtyRenderVisibility |
				  DirtyHitTestVisibility |
				  DirtyLocalClip |
				  DirtyClip |
				  DirtyChildrenZIndices),

	// DirtyBounds
	//
	// The bounds of this element need to be recomputed.  If
	// they're found to be different, invalidate the node and set
	// DirtyBounds on the parent.
	DirtyBounds            = 0x00100000,
	DirtyNewBounds         = 0x00200000,

	// DirtyInvalidate
	//
	// element->dirty_rect contains the area needing repaint.  If
	// we're the toplevel surface, we generate an expose event on
	// the surface.  Otherwise we pass the rect up to our parent
	// (and union it in with the parent's dirty_rect), and set
	// DirtyInvalidate on the parent.
	DirtyInvalidate        = 0x00400000,

	UpDirtyState           = (DirtyBounds |
				  DirtyInvalidate),

	DirtyState             = DownDirtyState | UpDirtyState,
	DirtyInUpDirtyList     = 0x40000000,
	DirtyInDownDirtyList   = 0x80000000
};

#endif // __DIRTY_H__
