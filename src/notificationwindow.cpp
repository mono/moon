/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * notificationwindow.cpp
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */
 
#include <config.h>
 
#include "notificationwindow.h"

namespace Moonlight {

NotificationWindow::NotificationWindow ()
{
	SetObjectType (Type::NOTIFICATIONWINDOW);
}

Window::Window ()
{
	SetObjectType (Type::WINDOW);
	moon_window = NULL;
}

void
Window::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty()->GetOwnerType() != Type::WINDOW)
		DependencyObject::OnPropertyChanged (args, error);

	if (moon_window) {
		int prop_id = args->GetProperty()->GetId();

		if (prop_id == Window::HeightProperty) {
			moon_window->SetHeight (args->GetNewValue()->AsDouble ());
		}
		else if (prop_id == Window::WidthProperty) {
			moon_window->SetWidth (args->GetNewValue()->AsDouble ());
		}
		else if (prop_id == Window::LeftProperty) {
			moon_window->SetLeft (args->GetNewValue()->AsDouble ());
		}
		else if (prop_id == Window::TopProperty) {
			moon_window->SetTop (args->GetNewValue()->AsDouble ());
		}
		else if (prop_id == Window::TopMostProperty) {
			// FIXME
			printf ("Window::TopMost changed!\n");
		}
		else if (prop_id == Window::WindowStateProperty) {
			// FIXME
			printf ("Window::WindowState changed!\n");
		}
	}

	NotifyListenersOfPropertyChange (args, error);
}

void
Window::SetMoonWindow (MoonWindow *moon_window)
{
	this->moon_window = moon_window;
}

bool
Window::ActivateWithError (MoonError *error)
{
	printf ("NIEX Window::ActivateWithError\n");
	return true;
}

void
Window::CloseWithError (MoonError *error)
{
	printf ("NIEX Window::CloseWithError\n");
}

void
Window::DragMoveWithError (MoonError *error)
{
	printf ("NIEX Window::DragMoveWithError\n");
}

void
Window::DragResizeWithError (MoonError *error)
{
	printf ("NIEX Window::DragResizeWithError\n");
}

void
Window::SetTitle (const char *title)
{
	if (moon_window)
		moon_window->SetTitle (title);
}

void
Window::SetStyle (const WindowStyle style)
{
	if (moon_window)
		moon_window->SetStyle (style);
}

};
