/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * eventargs.cpp: specialized code for dealing with mouse/stylus/keyboard event args.
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <config.h>

#include <gdk/gdkkeysyms.h>

#include "eventargs.h"
#include "uielement.h"
#include "collection.h"
#include "stylus.h"
#include "runtime.h"
#include "timeline.h"

EventArgs::EventArgs ()
	: DependencyObject (Type::EVENTARGS)
{
}

EventArgs::EventArgs (Type::Kind kind)
	: DependencyObject (kind)
{
}

EventArgs::~EventArgs ()
{
}

RenderingEventArgs::RenderingEventArgs (TimeSpan renderingTime)
	: EventArgs (Type::RENDERINGEVENTARGS)
{

	this->renderingTime = renderingTime;
}

RenderingEventArgs::~RenderingEventArgs ()
{
}

TimeSpan
RenderingEventArgs::GetRenderingTime ()
{
	return renderingTime;
}



CollectionChangedEventArgs::CollectionChangedEventArgs ()
	: EventArgs (Type::COLLECTIONCHANGEDEVENTARGS)
{
	action = CollectionChangedActionAdd;
	old_item = NULL;
	new_item = NULL;
	index = -1;
}

CollectionChangedEventArgs::CollectionChangedEventArgs (CollectionChangedAction action, Value *new_item, Value *old_item, int index)
	: EventArgs (Type::COLLECTIONCHANGEDEVENTARGS)
{
	this->action = action;
	this->new_item = new_item;
	this->old_item = old_item;
	this->index = index;
}

CollectionChangedEventArgs::~CollectionChangedEventArgs ()
{
}

void
CollectionChangedEventArgs::SetChangedAction (CollectionChangedAction action)
{
	this->action = action;
}
	
CollectionChangedAction
CollectionChangedEventArgs::GetChangedAction ()
{
	return action;
}
	
void
CollectionChangedEventArgs::SetNewItem (Value *item)
{
	new_item = item;
}
	
Value *
CollectionChangedEventArgs::GetNewItem ()
{
	return new_item;
}
	
void
CollectionChangedEventArgs::SetOldItem (Value *item)
{
	old_item = item;
}
	
Value *
CollectionChangedEventArgs::GetOldItem ()
{
	return old_item;
}
	
void
CollectionChangedEventArgs::SetIndex (int index)
{
	this->index = index;
}
	
int
CollectionChangedEventArgs::GetIndex ()
{
	return index;
}

DownloadProgressEventArgs::DownloadProgressEventArgs (double progress)
	: EventArgs (Type::DOWNLOADPROGRESSEVENTARGS)
{
	this->progress = progress;
}

DownloadProgressEventArgs::DownloadProgressEventArgs ()
	: EventArgs (Type::DOWNLOADPROGRESSEVENTARGS)
{
	progress = 0.0;
}

DownloadProgressEventArgs::~DownloadProgressEventArgs ()
{
}

void
DownloadProgressEventArgs::SetProgress (double progress)
{
	this->progress = progress;
}
	
double
DownloadProgressEventArgs::GetProgress ()
{
	return progress;
}

RoutedEventArgs::RoutedEventArgs (DependencyObject *source)
	: EventArgs (Type::ROUTEDEVENTARGS)
{
	if (source)
		source->ref ();
	
	this->source = source;
	handled = false;
}

RoutedEventArgs::RoutedEventArgs ()
	: EventArgs (Type::ROUTEDEVENTARGS)
{
	source = NULL;
	handled = false;
}

RoutedEventArgs::RoutedEventArgs (Type::Kind kind)
	: EventArgs (kind)
{
	source = NULL;
	handled = false;
}

RoutedEventArgs::RoutedEventArgs (DependencyObject *source, Type::Kind kind)
{
	if (source)
		source->ref ();
	this->source = source;
	handled = false;
}

RoutedEventArgs::~RoutedEventArgs ()
{
	if (source)
		source->unref ();
}

void
RoutedEventArgs::SetHandled (bool handled)
{
	this->handled = handled;
}
	
