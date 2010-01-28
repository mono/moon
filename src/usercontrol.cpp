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
	SetIsTabStop (false);
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
UserControl::MeasureOverride (Size availableSize)
{
	Size desired = Size (0,0);
	
	Thickness border = *GetPadding () + *GetBorderThickness ();

	// Get the desired size of our child, and include any margins we set
	VisualTreeWalker walker = VisualTreeWalker (this);
	while (UIElement *child = walker.Step ()) {
		child->Measure (availableSize.GrowBy (-border));
		desired = child->GetDesiredSize ();
	}

	desired = desired.GrowBy (border);
	
	return desired;
}

Size
UserControl::ArrangeOverride (Size finalSize)
{
	Thickness border = *GetPadding () + *GetBorderThickness ();

	Size arranged = finalSize;

	VisualTreeWalker walker = VisualTreeWalker (this);
	while (UIElement *child = walker.Step ()) {
		Size desired = child->GetDesiredSize ();
		Rect childRect (0,0,finalSize.width,finalSize.height);

		childRect = childRect.GrowBy (-border);

		child->Arrange (childRect);

		arranged = Size (childRect.width, childRect.height).GrowBy (border);
	}
	return arranged;
}
