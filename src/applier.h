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

#define APPLIER_PRECEDENCE_INSTANT 0
#define APPLIER_PRECEDENCE_ANIMATION 100
#define APPLIER_PRECEDENCE_ANIMATION_RESET APPLIER_PRECEDENCE_INSTANT

class Applier {

 private:
	GHashTable *objects;
	bool readonly;

 public:
	Applier ();
	~Applier ();

	void AddPropertyChange (DependencyObject *object, DependencyProperty *property, Value *v, int precedence);
	void Apply ();
	void Flush ();
};
	
#endif /* MOON_APPLIER_H */
