/*
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * Author:
 *   Jackson Harper (jackson@ximian.com)
 *
 * Copyright 2007-2008 Novell, Inc. (http://www.novell.com)
 *
 *
 * input.cpp: Simulate user input.
 *
 */



#include <math.h>
#include <unistd.h>

#define XK_MISCELLANY
#define XK_LATIN1
#include <X11/keysymdef.h>

#include <gdk/gdk.h>

#include "input.h"



#define XSCREEN_OF_POINTER -1

#define MOVE_MOUSE_LOGARITHMIC_INTERVAL  100000

#define MOUSE_IS_AT_POSITION_TOLERANCE	 2




InputProvider::InputProvider () : display (NULL), root_window (NULL), xtest_available (false), down_keys (NULL)
{
	display = XOpenDisplay (NULL);

	if (!display) {
		printf ("Unable to open XDisplay, input tests will not run.\n");
		return;
	}

	// I guess we'll assume there is only one screen
	root_window = XRootWindow (display, 0);

	if (root_window <= 0) {
		printf ("Unable to get the root window, some of the input tests will not run.\n");
		return;
	}

	int event_base, error_base, majorp, minorp;
	if (!XTestQueryExtension (display, &event_base, &error_base, &majorp, &minorp)) {
		printf ("XTEST Extension unavailable, input tests will not run.\n");
		return;
	}

	xtest_available = true;
}

InputProvider::~InputProvider ()
{
	while (down_keys)
		SendKeyInput (GPOINTER_TO_UINT (down_keys->data), false);

	g_slist_free (down_keys);
}

void
InputProvider::MoveMouse (int x, int y)
{
	g_assert (xtest_available);
	g_assert (display);

#if INTERMEDIATE_MOTION
	int current_x;
	int current_y;

	GetCursorPos (current_x, current_y);

	while (current_x != x || current_y != y) {
		XTestFakeMotionEvent (display, XSCREEN_OF_POINTER, 
				      current_x -= (current_x != x ? (current_x > x ? 1 : -1) : 0),
				      current_y -= (current_y != y ? (current_y > y ? 1 : -1) : 0),
				      CurrentTime);
		XFlush (display);
	}
#else
	XTestFakeMotionEvent (display, XSCREEN_OF_POINTER, x, y, CurrentTime);
	XFlush (display);
#endif 
}

void
InputProvider::MoveMouseLogarithmic (int x, int y)
{
	g_assert (xtest_available);
	g_assert (display);

	int current_x;
	int current_y;

	GetCursorPos (current_x, current_y);
	
	while (current_x != x || current_y != y) {
		current_x += (current_x < x ? 1.0 : -1.0 ) * 2.0 * log (1 + abs (current_x - x));
		current_y += (current_y < y ? 1.0 : -1.0 ) * 2.0 * log (1 + abs (current_y - y));

		XTestFakeMotionEvent (display, XSCREEN_OF_POINTER, current_x, current_y, CurrentTime);
		XFlush (display);

		usleep (MOVE_MOUSE_LOGARITHMIC_INTERVAL);
	}
}

bool
InputProvider::MouseIsAtPosition (int x, int y)
{
	int cur_x, cur_y;

	x = MAX (x, 0);
	y = MAX (y, 0);

	GetCursorPos (cur_x, cur_y);

	int delta = MAX (abs (cur_x - x), abs (cur_y - y));
	if (delta <= MOUSE_IS_AT_POSITION_TOLERANCE)
		return true;
	return false;
}

void
InputProvider::MouseLeftClick ()
{
	g_assert (xtest_available);
	g_assert (display);

	XTestFakeButtonEvent (display, 1, true, CurrentTime);
	XFlush (display);

	XTestFakeButtonEvent (display, 1, false, CurrentTime);
	XFlush (display);
}

void
InputProvider::MouseRightClick ()
{
	g_assert (xtest_available);
	g_assert (display);

	XTestFakeButtonEvent (display, 3, true, CurrentTime);
	XFlush (display);

	XTestFakeButtonEvent (display, 3, false, CurrentTime);
	XFlush (display);
}

void
InputProvider::MouseLeftButtonDown ()
{
	g_assert (xtest_available);
	g_assert (display);

	XTestFakeButtonEvent (display, 1, true, CurrentTime);
	XFlush (display);
}

void
InputProvider::MouseLeftButtonUp ()
{
	g_assert (xtest_available);
	g_assert (display);

	XTestFakeButtonEvent (display, 1, false, CurrentTime);
	XFlush (display);
}

