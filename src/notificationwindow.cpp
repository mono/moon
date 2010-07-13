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
}

};
