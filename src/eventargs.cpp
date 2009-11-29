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

MouseEventArgs::MouseEventArgs (MoonMouseEvent *event)
	: RoutedEventArgs (Type::MOUSEEVENTARGS)
{
	this->event = (MoonMouseEvent*)event->Clone();
}

MouseEventArgs::MouseEventArgs (Type::Kind kind, MoonMouseEvent *event)
	: RoutedEventArgs (kind)
{
	this->event = (MoonMouseEvent*)event->Clone();
}

MouseEventArgs::MouseEventArgs ()
	: RoutedEventArgs (Type::MOUSEEVENTARGS)
{
	event = NULL;
}

MouseEventArgs::~MouseEventArgs ()
{
	delete event;
}

void
MouseEventArgs::GetPosition (UIElement *relative_to, double *x, double *y)
{
       *x = *y = 0;

       if (event) {
	       Point p = event->GetPosition ();
	       *x = p.x;
	       *y = p.y;
       }

       if (relative_to) {
	       // FIXME this a nasty place to do this we should be able to
	       // reduce the problem for this kind of hit testing.
	       if (relative_to->GetSurface())
		       relative_to->GetSurface()->ProcessDirtyElements ();
	       relative_to->TransformPoint (x, y);
       }
}

StylusInfo*
MouseEventArgs::GetStylusInfo ()
{
	TabletDeviceType type = TabletDeviceTypeMouse;
	bool is_inverted = false;

	GetEvent()->GetStylusInfo (&type, &is_inverted);

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

	pressure = GetEvent()->GetPressure ();

	StylusPoint *point = new StylusPoint ();
	point->SetValue (StylusPoint::XProperty, Value(x));
	point->SetValue (StylusPoint::YProperty, Value(y));
	point->SetValue (StylusPoint::PressureFactorProperty, Value(pressure));

	points->Add (point);

	point->unref ();

	return points;
}

MouseButtonEventArgs::MouseButtonEventArgs ()
	: MouseEventArgs (Type::MOUSEBUTTONEVENTARGS, NULL)
{
}

MouseButtonEventArgs::MouseButtonEventArgs (MoonButtonEvent *event)
	: MouseEventArgs (Type::MOUSEBUTTONEVENTARGS, event)
{
}

MouseButtonEventArgs::~MouseButtonEventArgs ()
{
}

MouseWheelEventArgs::MouseWheelEventArgs ()
	: MouseEventArgs (Type::MOUSEWHEELEVENTARGS, NULL)
{
}

MouseWheelEventArgs::MouseWheelEventArgs (MoonScrollWheelEvent *event)
	: MouseEventArgs (Type::MOUSEWHEELEVENTARGS, event)
{
}

MouseWheelEventArgs::~MouseWheelEventArgs ()
{
}

int
MouseWheelEventArgs::GetWheelDelta ()
{
	MoonScrollWheelEvent *event = (MoonScrollWheelEvent*)GetEvent();
	return event ? event->GetWheelDelta () : 0;
}

KeyEventArgs::KeyEventArgs (MoonKeyEvent *event)
	: RoutedEventArgs (Type::KEYEVENTARGS)
{
	this->event = (MoonKeyEvent*)event->Clone ();
}

KeyEventArgs::KeyEventArgs ()
	: RoutedEventArgs (Type::KEYEVENTARGS)
{
	this->event = NULL;
}

KeyEventArgs::~KeyEventArgs ()
{
	delete event;
}

int
KeyEventArgs::GetKey ()
{
	return event ? event->GetSilverlightKey() : KeyUNKNOWN;
}

int
KeyEventArgs::GetPlatformKeyCode ()
{
	return event ? event->GetPlatformKeycode() : 0;
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
