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

/* @Namespace=None */
class TriggerAction : public DependencyObject {
 protected:
	virtual ~TriggerAction () {}
	
 public:
	/* @GenerateCBinding,GeneratePInvoke */
	TriggerAction () {}
	
	virtual Type::Kind GetObjectType () { return Type::TRIGGERACTION; };
	
	/* @GenerateCBinding */
	virtual void Fire () {}
};


/* @ContentProperty="Actions" */
/* @Namespace=System.Windows */
class EventTrigger : public DependencyObject {
	static void event_trigger_fire_actions (EventObject *sender, EventArgs *calldata, gpointer closure);
	
 protected:
	virtual ~EventTrigger ();

 public:
	/* @PropertyType=TriggerActionCollection,ManagedFieldAccess=Internal,ManagedSetterAccess=Internal */
	static DependencyProperty *ActionsProperty;
	/* @PropertyType=string,ManagedPropertyType=RoutedEvent,ManagedFieldAccess=Internal */
	static DependencyProperty *RoutedEventProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	EventTrigger ();
	
	virtual Type::Kind GetObjectType () { return Type::EVENTTRIGGER; };

	void SetTarget (DependencyObject *target);
	void RemoveTarget (DependencyObject *target);
};

G_BEGIN_DECLS
void event_trigger_action_add (EventTrigger *trigger, TriggerAction *action);
G_END_DECLS

#endif /* __MOON_TRIGGER_H__ */
