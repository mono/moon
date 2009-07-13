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

#include <gdk/gdkkeysyms.h>

#include "keyboard.h"

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

Key
Keyboard::MapKeyValToKey (guint keyval)
{
	switch (keyval) {
	case GDK_BackSpace:				return KeyBACKSPACE;
	case GDK_Tab: case GDK_ISO_Left_Tab:		return KeyTAB;
	case GDK_Return: case GDK_KP_Enter:		return KeyENTER;
	case GDK_Shift_L: case GDK_Shift_R:		return KeySHIFT;
	case GDK_Control_L: case GDK_Control_R:		return KeyCTRL;
	case GDK_Alt_L: case GDK_Alt_R:			return KeyALT;
	case GDK_Caps_Lock:				return KeyCAPSLOCK;
	case GDK_Escape:				return KeyESCAPE;
	case GDK_space: case GDK_KP_Space:		return KeySPACE;
	case GDK_Page_Up: case GDK_KP_Page_Up:		return KeyPAGEUP;
	case GDK_Page_Down: case GDK_KP_Page_Down:	return KeyPAGEDOWN;
	case GDK_End: case GDK_KP_End:			return KeyEND;
	case GDK_Home: case GDK_KP_Home:		return KeyHOME;
	case GDK_Left: case GDK_KP_Left:		return KeyLEFT;
	case GDK_Up: case GDK_KP_Up:			return KeyUP;
	case GDK_Right: case GDK_KP_Right:		return KeyRIGHT;
	case GDK_Down: case GDK_KP_Down:		return KeyDOWN;
	case GDK_Insert: case GDK_KP_Insert:		return KeyINSERT;
	case GDK_Delete: case GDK_KP_Delete:		return KeyDELETE;
	case GDK_0:					return KeyDIGIT0;
	case GDK_1:					return KeyDIGIT1;
	case GDK_2:					return KeyDIGIT2;
	case GDK_3:					return KeyDIGIT3;
	case GDK_4:					return KeyDIGIT4;
	case GDK_5:					return KeyDIGIT5;
	case GDK_6:					return KeyDIGIT6;
	case GDK_7:					return KeyDIGIT7;
	case GDK_8:					return KeyDIGIT8;
	case GDK_9:					return KeyDIGIT9;
	case GDK_a: case GDK_A:				return KeyA;
	case GDK_b: case GDK_B:				return KeyB;
	case GDK_c: case GDK_C:				return KeyC;
	case GDK_d: case GDK_D:				return KeyD;
	case GDK_e: case GDK_E:				return KeyE;
	case GDK_f: case GDK_F:				return KeyF;
	case GDK_g: case GDK_G:				return KeyG;
	case GDK_h: case GDK_H:				return KeyH;
	case GDK_i: case GDK_I:				return KeyI;
	case GDK_j: case GDK_J:				return KeyJ;
	case GDK_k: case GDK_K:				return KeyK;
	case GDK_l: case GDK_L:				return KeyL;
	case GDK_m: case GDK_M:				return KeyM;
	case GDK_n: case GDK_N:				return KeyN;
	case GDK_o: case GDK_O:				return KeyO;
	case GDK_p: case GDK_P:				return KeyP;
	case GDK_q: case GDK_Q:				return KeyQ;
	case GDK_r: case GDK_R:				return KeyR;
	case GDK_s: case GDK_S:				return KeyS;
	case GDK_t: case GDK_T:				return KeyT;
	case GDK_u: case GDK_U:				return KeyU;
	case GDK_v: case GDK_V:				return KeyV;
	case GDK_w: case GDK_W:				return KeyW;
	case GDK_x: case GDK_X:				return KeyX;
	case GDK_y: case GDK_Y:				return KeyY;
	case GDK_z: case GDK_Z:				return KeyZ;
		
	case GDK_F1: case GDK_KP_F1:			return KeyF1;
	case GDK_F2: case GDK_KP_F2:			return KeyF2;
	case GDK_F3: case GDK_KP_F3:			return KeyF3;
	case GDK_F4: case GDK_KP_F4:			return KeyF4;
	case GDK_F5:					return KeyF5;
	case GDK_F6:					return KeyF6;
	case GDK_F7:					return KeyF7;
	case GDK_F8:					return KeyF8;
	case GDK_F9:					return KeyF9;
	case GDK_F10:					return KeyF10;
	case GDK_F11:					return KeyF11;
	case GDK_F12:					return KeyF12;
		
	case GDK_KP_0:					return KeyNUMPAD0;
	case GDK_KP_1:					return KeyNUMPAD1;
	case GDK_KP_2:					return KeyNUMPAD2;
	case GDK_KP_3:					return KeyNUMPAD3;
	case GDK_KP_4:					return KeyNUMPAD4;
	case GDK_KP_5:					return KeyNUMPAD5;
	case GDK_KP_6:					return KeyNUMPAD6;
	case GDK_KP_7:					return KeyNUMPAD7;
	case GDK_KP_8:					return KeyNUMPAD8;
	case GDK_KP_9:					return KeyNUMPAD9;
		
	case GDK_KP_Multiply: case GDK_asterisk:	return KeyMULTIPLY;
	case GDK_KP_Add: case GDK_plus:			return KeyADD;
	case GDK_KP_Subtract: case GDK_minus:		return KeySUBTRACT;
	case GDK_KP_Decimal: case GDK_period:  		return KeyDECIMAL;
	case GDK_KP_Divide: case GDK_slash:		return KeyDIVIDE;
		
	default:
		return KeyUNKNOWN;
	}
}
