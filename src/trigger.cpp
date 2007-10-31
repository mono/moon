/*
 * trigger.cpp
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include <gtk/gtk.h>

#include "trigger.h"
#include "collection.h"
#include "uielement.h"


EventTrigger::EventTrigger ()
{
	this->SetValue (EventTrigger::ActionsProperty, Value::CreateUnref (new TriggerActionCollection ()));
}

//
// Intercept any changes to the actions property and mirror that into our
// own variable
//
void
EventTrigger::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop == ActionsProperty){
		TriggerActionCollection *newcol = GetValue (prop)->AsTriggerActionCollection();

		if (newcol) {
			if (newcol->closure)
				printf ("Warning we attached a property that was already attached\n");
			newcol->closure = this;
		}
	}

	NotifyAttachersOfPropertyChange (prop);
}

void
EventTrigger::SetTarget (DependencyObject *target)
{
	g_assert (target);

	// Despite the name, it can only be loaded (according to the docs)
	target->AddHandler (UIElement::LoadedEvent, event_trigger_fire_actions, this);
}

void
EventTrigger::RemoveTarget (DependencyObject *target)
{
	g_assert (target);

	target->RemoveHandler (UIElement::LoadedEvent, event_trigger_fire_actions, this);
}

EventTrigger::~EventTrigger ()
{
}

void
EventTrigger::event_trigger_fire_actions (EventObject *sender, gpointer calldata, gpointer closure)
{
	EventTrigger *trigger = (EventTrigger*)closure;
	g_assert (trigger);
	TriggerActionCollection *actions = trigger->GetValue (EventTrigger::ActionsProperty)->AsTriggerActionCollection();
	Collection::Node *n = (Collection::Node *) actions->list->First ();
	
	for ( ; n != NULL; n = (Collection::Node *) n->next) {
		TriggerAction *action = (TriggerAction *) n->obj;
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
	EventTrigger::RoutedEventProperty = DependencyObject::Register (Type::EVENTTRIGGER, "RoutedEvent", Type::STRING);
	EventTrigger::ActionsProperty = DependencyObject::Register (Type::EVENTTRIGGER, "Actions", Type::TRIGGERACTION_COLLECTION);
}
