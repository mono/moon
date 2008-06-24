/*
 * asf-structures.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include "asf-debug.h"

#if DEBUG

GHashTable* ObjectTracker::current_ids = NULL;
GHashTable* ObjectTracker::current_objects = NULL;

#endif
