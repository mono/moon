/*
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include "applier.h"

Applier::Applier ()
{
	list = NULL;
}

Applier::~Applier ()
{
	g_list_free (list);
}