void
InputProvider::SendKeyInput (uint32 keysym, bool key_down)
{
	g_assert (display);
	g_assert (xtest_available);

	int mapped = MapToKeysym (keysym);
	int keycode = XKeysymToKeycode (display, mapped);

	XTestFakeKeyEvent (display, keycode, key_down, CurrentTime);
	XFlush (display);

	if (key_down) {
		if (!g_slist_find (down_keys, GUINT_TO_POINTER (keysym)))
			down_keys = g_slist_append (down_keys, GUINT_TO_POINTER (keysym));
	} else
		down_keys = g_slist_remove (down_keys, GUINT_TO_POINTER (keysym));
}

void
InputProvider::GetCursorPos (int &x, int &y)
{
	g_assert (display);
	g_assert (root_window > 0);

	Window root_return, child_return;
	int x_win, y_win;
	unsigned int mask;

	XQueryPointer (display, root_window, &root_return, &child_return, &x, &y, &x_win, &y_win, &mask);
}

enum VirtualKeys {
    VK_LBUTTON       = 0x01,
    VK_RBUTTON       = 0x02,
    VK_CANCEL        = 0x03,
    VK_MBUTTON       = 0x04,
    VK_XBUTTON1      = 0x05,
    VK_XBUTTON2      = 0x06,
    VK_BACK          = 0x08,
    VK_TAB           = 0x09,
    VK_CLEAR         = 0x0C,
    VK_RETURN        = 0x0D,
    VK_SHIFT         = 0x10,
    VK_CONTROL       = 0x11,
    VK_MENU          = 0x12,
    VK_PAUSE         = 0x13,
    VK_CAPITAL       = 0x14,
    VK_KANA          = 0x15,
    VK_HANGEUL       = 0x15,
    VK_HANGUL        = 0x15,
    VK_JUNJA         = 0x17,
    VK_FINAL         = 0x18,
    VK_HANJA         = 0x19,
    VK_KANJI         = 0x19,
    VK_ESCAPE        = 0x1B,
    VK_CONVERT       = 0x1C,
    VK_NONCONVERT    = 0x1D,
    VK_ACCEPT        = 0x1E,
    VK_MODECHANGE    = 0x1F,
    VK_SPACE         = 0x20,
    VK_PRIOR         = 0x21,
    VK_NEXT          = 0x22,
    VK_END           = 0x23,
    VK_HOME          = 0x24,
    VK_LEFT          = 0x25,
    VK_UP            = 0x26,
    VK_RIGHT         = 0x27,
    VK_DOWN          = 0x28,
    VK_SELECT        = 0x29,
    VK_PRINT         = 0x2A,
    VK_EXECUTE       = 0x2B,
    VK_SNAPSHOT      = 0x2C,
    VK_INSERT        = 0x2D,
    VK_DELETE        = 0x2E,
    VK_HELP          = 0x2F,
    VK_0             = 0x30,
    VK_1             = 0x31,
    VK_2             = 0x32,
    VK_3             = 0x33,
    VK_4             = 0x34,
    VK_5             = 0x35,
    VK_6             = 0x36,
    VK_7             = 0x37,
    VK_8             = 0x38,
    VK_9             = 0x39,
    VK_A             = 0x41,
    VK_B             = 0x42,
    VK_C             = 0x43,
    VK_D             = 0x44,
    VK_E             = 0x45,
    VK_F             = 0x46,
    VK_G             = 0x47,
    VK_H             = 0x48,
    VK_I             = 0x49,
    VK_J             = 0x4A,
    VK_K             = 0x4B,
    VK_L             = 0x4C,
    VK_M             = 0x4D,
    VK_N             = 0x4E,
    VK_O             = 0x4F,
    VK_P             = 0x50,
    VK_Q             = 0x51,
    VK_R             = 0x52,
    VK_S             = 0x53,
    VK_T             = 0x54,
    VK_U             = 0x55,
    VK_V             = 0x56,
    VK_W             = 0x57,
    VK_X             = 0x58,
    VK_Y             = 0x59,
    VK_Z             = 0x5A,
    VK_LWIN          = 0x5B,
    VK_RWIN          = 0x5C,
    VK_APPS          = 0x5D,
    VK_SLEEP         = 0x5F,
    VK_NUMPAD0       = 0x60,
    VK_NUMPAD1       = 0x61,
    VK_NUMPAD2       = 0x62,
    VK_NUMPAD3       = 0x63,
    VK_NUMPAD4       = 0x64,
    VK_NUMPAD5       = 0x65,
    VK_NUMPAD6       = 0x66,
    VK_NUMPAD7       = 0x67,
    VK_NUMPAD8       = 0x68,
    VK_NUMPAD9       = 0x69,
    VK_MULTIPLY      = 0x6A,
    VK_ADD           = 0x6B,
    VK_SEPARATOR     = 0x6C,
    VK_SUBTRACT      = 0x6D,
    VK_DECIMAL       = 0x6E,
    VK_DIVIDE        = 0x6F,
    VK_F1            = 0x70,
    VK_F2            = 0x71,
    VK_F3            = 0x72,
    VK_F4            = 0x73,
    VK_F5            = 0x74,
    VK_F6            = 0x75,
    VK_F7            = 0x76,
    VK_F8            = 0x77,
    VK_F9            = 0x78,
    VK_F10           = 0x79,
    VK_F11           = 0x7A,
    VK_F12           = 0x7B,
    VK_F13           = 0x7C,
    VK_F14           = 0x7D,
    VK_F15           = 0x7E,
    VK_F16           = 0x7F,
    VK_F17           = 0x80,
    VK_F18           = 0x81,
    VK_F19           = 0x82,
    VK_F20           = 0x83,
    VK_F21           = 0x84,
    VK_F22           = 0x85,
    VK_F23           = 0x86,
    VK_F24           = 0x87,
    VK_NUMLOCK       = 0x90,
    VK_SCROLL        = 0x91,
    VK_LSHIFT        = 0xA0,
    VK_RSHIFT        = 0xA1,
    VK_LCONTROL      = 0xA2,
    VK_RCONTROL      = 0xA3,
    VK_LMENU         = 0xA4,
    VK_RMENU         = 0xA5,
    VK_OEM_1         = 0xBA,
    VK_OEM_PLUS      = 0xBB,
    VK_OEM_COMMA     = 0xBC,
    VK_OEM_MINUS     = 0xBD,
    VK_OEM_PERIOD    = 0xBE,
    VK_OEM_2         = 0xBF,
    VK_OEM_3         = 0xC0
};


