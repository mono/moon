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
		char* event = GetValue (EventTrigger::RoutedEventProperty)->AsString();
		char* dot;
		if ((dot = strchr (event, '.')) == NULL) {
			registered_event_id = target->GetType()->LookupEvent (GetValue (EventTrigger::RoutedEventProperty)->AsString());
		}
		else {
			char *type = g_strndup (event, dot-event);
			char *event_name = g_strdup (dot + 1);

			Type *event_type = Type::Find(type);
			if (event_type) {
				// event type has to exist
				if (target->GetType()->IsSubclassOf(event_type->GetKind())) {
					// the type of the target has
					// to be a subclass of the
					// event type for the event to
					// be available.
					registered_event_id = event_type->LookupEvent (event_name);
				}
			}

			g_free (type);
			g_free (event_name);
		}
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
