/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * eventargs.cpp: specialized code for dealing with mouse/stylus/keyboard event args.
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gdk/gdkkeysyms.h>

#include "eventargs.h"
#include "uielement.h"
#include "collection.h"
#include "stylus.h"
#include "runtime.h"

RoutedEventArgs::RoutedEventArgs ()
{
	source = NULL;
}

RoutedEventArgs::~RoutedEventArgs ()
{
	if (source)
		source->unref ();
}

void
RoutedEventArgs::SetSource (DependencyObject *el)
{
	if (source)
		source->unref();
	source = el;
	if (source)
		source->ref();
}

MouseEventArgs::MouseEventArgs (GdkEvent *event)
{
	this->event = gdk_event_copy (event);
	handled = false;
}

MouseEventArgs::MouseEventArgs ()
{
	event = gdk_event_new (GDK_MOTION_NOTIFY);
	handled = false;
}

MouseEventArgs::~MouseEventArgs ()
{
	gdk_event_free (event);
}

int
MouseEventArgs::GetState ()
{
	GdkModifierType state;
	gdk_event_get_state (event, &state);
	return (int)state;
}

void
MouseEventArgs::GetPosition (UIElement *relative_to, double *x, double *y)
{
	*x = *y = 0.0;
	if (gdk_event_get_coords (event, x, y)) {
		if (relative_to) {
			// FIXME this a nasty place to do this we should be able to
			// reduce the problem for this kind of hit testing.
			if (relative_to->GetSurface() &&
			    relative_to->GetSurface()->IsAnythingDirty())
				relative_to->GetSurface()->ProcessDirtyElements ();


			uielement_transform_point (relative_to, x, y);
		}
	}
}

StylusInfo*
MouseEventArgs::GetStylusInfo ()
{
	TabletDeviceType type = TabletDeviceTypeMouse;
	bool is_inverted = false;
	GdkDevice *gdk_device;

	switch (event->type) {
	case GDK_MOTION_NOTIFY:
		gdk_device = ((GdkEventMotion*)event)->device;
		break;
	case GDK_BUTTON_PRESS:
	case GDK_BUTTON_RELEASE:
		gdk_device = ((GdkEventButton*)event)->device;
		break;

	default:
	case GDK_ENTER_NOTIFY:
	case GDK_LEAVE_NOTIFY:
		/* GdkEventCrossing doesn't have a device field.  ugh */
		gdk_device = NULL;
		break;
	}

	if (gdk_device) {
		switch (gdk_device->source) {
		case GDK_SOURCE_PEN:
		case GDK_SOURCE_ERASER:
			type = TabletDeviceTypeStylus;
			break;
		case GDK_SOURCE_MOUSE:
		case GDK_SOURCE_CURSOR: /* XXX not sure where to lump this in..  in the stylus block? */
		default:
			type = TabletDeviceTypeMouse;
			break;
		}

		is_inverted = (gdk_device->source == GDK_SOURCE_ERASER);
	}

	StylusInfo *info = new StylusInfo ();

	info->SetValue (StylusInfo::DeviceTypeProperty, Value (type));
	info->SetValue (StylusInfo::IsInvertedProperty, Value (is_inverted));

	return info;
}

StylusPointCollection*
MouseEventArgs::GetStylusPoints (UIElement *ink_presenter)
{
	StylusPointCollection *points = new StylusPointCollection ();
	double pressure;
	double x, y;
	
	GetPosition (ink_presenter, &x, &y);
	if (!((GdkEventMotion *) event)->device || !gdk_event_get_axis (event, GDK_AXIS_PRESSURE, &pressure))
		pressure = 0.0;
	
	StylusPoint *point = new StylusPoint ();
	point->SetValue (StylusPoint::XProperty, Value(x));
	point->SetValue (StylusPoint::YProperty, Value(y));
	point->SetValue (StylusPoint::PressureFactorProperty, Value(pressure));

	points->Add (point);

	point->unref ();

	return points;
}

ModifierKeys Keyboard::Modifiers = ModifierKeyNone;

ModifierKeys
keyboard_get_modifiers (void)
{
	return Keyboard::Modifiers;
}

KeyEventArgs::KeyEventArgs (GdkEventKey *event)
{
	this->event = (GdkEventKey *) gdk_event_copy ((GdkEvent *) event);
	handled = false;
}

KeyEventArgs::KeyEventArgs ()
{
	event = (GdkEventKey *) gdk_event_new (GDK_KEY_PRESS);
	handled = false;
}

KeyEventArgs::~KeyEventArgs ()
{
	gdk_event_free ((GdkEvent *) event);
}

int
KeyEventArgs::GetState ()
{
	GdkModifierType state;
	gdk_event_get_state ((GdkEvent *) event, &state);
	return (int)state;
}

Key
KeyEventArgs::GetKey ()
{
	return gdk_keyval_to_key (event->keyval);
}

int
KeyEventArgs::GetPlatformKeyCode ()
{
	return event->hardware_keycode;
}

Key
KeyEventArgs::gdk_keyval_to_key (guint keyval)
{
	switch (keyval) {
	case GDK_BackSpace:				return KeyBACKSPACE;
	case GDK_Tab:					return KeyTAB;
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
	case GDK_Delete: case GDK_KP_Delete:		return KeyINSERT;
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
	  
	case GDK_KP_Multiply:				return KeyMULTIPLY;
	case GDK_KP_Add:				return KeyADD;
	case GDK_KP_Subtract:				return KeySUBTRACT;
	case GDK_KP_Decimal:				return KeyDECIMAL;
	case GDK_KP_Divide:				return KeyDIVIDE;
	  
	default:
		return KeyUNKNOWN;
	}
}
