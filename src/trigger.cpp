/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * trigger.cpp
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include "runtime.h"
#include "trigger.h"
#include "collection.h"
#include "uielement.h"


EventTrigger::EventTrigger ()
{
	SetObjectType (Type::EVENTTRIGGER);
	registered_event_id = -1;
}

EventTrigger::~EventTrigger ()
{
}

void
EventTrigger::SetTarget (DependencyObject *target)
{
	g_return_if_fail (target);
	
#if AT_SOME_POINT_SILVERLIGHT_GETS_ITS_HEAD_OUT_OF_ITS_
	const char *event = GetRoutedEvent ();
	const char *dot;
	
	if (event && (dot = strchr (event, '.'))) {
		char *type = g_strndup (event, dot-event);
		const char *event_name = dot + 1;
		
		Type *event_type = Type::Find (type);
		g_free (type);
		
		if (event_type) {
			// event type has to exist
			if (target->GetType()->IsSubclassOf (event_type->GetKind ())) {
				// the type of the target has
				// to be a subclass of the
				// event type for the event to
				// be available.
				registered_event_id = event_type->LookupEvent (event_name);
			}
		}
	} else if (event) {
		registered_event_id = target->GetType ()->LookupEvent (event);
	}
#else
	registered_event_id = UIElement::LoadedEvent;
#endif
	
	if (registered_event_id == -1)
		g_warning ("failed to set target");
	
	if (registered_event_id != -1)
		target->AddHandler (registered_event_id, event_trigger_fire_actions, this);
}

void
EventTrigger::RemoveTarget (DependencyObject *target)
{
	g_return_if_fail (target);

	if (registered_event_id != -1) {
		target->RemoveHandler (registered_event_id, event_trigger_fire_actions, this);
		registered_event_id = -1;
	}
}

void
EventTrigger::event_trigger_fire_actions (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	EventTrigger *trigger = (EventTrigger *) closure;
	
	g_return_if_fail (trigger);
	
	TriggerActionCollection *actions = trigger->GetActions();
	
	for (int i = 0; i < actions->GetCount (); i++) {
		TriggerAction *action = actions->GetValueAt (i)->AsTriggerAction ();
		action->Fire ();
	}
}


void
event_trigger_action_add (EventTrigger *trigger, TriggerAction *action)
{
	trigger->GetActions()->Add (action);
}