bool
RoutedEventArgs::GetHandled ()
{
	return handled;
}

DependencyObject *
RoutedEventArgs::GetSource ()
{
	return source;
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

LogReadyRoutedEventArgs::LogReadyRoutedEventArgs ()
	: RoutedEventArgs (Type::LOGREADYROUTEDEVENTARGS)
{
	log = NULL;
	log_source = (LogSource) 0;
}

MouseEventArgs::MouseEventArgs (GdkEvent *event)
	: RoutedEventArgs (Type::MOUSEEVENTARGS)
{
	this->event = gdk_event_copy (event);
}

MouseEventArgs::MouseEventArgs (Type::Kind kind, GdkEvent *event)
	: RoutedEventArgs (kind)
{
	this->event = gdk_event_copy (event);
}

MouseEventArgs::MouseEventArgs ()
	: RoutedEventArgs (Type::MOUSEEVENTARGS)
{
	event = gdk_event_new (GDK_MOTION_NOTIFY);
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
			if (relative_to->GetSurface())
				relative_to->GetSurface()->ProcessDirtyElements ();


			relative_to->TransformPoint (x, y);
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

MouseButtonEventArgs::MouseButtonEventArgs (GdkEvent *event)
	: MouseEventArgs (Type::MOUSEBUTTONEVENTARGS, event)
{
}

MouseButtonEventArgs::MouseButtonEventArgs ()
	: MouseEventArgs (Type::MOUSEBUTTONEVENTARGS, NULL)
{
	event = gdk_event_new (GDK_BUTTON_PRESS);
}

int
MouseButtonEventArgs::GetButton ()
{
	switch (event->type) {
	case GDK_BUTTON_RELEASE:
	case GDK_3BUTTON_PRESS:
	case GDK_2BUTTON_PRESS:
	case GDK_BUTTON_PRESS:
		return ((GdkEventButton *) event)->button;
		break;
	default:
		return 0;
	}
}

int
MouseButtonEventArgs::GetClickCount ()
{
	switch (event->type) {
	case GDK_3BUTTON_PRESS:
		return 3;
	case GDK_2BUTTON_PRESS:
		return 2;
	case GDK_BUTTON_PRESS:
		return 1;
	default:
		return 0;
	}
}

MouseButtonEventArgs::~MouseButtonEventArgs ()
{
}

MouseWheelEventArgs::MouseWheelEventArgs (GdkEvent *event)
	: MouseEventArgs (Type::MOUSEWHEELEVENTARGS, event)
{
}

MouseWheelEventArgs::MouseWheelEventArgs ()
	: MouseEventArgs (Type::MOUSEWHEELEVENTARGS, NULL)
{
	event = gdk_event_new (GDK_SCROLL);
}


MouseWheelEventArgs::~MouseWheelEventArgs ()
{
}

#define MOON_SCROLL_WHEEL_DELTA 10

int
MouseWheelEventArgs::GetWheelDelta ()
{
	/* we only handle UP/DOWN scroll events for the time being */
	switch (((GdkEventScroll*)event)->direction) {
	case GDK_SCROLL_UP:
		return MOON_SCROLL_WHEEL_DELTA;
	case GDK_SCROLL_DOWN:
		return -MOON_SCROLL_WHEEL_DELTA;

	default:
	case GDK_SCROLL_LEFT:
	case GDK_SCROLL_RIGHT:
		return 0;
	}
}

KeyEventArgs::KeyEventArgs (GdkEventKey *event)
	: RoutedEventArgs (Type::KEYEVENTARGS)
{
	this->event = (GdkEventKey *) gdk_event_copy ((GdkEvent *)event);
}

KeyEventArgs::KeyEventArgs ()
	: RoutedEventArgs (Type::KEYEVENTARGS)
{
	event = (GdkEventKey *) gdk_event_new (GDK_KEY_PRESS);
}

KeyEventArgs::~KeyEventArgs ()
{
	gdk_event_free ((GdkEvent *) event);
}

GdkEventKey *
KeyEventArgs::GetEvent ()
{
	return event;
}

int
KeyEventArgs::GetKey ()
{
	return Keyboard::MapKeyValToKey (event->keyval);
}

int
KeyEventArgs::GetPlatformKeyCode ()
{
	return (moonlight_flags & RUNTIME_INIT_EMULATE_KEYCODES) ? Keyboard::MapGdkToVKey (event) : event->hardware_keycode;
}

GdkModifierType
KeyEventArgs::GetModifiers ()
{
	return (GdkModifierType) event->state;
}

bool
KeyEventArgs::IsModifier ()
{
#if GTK_CHECK_VERSION(2,10,0)
	if (gtk_check_version(2,10,0))
		return event->is_modifier;
	else
#endif
		switch (event->keyval) {
		case GDK_Shift_L:
		case GDK_Shift_R:
		case GDK_Control_L:
		case GDK_Control_R:
		case GDK_Meta_L:
		case GDK_Meta_R:
		case GDK_Alt_L:
		case GDK_Alt_R:
		case GDK_Super_L:
		case GDK_Super_R:
		case GDK_Hyper_L:
		case GDK_Hyper_R:
			return true;
		default:
			return false;
		}
}

guint
KeyEventArgs::GetKeyVal ()
{
	return event->keyval;
}

gunichar
KeyEventArgs::GetUnicode ()
{
	return gdk_keyval_to_unicode (event->keyval);
}


//
// ErrorEventArgs
//
ErrorEventArgs::ErrorEventArgs (Type::Kind kind, ErrorEventArgsType type, const MoonError error)
{
	Initialize (kind, type, error, 0, NULL);
}

ErrorEventArgs::ErrorEventArgs (ErrorEventArgsType type, MoonError error)
{
	Initialize (Type::ERROREVENTARGS, type, error, 0, NULL);
}

ErrorEventArgs::ErrorEventArgs (ErrorEventArgsType type, MoonError error, int extended_error_code, const char *extended_msg)
{
	Initialize (Type::ERROREVENTARGS, type, error, extended_error_code, extended_msg);
}

void
ErrorEventArgs::Initialize (Type::Kind kind, ErrorEventArgsType type, const MoonError &error, int extended_error_code, const char *extended_msg)
{
	SetObjectType (kind);
	error_type = type;
	this->error = new MoonError (error);
	extended_message = g_strdup (extended_msg);
	extended_code = extended_error_code;
}

ErrorEventArgs::~ErrorEventArgs ()
{
	delete error;
	g_free (extended_message);
}


//
// ImageErrorEventArgs
//

ImageErrorEventArgs::ImageErrorEventArgs (MoonError error)
  : ErrorEventArgs (Type::IMAGEERROREVENTARGS, ImageError, error)
{
}

ImageErrorEventArgs::~ImageErrorEventArgs ()
{
}


//
//
// ParserErrorEventArgs
//

ParserErrorEventArgs::ParserErrorEventArgs (const char *msg, const char *file,
					    int line, int column, int error_code, 
					    const char *element, const char *attribute)
  : ErrorEventArgs (Type::PARSERERROREVENTARGS, ParserError, MoonError (MoonError::XAML_PARSE_EXCEPTION, error_code, msg))
{
	xml_attribute = g_strdup (attribute);
	xml_element = g_strdup (element);
	xaml_file = g_strdup (file);
	char_position = column;
	line_number = line;
}

ParserErrorEventArgs::~ParserErrorEventArgs ()
{
	g_free (xaml_file);
	g_free (xml_element);
	g_free (xml_attribute);
}

//
// TimelineMarkerRoutedEventArgs
//

TimelineMarkerRoutedEventArgs::TimelineMarkerRoutedEventArgs (TimelineMarker *marker)
	: RoutedEventArgs (Type::TIMELINEMARKERROUTEDEVENTARGS)
{
	this->marker = marker;
	if (marker)
		marker->ref ();
}

TimelineMarkerRoutedEventArgs::~TimelineMarkerRoutedEventArgs ()
{
	if (marker)
		marker->unref ();
}
