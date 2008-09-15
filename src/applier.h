/*
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef MOON_APPLIER_H
#define MOON_APPLIER_H

#include <glib.h>

#include "trigger.h"
#include "collection.h"
#include "clock.h"
#include "list.h"
#include "point.h"

class Applier {

 private:
	GList *list;

 protected:
	~Applier ();

 public:
	Applier ();
};
	
#endif /* MOON_APPLIER_H */
