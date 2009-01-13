/*
 * usercontrol.cpp:
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>
#include "usercontrol.h"

UIElement *
user_control_get_content (UserControl *user_control)
{
	Value* v =user_control-> GetValue (UserControl::ContentProperty);
	if (!v)
		return NULL;
	return v->AsUIElement ();
}

void
UserControl::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property->GetOwnerType() != Type::USERCONTROL) {
		Control::OnPropertyChanged (args);
		return;
	}
	
	if (args->property == UserControl::ContentProperty){
		if (args->old_value) {
			ElementRemoved (args->old_value->AsUIElement ());
		}
		if (args->new_value) {
			ElementAdded (args->new_value->AsUIElement ());
		}

		UpdateBounds ();
	}
	NotifyListenersOfPropertyChange (args);
}

UserControl::UserControl ()
{
	SetIsTabStop (false);
}

UserControl::~UserControl ()
{
}
