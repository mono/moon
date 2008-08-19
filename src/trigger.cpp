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
	SetValue (EventTrigger::ActionsProperty, Value::CreateUnref (new TriggerActionCollection ()));
	registered_event_id = -1;
}

EventTrigger::~EventTrigger ()
{
}

void
EventTrigger::SetTarget (DependencyObject *target)
{
	g_return_if_fail (target);

	if (target->GetSurface() && target->GetSurface()->IsSilverlight2()) {
		registered_event_id = target->GetType()->LookupEvent (GetValue (EventTrigger::RoutedEventProperty)->AsString());
		if (registered_event_id == -1)
			g_warning ("failed to set target");
	}
	else {
		/* Despite the name, in silverlight 1.0 it can only be
		   loaded (according to the docs) */
		registered_event_id = UIElement::LoadedEvent;
	}

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
	
	TriggerActionCollection *actions = trigger->GetValue (EventTrigger::ActionsProperty)->AsTriggerActionCollection ();
	
	for (int i = 0; i < actions->GetCount (); i++) {
		TriggerAction *action = actions->GetValueAt (i)->AsTriggerAction ();
		action->Fire ();
	}
}


void
event_trigger_action_add (EventTrigger *trigger, TriggerAction *action)
{
	trigger->GetValue (EventTrigger::ActionsProperty)->AsTriggerActionCollection()->Add (action);
}
