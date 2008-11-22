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

MoonWindowless::MoonWindowless (int width, int height, PluginInstance *plugin)
	: MoonWindow (width, height)
{
	this->plugin = plugin;

	UpdateWindowInfo ();
}

void
MoonWindowless::UpdateWindowInfo ()
{
	// It appears opera doesn't do a good job of keeping the NPWindow members valid
	// between SetWindow calls so we have to work around this by copying the members
	// we need.  This is really ugly.

	NPWindow *window = plugin->GetWindow();
	NPSetWindowCallbackStruct *ws_info = (NPSetWindowCallbackStruct*)window->ws_info;
	visualid = ws_info->visual ? visualid = ws_info->visual->visualid : 0;
	x = window->x;
	y = window->y;
}

void
MoonWindowless::Resize (int width, int height)
{
	bool emit_resize = false;

	UpdateWindowInfo ();

        if (this->width != width || this->height != height) {
		this->width = width;
		this->height = height;
		
		emit_resize = true;
	}

	surface->HandleUIWindowAllocation (emit_resize);
}

void
MoonWindowless::SetCursor (MouseCursor cursor)
{
#if (NP_VERSION_MINOR >= NPVERS_HAS_WINDOWLESS_CURSORS)
	NPCursor npcursor;
	switch (cursor) {
	case MouseCursorDefault:
		npcursor = NPCursorAuto;
		break;
	case MouseCursorArrow:
		npcursor = NPCursorPointer;
		break;
	case MouseCursorWait:
		npcursor = NPCursorWait;
		break;
	case MouseCursorIBeam:
		npcursor = NPCursorText;
		break;
	case MouseCursorStylus:
		npcursor = NPCursorPointer; // XXX ugh...
		break;
	case MouseCursorEraser:
		npcursor = NPCursorPointer; // XXX ugh...
		break;
	case MouseCursorNone:
		// Silverlight display no cursor if the enumeration value is invalid (e.g. -1)
	default:
		npcursor = NPCursorNone;
		break;
	}

	NPN_SetValue (plugin->GetInstance(), NPPVcursor, (void*)npcursor);
#endif
}

void
MoonWindowless::Invalidate (Rect r)
{
	NPRect nprect;

	// Mozilla gets seriously confused about invalidations 
	// outside the windowless bounds.
	r = r.Intersection (Rect (0, 0, GetWidth(), GetHeight())).RoundOut ();

	nprect.left = (uint16_t)r.x;
	nprect.top = (uint16_t)r.y;
	nprect.right = (uint16_t)(r.x + r.width);
	nprect.bottom = (uint16_t)(r.y + r.height);

	NPN_InvalidateRect (plugin->GetInstance(), &nprect);
}

void
MoonWindowless::ProcessUpdates ()
{
	//NPN_ForceRedraw (plugin->GetInstance());
}

gboolean
MoonWindowless::HandleEvent (XEvent *event)
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
			GdkVisual *visual = gdkx_visual_get (visualid);

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

				expose.area.x = expose.area.y = 0;

				surface->PaintToDrawable (drawable, visual, &expose, x, y, GetTransparent(), false);
				handled = TRUE;

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

		handled = surface->HandleUIMotion (&motion);
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
			handled = PluginInstance::plugin_button_press_callback (NULL, &button, plugin);
		if (!handled) {
			if (xev->type == ButtonPress)
				handled = surface->HandleUIButtonPress (&button);
			else
				handled = surface->HandleUIButtonRelease (&button);
		}
		break;
	}
	case KeyPress:
	case KeyRelease: {
		GdkEventKey key;

		key.type = xev->type == KeyPress ? GDK_KEY_PRESS : GDK_KEY_RELEASE;
		key.string = NULL;
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
			handled = surface->HandleUIKeyPress (&key);
		else
			handled = surface->HandleUIKeyRelease (&key);

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
		// crossing.mode = (GdkCrossingMode)xev->xcrossing.mode; // XXX
		crossing.mode = GDK_CROSSING_NORMAL;
		crossing.detail = (GdkNotifyType)xev->xcrossing.detail; // XXX
		crossing.focus = xev->xcrossing.focus;
		crossing.state = xev->xcrossing.state;

		surface->HandleUICrossing (&crossing);
		break;
	}
	case FocusIn: {
		surface->HandleUIFocusIn (NULL);
		break;
	}

	case FocusOut: {
		surface->HandleUIFocusOut (NULL);
		break;
	}
	default:
		d(printf ("Unhandled Xlib event %d\n", xev->type));
		break;
	}

	return handled;
}

void
MoonWindowless::Show ()
{
	// nothing needed here
}

void
MoonWindowless::Hide ()
{
	// nothing needed here
}

void
MoonWindowless::EnableEvents (bool first)
{
	// nothing needed here, NPAPI pushes events through
	// HandleEvent.
}

void
MoonWindowless::DisableEvents ()
{
	// nothing needed here, NPAPI pushes events through
	// HandleEvent.
}

void
MoonWindowless::GrabFocus ()
{
	// we can't grab focus - the browser handles that.
}

bool
MoonWindowless::HasFocus ()
{
	// XXX maybe we should track the focus in/out events?
	return false;
}

void
MoonWindowless::SetSurface (Surface *s)
{
	MoonWindow::SetSurface (s);
	s->HandleUIWindowAvailable ();
}

GdkWindow*
MoonWindowless::GetGdkWindow ()
{
	/* GdkNativeWindow is a CARD32 on X11.  gcc/netscape with -fstack-protector-all has a
	 * bug on amd64 where it compiles the following code:
	 * 
	 * GdkNativeWindow window;
	 * NPN_GetValue (plugin->GetInstance(), NPNVnetscapeWindow, (void*)&window);
	 * 
	 * and emits assembly like: 
	 *      mov    %fs:0x28,%rax   [calculate the stack cookie and put it in $rax]
	 *      mov    %rax,-0x8(%rbp) [store the stack cookit at $rbp-0x8]
	 *      ...
	 *      lea    -0xc(%rbp),%rdx [load the address of GdkNativeWindow from the stack into $rdx]
	 *
	 * This ends up with a stack layout like this:
	 * 
	 * --------------   [top of stack] %rbp
	 * |    -0x0    |   [stack cookie]
	 * |    -0x1    |          |
	 * |    -0x2    |          |
	 * |    -0x3    |          |
	 * |    -0x4    |          |
	 * |    -0x5    |          |
	 * |    -0x5    |          |
	 * |    -0x7    |   [stack cookie]
	 * |    -0x8    |   [GdkNativeWindow]
	 * |    -0x9    |          |
	 * |    -0xa    |          |
	 * |    -0xb    |          |
	 * |    -0xc    |   [GdkNativeWindow]
	 *      ....
	 * --------------   [bottom of stack]
	 *
	 * NPN_GetValue however is treating NPNVnetscapeWindow as a long, so when it writes the
	 * GdkNativeWindow to clobbers the bottom 4 bytes of the stack cookie.
	 *
	 * We're going to manually allocate a long which will put the GdkNativeWindow from 
	 * 0x08->0x10, preventing Mozilla from overwriting the stack cookie.
	 *
	 * This sucks.
	 */ 
	long window;
	NPN_GetValue (plugin->GetInstance(), NPNVnetscapeWindow, (void*)&window);
	GdkWindow *gdk = gdk_window_foreign_new ((GdkNativeWindow) window);
	return gdk;
}
