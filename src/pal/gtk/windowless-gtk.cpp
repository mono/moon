/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * windowless-gtk.cpp: Windowsless Surface subclass for Gtk
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <config.h>

#include <glib.h>

#include "windowless-gtk.h"

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

MoonWindowlessGtk::MoonWindowlessGtk (int w, int h, PluginInstance *plugin)
  : MoonWindowGtk (MoonWindowType_Plugin, w, h)
{
	this->plugin = plugin;

	backing_store_width = w;
	backing_store_height = h;

	UpdateWindowInfo ();
}

MoonWindowlessGtk::~MoonWindowlessGtk ()
{
}

void
MoonWindowlessGtk::ConnectToContainerPlatformWindow (gpointer container_window)
{
	// do nothing here
}

void
MoonWindowlessGtk::UpdateWindowInfo ()
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
MoonWindowlessGtk::Resize (int width, int height)
{
	bool emit_resize = false;

	UpdateWindowInfo ();

        if (this->width != width || this->height != height) {
		this->width = width;
		this->height = height;

		g_free (backing_image_data);
		backing_image_data = NULL;

		backing_store_width = GetWidth();
		backing_store_height = GetHeight();

		emit_resize = true;
	}

	surface->HandleUIWindowAllocation (emit_resize);
}

void
MoonWindowlessGtk::SetCursor (CursorType cursor)
{
#if (NP_VERSION_MINOR >= NPVERS_HAS_WINDOWLESS_CURSORS)
	NPCursor npcursor;
	switch (cursor) {
	case CursorTypeDefault:
		npcursor = NPCursorAuto;
		break;
	case CursorTypeArrow:
		npcursor = NPCursorPointer;
		break;
	case CursorTypeWait:
		npcursor = NPCursorWait;
		break;
	case CursorTypeIBeam:
		npcursor = NPCursorText;
		break;
	case CursorTypeStylus:
		npcursor = NPCursorPointer; // XXX ugh...
		break;
	case CursorTypeEraser:
		npcursor = NPCursorPointer; // XXX ugh...
		break;
	case CursorTypeNone:
		// Silverlight display no cursor if the enumeration value is invalid (e.g. -1)
	default:
		npcursor = NPCursorNone;
		break;
	}

	MOON_NPN_SetValue (plugin->GetInstance(), NPPVcursor, (void*)npcursor);
#endif
}

void
MoonWindowlessGtk::Invalidate (Rect r)
{
	NPRect nprect;

	// Mozilla gets seriously confused about invalidations 
	// outside the windowless bounds.
	r = r.Intersection (Rect (0, 0, GetWidth(), GetHeight())).RoundOut ();

	nprect.left = (uint16_t)r.x;
	nprect.top = (uint16_t)r.y;
	nprect.right = (uint16_t)(r.x + r.width);
	nprect.bottom = (uint16_t)(r.y + r.height);

	MOON_NPN_InvalidateRect (plugin->GetInstance(), &nprect);
}

void
MoonWindowlessGtk::ProcessUpdates ()
{
	//MOON_NPN_ForceRedraw (plugin->GetInstance());
}

