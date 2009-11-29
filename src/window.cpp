/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * window.cpp:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */
 
 #include "config.h"
 
 #include "window.h"
 #include "runtime.h"
 
void
MoonWindow::SetCurrentDeployment ()
{
	g_return_if_fail (surface != NULL);
	surface->SetCurrentDeployment ();
}