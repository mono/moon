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

#include "enums.h"
#include "pal.h"

namespace Moonlight {

class Keyboard {
	static ModifierKeys modifiers;
	static GHashTable *pressedKeys;
	
 public:
	/* @GeneratePInvoke */
	static ModifierKeys GetModifiers ();
	
	static void UpdateModifiers (MoonEvent *event);
	
	static void OnKeyPress (MoonKeyEvent *key);
	static void OnKeyRelease (MoonKeyEvent *key);
	
	static bool IsKeyPressed (MoonKeyEvent *key);
};

};
#endif /* __KEYBOARD_H__ */