int
InputProvider::MapToKeysym (int key)
{
	int res = key;

	switch (key) {

	case VK_CANCEL: res = XK_Cancel; break;
	case VK_BACK: res = XK_BackSpace; break;
	case VK_TAB: res = XK_Tab; break;
	case VK_CLEAR: res = XK_Clear; break;
	case VK_RETURN: res = XK_Return; break;
	case VK_SHIFT: res = XK_Shift_L; break;
	case VK_CONTROL: res = XK_Control_L; break;
	case VK_MENU: res = XK_Menu; break;
	case VK_PAUSE: res = XK_Pause; break;
	case VK_CAPITAL: res = XK_Caps_Lock; break;

		/*
	case VK_KANA:
    	case VK_HANGEUL:
    	case VK_HANGUL:
    	case VK_JUNJA:
    	case VK_FINAL:
    	case VK_HANJA:
    	case VK_KANJI:
		*/

	case VK_ESCAPE: res = XK_Escape; break;
//	case VK_CONVERT:	
//	case VK_NONCONVERT:
//	case VK_ACCEPT:

	case VK_MODECHANGE: res = XK_Mode_switch; break;
	case VK_SPACE: res = XK_space; break;
	case VK_PRIOR: res = XK_Prior; break;
	case VK_NEXT: res = XK_Next; break;
	case VK_END: res = XK_End; break;
	case VK_HOME: res = XK_Home; break;
	case VK_LEFT: res = XK_Left; break;
	case VK_UP: res = XK_Up; break;
	case VK_RIGHT: res = XK_Right; break;
	case VK_DOWN: res = XK_Down; break;
	case VK_SELECT: res = XK_Select; break;
	case VK_PRINT: res = XK_Print; break;
	case VK_EXECUTE: res = XK_Execute; break;
	case VK_SNAPSHOT: res = XK_Print; break;
	case VK_INSERT: res = XK_Insert; break;
	case VK_DELETE: res = XK_Delete; break;
	case VK_HELP: res = XK_Help; break;
	case VK_0: res = XK_0; break;
	case VK_1: res = XK_1; break;
	case VK_2: res = XK_2; break;
	case VK_3: res = XK_3; break;
	case VK_4: res = XK_4; break;
	case VK_5: res = XK_5; break;
	case VK_6: res = XK_6; break;
	case VK_7: res = XK_7; break;
	case VK_8: res = XK_8; break;
	case VK_9: res = XK_9; break;
	case VK_A: res = XK_A; break;
	case VK_B: res = XK_B; break;
	case VK_C: res = XK_C; break;
	case VK_D: res = XK_D; break;
	case VK_E: res = XK_E; break;
	case VK_F: res = XK_F; break;
	case VK_G: res = XK_G; break;
	case VK_H: res = XK_H; break;
	case VK_I: res = XK_I; break;
	case VK_J: res = XK_J; break;
	case VK_K: res = XK_K; break;
	case VK_L: res = XK_L; break;
	case VK_M: res = XK_M; break;
	case VK_N: res = XK_N; break;
	case VK_O: res = XK_O; break;
	case VK_P: res = XK_P; break;
	case VK_Q: res = XK_Q; break;
	case VK_R: res = XK_R; break;
	case VK_S: res = XK_S; break;
	case VK_T: res = XK_T; break;
	case VK_U: res = XK_U; break;
	case VK_V: res = XK_V; break;
	case VK_W: res = XK_W; break;
	case VK_X: res = XK_X; break;
	case VK_Y: res = XK_Y; break;
	case VK_Z: res = XK_Z; break;
/*
    case VK_LWIN          = 0x5B,
    case VK_RWIN          = 0x5C,
    case VK_APPS          = 0x5D,

    case VK_SLEEP         = 0x5F,
*/
	case VK_NUMPAD0: res = XK_KP_0; break;
	case VK_NUMPAD1: res = XK_KP_1; break;
	case VK_NUMPAD2: res = XK_KP_2; break;
	case VK_NUMPAD3: res = XK_KP_3; break;
	case VK_NUMPAD4: res = XK_KP_4; break;
	case VK_NUMPAD5: res = XK_KP_5; break;
	case VK_NUMPAD6: res = XK_KP_6; break;
	case VK_NUMPAD7: res = XK_KP_7; break;
	case VK_NUMPAD8: res = XK_KP_8; break;
	case VK_NUMPAD9: res = XK_KP_9; break;
	case VK_MULTIPLY: res = XK_KP_Multiply; break;
	case VK_ADD: res = XK_KP_Add; break;
	case VK_SEPARATOR: res = XK_KP_Separator; break;
	case VK_SUBTRACT: res = XK_KP_Subtract; break;
	case VK_DECIMAL: res = XK_KP_Decimal; break;
	case VK_DIVIDE: res = XK_KP_Divide; break;
	case VK_F1: res = XK_F1; break;
	case VK_F2: res = XK_F2; break;
	case VK_F3: res = XK_F3; break;
	case VK_F4: res = XK_F4; break;
	case VK_F5: res = XK_F5; break;
	case VK_F6: res = XK_F6; break;
	case VK_F7: res = XK_F7; break;
	case VK_F8: res = XK_F8; break;
	case VK_F9: res = XK_F9; break;
	case VK_F10: res = XK_F10; break;
	case VK_F11: res = XK_F11; break;
	case VK_F12: res = XK_F12; break;
	case VK_F13: res = XK_F13; break;
	case VK_F14: res = XK_F14; break;
	case VK_F15: res = XK_F15; break;
	case VK_F16: res = XK_F16; break;
	case VK_F17: res = XK_F17; break;
	case VK_F18: res = XK_F18; break;
	case VK_F19: res = XK_F19; break;
	case VK_F20: res = XK_F20; break;
	case VK_F21: res = XK_F21; break;
	case VK_F22: res = XK_F22; break;
	case VK_F23: res = XK_F23; break;
	case VK_F24: res = XK_F24; break;

	case VK_NUMLOCK: res = XK_Num_Lock; break;
	case VK_SCROLL: res = XK_Scroll_Lock;
	case VK_LSHIFT: res = XK_Shift_L; break;
	case VK_RSHIFT: res = XK_Shift_R; break;
	case VK_LCONTROL: res = XK_Control_L; break;
	case VK_RCONTROL: res = XK_Control_R; break;

		/*
	case VK_LMENU:
	case VK_RMENU:
	case VK_OEM_1:
		*/
		
	case VK_OEM_PLUS: res = XK_plus; break;
	case VK_OEM_COMMA: res = XK_comma; break;
	case VK_OEM_MINUS: res = XK_minus; break;
	case VK_OEM_PERIOD: res = XK_period; break;

		/*
	case VK_OEM_2:
	case VK_OEM_3:
		*/

	}

	return res;
}

