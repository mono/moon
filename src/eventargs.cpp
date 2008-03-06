/*
 * eventargs.cpp: specialized code for dealing with mouse/stylus/keyboard event args.
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include "eventargs.h"
#include "uielement.h"
#include "collection.h"
#include "stylus.h"
#include "dirty.h"
#include "media.h"

MouseEventArgs::MouseEventArgs (GdkEvent *event)
{
	this->event = gdk_event_copy (event);
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
			if (is_anything_dirty())
				process_dirty_elements();


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

	double x, y;
	double pressure;

	GetPosition (ink_presenter, &x, &y);
	if (!gdk_event_get_axis (event, GDK_AXIS_PRESSURE, &pressure))
		pressure = 0.0;

	StylusPoint *point = new StylusPoint ();
	point->SetValue (StylusPoint::XProperty, Value(x));
	point->SetValue (StylusPoint::YProperty, Value(y));
	point->SetValue (StylusPoint::PressureFactorProperty, Value(pressure));

	points->Add (point);

	point->unref ();

	return points;
}

int
mouse_event_args_get_state (MouseEventArgs *args)
{
	return args->GetState ();
}

void
mouse_event_args_get_position (MouseEventArgs *args, UIElement *relative_to, double *x, double *y)
{
	args->GetPosition (relative_to, x, y);
}

StylusInfo*
mouse_event_args_get_stylus_info (MouseEventArgs *args)
{
	return args->GetStylusInfo ();
}

StylusPointCollection*
mouse_event_args_get_stylus_points (MouseEventArgs *args, UIElement *ink_presenter)
{
	return args->GetStylusPoints (ink_presenter);
}

/*
 * MarkerReachedEventArgs
 */

MarkerReachedEventArgs::MarkerReachedEventArgs (TimelineMarker *marker)
{
	this->marker = marker;
	this->marker->ref ();
}

MarkerReachedEventArgs::~MarkerReachedEventArgs ()
{
	marker->unref ();
}

