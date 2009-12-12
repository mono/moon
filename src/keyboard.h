/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * keyboard.h: keyboard utils
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include <glib.h>

#include <gdk/gdkevents.h>

#include "enums.h"

class Keyboard {
	static ModifierKeys modifiers;
	static GHashTable *pressedKeys;
	
 public:
	/* @GenerateCBinding,GeneratePInvoke */
	static ModifierKeys GetModifiers ();
	
	static void SetModifiers (ModifierKeys m);
	
	static void OnKeyPress (Key key);
	static void OnKeyRelease (Key key);
	
	static bool IsKeyPressed (Key key);
	
	static Key MapKeyValToKey (guint keyval);

	static int MapGdkToVKey (GdkEventKey *event);
};

#endif /* __KEYBOARD_H__ */
