/*
 * trigger.h
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_TRIGGER_H__
#define __MOON_TRIGGER_H__

#include <glib.h>

#include "dependencyobject.h"

class TriggerAction : public DependencyObject {
 protected:
	virtual ~TriggerAction () {}

 public:
	TriggerAction () { };

	virtual Type::Kind GetObjectType () { return Type::TRIGGERACTION; };
	virtual void Fire () = 0;
};


/* @ContentProperty="Actions" */
/* @Namespace=System.Windows */
class EventTrigger : public DependencyObject {
 protected:
	virtual ~EventTrigger ();

 public:
	/* @GenerateCBinding */
	EventTrigger ();
	
	virtual Type::Kind GetObjectType () { return Type::EVENTTRIGGER; };

	void SetTarget (DependencyObject *target);
	void RemoveTarget (DependencyObject *target);

	/* @PropertyType=string,ManagedPropertyType=RoutedEvent */
	static DependencyProperty *RoutedEventProperty;
	/* @PropertyType=TriggerActionCollection */
	static DependencyProperty *ActionsProperty;

 private:
	static void event_trigger_fire_actions (EventObject *sender, EventArgs *calldata, gpointer closure);
};

G_BEGIN_DECLS
void event_trigger_action_add (EventTrigger *trigger, TriggerAction *action);
G_END_DECLS

#endif /* __MOON_TRIGGER_H__ */
