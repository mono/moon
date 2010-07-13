/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * keyboard.cpp: 
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <config.h>

#include <stdio.h>

#include "keyboard.h"

namespace Moonlight {

ModifierKeys Keyboard::modifiers = ModifierKeyNone;
GHashTable  *Keyboard::pressedKeys = NULL;


ModifierKeys
Keyboard::GetModifiers ()
{
	return modifiers;
}

void
Keyboard::SetModifiers (ModifierKeys m)
{
	modifiers = m;
}

void
Keyboard::OnKeyPress (Key key)
{
	if (!pressedKeys)
		pressedKeys = g_hash_table_new (g_direct_hash, g_direct_equal);
	
	g_hash_table_insert (pressedKeys, GINT_TO_POINTER (key), GINT_TO_POINTER (1));
	
	switch (key) {
	case KeyCTRL:
		modifiers = (ModifierKeys) (modifiers | ModifierKeyControl);
		break;
	case KeyALT:
		modifiers = (ModifierKeys) (modifiers | ModifierKeyAlt);
		break;
	case KeySHIFT:
		modifiers = (ModifierKeys) (modifiers | ModifierKeyShift);
		break;
	default:
		break;
	}
}

void
Keyboard::OnKeyRelease (Key key)
{
	if (!pressedKeys)
		return;
	
	g_hash_table_remove (pressedKeys, GINT_TO_POINTER (key));
	
	switch (key) {
	case KeyCTRL:
		modifiers = (ModifierKeys) (modifiers & ~ModifierKeyControl);
		break;
	case KeyALT:
		modifiers = (ModifierKeys) (modifiers & ~ModifierKeyAlt);
		break;
	case KeySHIFT:
		modifiers = (ModifierKeys) (modifiers & ~ModifierKeyShift);
		break;
	default:
		break;
	}
}

bool
Keyboard::IsKeyPressed (Key key)
{
	return pressedKeys && (g_hash_table_lookup (pressedKeys, GINT_TO_POINTER (key)) != NULL);
}

};
