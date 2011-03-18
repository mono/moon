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

struct ModifierMap {
	MoonModifier mod;
	ModifierKeys key;
} modifier_map[] = {
	{ MoonModifier_Mod1,    ModifierKeyAlt     },
	{ MoonModifier_Control, ModifierKeyControl },
	{ MoonModifier_Shift,   ModifierKeyShift   },
	{ MoonModifier_Meta,    ModifierKeyWindows },
};

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
Keyboard::OnKeyPress (MoonKeyEvent *key)
{
	MoonModifier mods = key->GetModifiers ();
	
	if (!pressedKeys)
		pressedKeys = g_hash_table_new (g_direct_hash, g_direct_equal);
	
	g_hash_table_insert (pressedKeys, GINT_TO_POINTER (key->GetSilverlightKey ()), GINT_TO_POINTER (1));
	
	modifiers = ModifierKeyNone;
	for (guint i = 0; i < G_N_ELEMENTS (modifier_map); i++) {
		if (modifier_map[i].mod & mods)
			modifiers = (ModifierKeys) (modifiers | modifier_map[i].key);
	}
}

void
Keyboard::OnKeyRelease (MoonKeyEvent *key)
{
	MoonModifier mods = key->GetModifiers ();
	
	if (!pressedKeys)
		return;
	
	g_hash_table_remove (pressedKeys, GINT_TO_POINTER (key->GetSilverlightKey ()));
	
	modifiers = ModifierKeyNone;
	for (guint i = 0; i < G_N_ELEMENTS (modifier_map); i++) {
		if (modifier_map[i].mod & mods)
			modifiers = (ModifierKeys) (modifiers | modifier_map[i].key);
	}
}

bool
Keyboard::IsKeyPressed (MoonKeyEvent *key)
{
	return pressedKeys && (g_hash_table_lookup (pressedKeys, GINT_TO_POINTER (key->GetSilverlightKey ())) != NULL);
}

};
