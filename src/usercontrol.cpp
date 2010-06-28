/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
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
#include "collection.h"
#include "runtime.h"

UserControl::UserControl ()
{
	SetObjectType (Type::USERCONTROL);
}

UserControl::~UserControl ()
{
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
UserControl::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType() != Type::USERCONTROL) {
		Control::OnPropertyChanged (args, error);
		return;
	}
	
	if (args->GetId () == UserControl::ContentProperty){
		if (args->GetOldValue () && args->GetOldValue ()->AsUIElement ()) {
			if (args->GetOldValue()->Is(GetDeployment (), Type::FRAMEWORKELEMENT)) {
				args->GetOldValue()->AsFrameworkElement()->SetLogicalParent (NULL, error);
				if (error->number)
					return;
			}
			ElementRemoved (args->GetOldValue()->AsUIElement ());
		}
		if (args->GetNewValue () && args->GetNewValue ()->AsUIElement ()) {
			if (args->GetNewValue()->Is(GetDeployment (), Type::FRAMEWORKELEMENT)) {
				args->GetNewValue()->AsFrameworkElement()->SetLogicalParent (this, error);
				if (error->number)
					return;
			}
			ElementAdded (args->GetNewValue()->AsUIElement ());
		}

		UpdateBounds ();
	}
	NotifyListenersOfPropertyChange (args, error);
}

Size
UserControl::MeasureOverrideWithError (Size availableSize, MoonError *error)
{
	Size desired = Size (0,0);
	
	Thickness border = *GetPadding () + *GetBorderThickness ();

	// Get the desired size of our child, and include any margins we set
	VisualTreeWalker walker = VisualTreeWalker (this);
	while (UIElement *child = walker.Step ()) {
		child->MeasureWithError (availableSize.GrowBy (-border), error);
		desired = child->GetDesiredSize ();
	}

	desired = desired.GrowBy (border);
	
	return desired;
}

Size
UserControl::ArrangeOverrideWithError (Size finalSize, MoonError *error)
{
	Thickness border = *GetPadding () + *GetBorderThickness ();

	Size arranged = finalSize;

	VisualTreeWalker walker = VisualTreeWalker (this);
	while (UIElement *child = walker.Step ()) {
		Rect childRect (0,0,finalSize.width,finalSize.height);

		childRect = childRect.GrowBy (-border);

		child->ArrangeWithError (childRect, error);

		arranged = Size (childRect.width, childRect.height).GrowBy (border);
	}
	return arranged;
}