gboolean
MoonWindowlessGtk::HandleEvent (gpointer platformEvent)
{
	MoonEventStatus handled = MoonEventNotHandled;
	XEvent *xev = (XEvent*)platformEvent;
	
	SetCurrentDeployment ();

	if (!surface)
		return false;

	switch (xev->type) {
	case GraphicsExpose: {
		GdkNativeWindow x11_drawable = (GdkNativeWindow)xev->xgraphicsexpose.drawable;
		GdkDrawable *drawable;

		drawable = gdk_pixmap_lookup (x11_drawable);
		if (drawable) {
			g_object_ref (drawable);
		}
		else {
			drawable = gdk_pixmap_foreign_new (x11_drawable);
			if (!drawable)
				drawable = gdk_window_foreign_new (x11_drawable);
		}

		if (drawable) {
			GdkVisual *visual = gdkx_visual_get (visualid);

			if (visual) {
				GdkEventExpose expose;

				expose.type = GDK_EXPOSE;
				expose.window = NULL;
				expose.send_event = FALSE;
			       
				expose.area.x = xev->xgraphicsexpose.x;
				expose.area.y = xev->xgraphicsexpose.y;
				expose.area.width = xev->xgraphicsexpose.width;
				expose.area.height = xev->xgraphicsexpose.height;
				/* XXX ugh */
				expose.area.x -= x;
				expose.area.y -= y;
				expose.region = gdk_region_rectangle (&expose.area);

				PaintToDrawable (drawable, visual, &expose, x, y, GetTransparent(), false);

				handled = MoonEventHandled;

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

		MoonMotionEvent *mevent = (MoonMotionEvent*)Runtime::GetWindowingSystem ()->CreateEventFromPlatformEvent (&motion);
		handled = surface->HandleUIMotion (mevent);
		delete mevent;
		break;
	}
	case ButtonPress:
	case ButtonRelease: {
		if (xev->xbutton.button >= 4 && xev->xbutton.button <= 7) {
			GdkEventScroll scroll;
			
			if (xev->type == ButtonRelease)
				break;
			
			scroll.type = GDK_SCROLL;
			
			switch (xev->xbutton.button) {
			case 4: scroll.direction = GDK_SCROLL_UP; break;
			case 5: scroll.direction = GDK_SCROLL_DOWN; break;
			case 6: scroll.direction = GDK_SCROLL_LEFT; break;
			case 7: scroll.direction = GDK_SCROLL_RIGHT; break;
			}
			
			scroll.window = NULL;
			scroll.send_event = xev->xbutton.send_event;
			scroll.time = xev->xbutton.time;
			scroll.x = xev->xbutton.x;
			scroll.y = xev->xbutton.y;
			scroll.x_root = xev->xbutton.x_root;
			scroll.y_root = xev->xbutton.y_root;
			scroll.state = xev->xbutton.state;
			scroll.device = NULL; // XXX
			
			MoonScrollWheelEvent *mevent = (MoonScrollWheelEvent *) Runtime::GetWindowingSystem ()->CreateEventFromPlatformEvent (&scroll);
			handled = surface->HandleUIScroll (mevent);
			delete mevent;
			break;
		} else {
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
			button.device = NULL;
			
			if (xev->type == ButtonPress && MoonWindowGtk::container_button_press_callback (NULL, &button, this))
				handled = MoonEventHandled;
			if (handled == MoonEventNotHandled) {
				MoonButtonEvent *mevent = (MoonButtonEvent*)Runtime::GetWindowingSystem ()->CreateEventFromPlatformEvent (&button);
				if (xev->type == ButtonPress)
					handled = surface->HandleUIButtonPress (mevent);
				else
					handled = surface->HandleUIButtonRelease (mevent);
				
				delete mevent;
			}
		}
		break;
	}
	case KeyPress:
	case KeyRelease: {
		// make sure everything is initialized correctly (structure members vary with gdk version)
		GdkEventKey *key = (GdkEventKey*) gdk_event_new (xev->type == KeyPress ? GDK_KEY_PRESS : GDK_KEY_RELEASE);
		// gtk_im_context_xim_filter_keypress will dereference the NULL window leading to a SEGSIGV
		key->window = GDK_WINDOW (GetPlatformWindow ());
		key->send_event = xev->xkey.send_event;
		key->time = xev->xkey.time;
		key->state = xev->xkey.state;
		key->hardware_keycode = xev->xkey.keycode;

		gint effective_group;

		gdk_keymap_translate_keyboard_state (gdk_keymap_get_default (),
						     xev->xkey.keycode,
						     (GdkModifierType)xev->xkey.state, // XXX
						     0, // XXX
						     &key->keyval,
						     &effective_group,
						     NULL,
						     NULL);

		key->group = (guint8)effective_group;

		MoonKeyEvent *mevent = (MoonKeyEvent*)Runtime::GetWindowingSystem ()->CreateEventFromPlatformEvent (key);

		if (xev->type == KeyPress)
			handled = surface->HandleUIKeyPress (mevent);
		else
			handled = surface->HandleUIKeyRelease (mevent);

		delete mevent;

		gdk_event_free ((GdkEvent*) key);
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

		MoonCrossingEvent *mevent = (MoonCrossingEvent*)Runtime::GetWindowingSystem ()->CreateEventFromPlatformEvent (&crossing);
		surface->HandleUICrossing (mevent);
		delete mevent;
		break;
	}
	case FocusIn:
	case FocusOut: {
		GdkEventFocus focus;

		focus.type = GDK_FOCUS_CHANGE;
		focus.window = NULL;
		focus.send_event = xev->xfocus.send_event;
		focus.in = xev->type == FocusIn;
	  
		MoonFocusEvent *mevent = (MoonFocusEvent*)Runtime::GetWindowingSystem ()->CreateEventFromPlatformEvent (&focus);

		if (focus.in)
			surface->HandleUIFocusIn (mevent);
		else
			surface->HandleUIFocusOut (mevent);

		break;
	}
	default:
		d(printf ("Unhandled Xlib event %d\n", xev->type));
		break;
	}

	return handled == MoonEventHandled;
}

void
MoonWindowlessGtk::Show ()
{
	// nothing needed here
}

void
MoonWindowlessGtk::Hide ()
{
	// nothing needed here
}

void
MoonWindowlessGtk::EnableEvents (bool first)
{
	// nothing needed here, NPAPI pushes events through
	// HandleEvent.
}

void
MoonWindowlessGtk::DisableEvents ()
{
	// nothing needed here, NPAPI pushes events through
	// HandleEvent.
}

void
MoonWindowlessGtk::GrabFocus ()
{
	// we can't grab focus - the browser handles that.
}

bool
MoonWindowlessGtk::HasFocus ()
{
	// XXX maybe we should track the focus in/out events?
	return false;
}

void
MoonWindowlessGtk::SetSurface (Surface *s)
{
	MoonWindowGtk::SetSurface (s);
	s->HandleUIWindowAvailable ();
}

gpointer
MoonWindowlessGtk::GetPlatformWindow ()
{
	GdkNativeWindow window;
	MOON_NPN_GetValue (plugin->GetInstance(), NPNVnetscapeWindow, (void*)&window);
	GdkWindow *gdk = gdk_window_foreign_new (window);
	return gdk;
}
