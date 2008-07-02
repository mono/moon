/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * windowless.cpp: Windowsless Surface subclass
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <config.h>

#include "windowless.h"

#define Visual _XxVisual
#define Region _XxRegion
#include <gdk/gdkx.h>
#undef Visual
#undef Region

#ifdef DEBUG
#define d(x) x
#else
#define d(x)
#endif

WindowlessSurface::WindowlessSurface (int width, int height, PluginInstance *plugin)
  : Surface (width, height, true)
{
	this->plugin = plugin;
	SetTrans (true);
}

void
WindowlessSurface::SetCursor (GdkCursor *cursor)
{
	// turned off for now.  hopefully we can get this switched on for
	// newer versions of ff3
	// see https://bugzilla.mozilla.org/show_bug.cgi?id=430451

#if 0 && (NP_VERSION_MINOR >= NPVERS_HAS_CURSOR)
	NPN_SetValue (plugin->GetInstance(), NPNVcursor, GDK_CURSOR_XCURSOR(cursor));
#endif
}

void
WindowlessSurface::Invalidate (Rect r)
{
	NPRect nprect;

	// Mozilla gets seriously confused about invalidations 
	// outside the windowless bounds.
	r = r.Intersection (Rect (0, 0, GetWidth(), GetHeight())).RoundOut ();

	nprect.left = (uint16)r.x;
	nprect.top = (uint16)r.y;
	nprect.right = (uint16)(r.x + r.w);
	nprect.bottom = (uint16)(r.y + r.h);

	NPN_InvalidateRect (plugin->GetInstance(), &nprect);
}

void
WindowlessSurface::ProcessUpdates ()
{
	NPN_ForceRedraw (plugin->GetInstance());
}

gboolean
WindowlessSurface::HandleEvent (XEvent *event)
{
	XEvent *xev = (XEvent*)event;
	gboolean handled = FALSE;

	switch (xev->type) {
	case GraphicsExpose: {

		GdkDrawable *drawable = gdk_pixmap_foreign_new ((GdkNativeWindow)xev->xgraphicsexpose.drawable);
		if (!drawable) {
			drawable = gdk_window_foreign_new ((GdkNativeWindow)xev->xgraphicsexpose.drawable);
		}

		if (drawable) {
			NPWindow *window = plugin->GetWindow();
			NPSetWindowCallbackStruct *ws_info = (NPSetWindowCallbackStruct*)window->ws_info;
			GdkVisual *visual = gdkx_visual_get (ws_info->visual->visualid);

			if (visual) {
				GdkEventExpose expose;

				expose.type = GDK_EXPOSE;
				expose.window = NULL;
				expose.send_event = FALSE;
				expose.area = Rect (xev->xgraphicsexpose.x,
						    xev->xgraphicsexpose.y,
						    xev->xgraphicsexpose.width,
						    xev->xgraphicsexpose.height).ToGdkRectangle ();
				/* XXX ugh */
				expose.region = gdk_region_rectangle (&expose.area);

				handled = expose_to_drawable (drawable, visual, &expose, window->x, window->y);

				gdk_region_destroy (expose.region);
			} else {
				d(printf ("no gdk visual\n"));
			}
			
			g_object_unref (drawable);
		} else {
			d(printf ("no gdk drawable\n"));
		}
		break;
	}
	case MotionNotify: {
		GdkEventMotion motion;
		
		motion.type = GDK_MOTION_NOTIFY;
		motion.window = NULL;
		motion.send_event = xev->xmotion.send_event;
		motion.x = xev->xmotion.x;
		motion.y = xev->xmotion.y;
		motion.axes = NULL;
		motion.state = xev->xmotion.state;
		motion.is_hint = xev->xmotion.is_hint;
		motion.device = NULL; // XXX
		motion.x_root = xev->xmotion.x_root;
		motion.y_root = xev->xmotion.y_root;

		handled = Surface::motion_notify_callback (NULL, &motion, this);
		break;
	}
	case ButtonPress:
	case ButtonRelease: {
		GdkEventButton button;

		button.type = xev->type == ButtonPress ? GDK_BUTTON_PRESS : GDK_BUTTON_RELEASE;
		button.window = NULL;
		button.send_event = xev->xbutton.send_event;
		button.time = xev->xbutton.time;
		button.x = xev->xbutton.x;
		button.y = xev->xbutton.y;
		button.x_root = xev->xbutton.x_root;
		button.y_root = xev->xbutton.y_root;
		button.state = xev->xbutton.state;
		button.button = xev->xbutton.button;
		button.axes = NULL;

		if (xev->type == ButtonPress)
			handled = PluginInstance::plugin_button_press_callback (NULL, &button, this);
		if (!handled) {
			if (xev->type == ButtonPress)
				handled = Surface::button_press_callback (NULL, &button, this);
			else
				handled = Surface::button_release_callback (NULL, &button, this);
		}
		break;
	}
	case KeyPress:
	case KeyRelease: {
		GdkEventKey key;

		key.type = xev->type == KeyPress ? GDK_KEY_PRESS : GDK_KEY_RELEASE;
		key.window = NULL;
		key.send_event = xev->xkey.send_event;
		key.time = xev->xkey.time;
		key.state = xev->xkey.state;
		key.hardware_keycode = xev->xkey.keycode;

		gint effective_group;

		gdk_keymap_translate_keyboard_state (gdk_keymap_get_default (),
						     xev->xkey.keycode,
						     (GdkModifierType)xev->xkey.state, // XXX
						     0, // XXX
						     &key.keyval,
						     &effective_group,
						     NULL,
						     NULL);

		key.group = (guint8)effective_group;

		if (xev->type == KeyPress)
			handled = Surface::key_press_callback (NULL, &key, this);
		else
			handled = Surface::key_release_callback (NULL, &key, this);

		break;
	}
	case EnterNotify:
	case LeaveNotify: {
		GdkEventCrossing crossing;

		crossing.type = xev->type == EnterNotify ? GDK_ENTER_NOTIFY : GDK_LEAVE_NOTIFY;
		crossing.window = crossing.subwindow = NULL;
		crossing.send_event = xev->xcrossing.send_event;
		crossing.time = xev->xcrossing.time;
		crossing.x = xev->xcrossing.x;
		crossing.y = xev->xcrossing.y;
		crossing.x_root = xev->xcrossing.x_root;
		crossing.y_root = xev->xcrossing.y_root;
		crossing.mode = (GdkCrossingMode)xev->xcrossing.mode; // XXX
		crossing.detail = (GdkNotifyType)xev->xcrossing.detail; // XXX
		crossing.focus = xev->xcrossing.focus;
		crossing.state = xev->xcrossing.state;

		Surface::crossing_notify_callback (NULL, &crossing, this);
		break;
	}
	case FocusIn: {
		Surface::focus_in_callback (NULL, NULL, this);
		break;
	}

	case FocusOut: {
		Surface::focus_out_callback (NULL, NULL, this);
		break;
	}
	default:
		d(printf ("Unhandled Xlib event %d\n", xev->type));
		break;
	}

	return handled;
}
