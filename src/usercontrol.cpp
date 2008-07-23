/*
 * control.cpp:
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>
#include "usercontrol.h"

UserControl *
user_control_new (void)
{
	UserControl *x = new UserControl ();
	printf ("UserControl is %p\n", x);
	return x;
}

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
	if (args->property->type != Type::USERCONTROL) {
		Control::OnPropertyChanged (args);
		return;
	}
	
	if (args->property == UserControl::ContentProperty){
		SetContent (args->new_value->AsUIElement (), GetSurface ());
		UpdateBounds ();
	}
	NotifyListenersOfPropertyChange (args);
}

UserControl::UserControl ()
{
}

UserControl::~UserControl ()
{
}

DependencyProperty *UserControl::ContentProperty;

void 
user_control_init ()
{
	UserControl::ContentProperty = DependencyProperty::Register (Type::USERCONTROL, "Content", Type::UIELEMENT);
}
