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

#include "trigger.h"
#include "collection.h"
#include "uielement.h"


EventTrigger::EventTrigger ()
{
	SetValue (EventTrigger::ActionsProperty, Value::CreateUnref (new TriggerActionCollection ()));
}

void
EventTrigger::SetTarget (DependencyObject *target)
{
	g_return_if_fail (target);

	// Despite the name, it can only be loaded (according to the docs)
	target->AddHandler (UIElement::LoadedEvent, event_trigger_fire_actions, this);
}

void
EventTrigger::RemoveTarget (DependencyObject *target)
{
	g_return_if_fail (target);

	target->RemoveHandler (UIElement::LoadedEvent, event_trigger_fire_actions, this);
}

EventTrigger::~EventTrigger ()
{
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


EventTrigger *
event_trigger_new (void)
{
	return new EventTrigger ();
}

void
event_trigger_action_add (EventTrigger *trigger, TriggerAction *action)
{
	trigger->GetValue (EventTrigger::ActionsProperty)->AsTriggerActionCollection()->Add (action);
}

DependencyProperty* EventTrigger::RoutedEventProperty;
DependencyProperty* EventTrigger::ActionsProperty;

void
event_trigger_init (void)
{
	EventTrigger::RoutedEventProperty = DependencyProperty::Register (Type::EVENTTRIGGER, "RoutedEvent", Type::STRING);
	EventTrigger::ActionsProperty = DependencyProperty::Register (Type::EVENTTRIGGER, "Actions", Type::TRIGGERACTION_COLLECTION);
}
