/*
 * asf-structures.h: 
 *
 * Author: Rolf Bjarne Kvinge  (RKvinge@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include "debug.h"

GHashTable* ObjectTracker::current_ids = NULL;
GHashTable* ObjectTracker::current_objects = NULL;