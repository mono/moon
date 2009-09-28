/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include <glib.h>

#include <stdlib.h>
#include <string.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <time.h>

#include "timesource.h"

#ifdef _MSC_VER
#include "Winsock2.h"

static int
gettimeofday (struct timeval *tv, void *tz)
{
	long int l = GetTickCount ();
	
	tv->tv_sec = l / 1000;
	tv->tv_usec = (l % 1000) * 1000;
	return 0;
} 
#endif // _MSC_VER


TimeSpan
get_now (void)
{
        struct timeval tv;
        TimeSpan res;
#ifdef CLOCK_MONOTONIC
	struct timespec tspec;
	if (clock_gettime (CLOCK_MONOTONIC, &tspec) == 0) {
		res = (TimeSpan)((gint64)tspec.tv_sec * 10000000 + tspec.tv_nsec / 100);
		return res;
	}
#endif

        if (gettimeofday (&tv, NULL) == 0) {
                res = (TimeSpan)(tv.tv_sec * 1000000 + tv.tv_usec) * 10;
                return res;
        }

	// XXX error
	return 0;
}


TimeSource::TimeSource (Deployment *deployment) : EventObject (deployment)
{
	SetObjectType (Type::TIMESOURCE);
}

TimeSource::TimeSource ()
{
	SetObjectType (Type::TIMESOURCE);
}

TimeSource::~TimeSource ()
{
}

void
TimeSource::Start ()
{
}

void
TimeSource::Stop ()
{
}

void
TimeSource::SetTimerFrequency (int frequency)
{
}

TimeSpan
TimeSource::GetNow ()
{
	return 0;
}

SystemTimeSource::SystemTimeSource (Deployment *deployment) : TimeSource (deployment)
{
	SetObjectType (Type::SYSTEMTIMESOURCE);
	timeout_id = 0;
	frequency = -1;
}

SystemTimeSource::SystemTimeSource ()
{
	SetObjectType (Type::SYSTEMTIMESOURCE);
	timeout_id = 0;
	frequency = -1;
}

SystemTimeSource::~SystemTimeSource ()
{
	Stop();
}

void
SystemTimeSource::SetTimerFrequency (int timeout)
{
	bool running = timeout_id != 0;
	
	if (running)
		Stop ();

	frequency = timeout;

	if (running)
		Start ();
}

void
SystemTimeSource::Start ()
{
	if (timeout_id != 0)
		return;
	
	if (frequency == -1)
		g_warning ("SystemTimeSource::frequency uninitialized in ::Start()");
	
	timeout_id = g_timeout_add_full (MOON_PRIORITY_DEFAULT, frequency, SystemTimeSource::tick_timeout, this, NULL);
}

void
SystemTimeSource::Stop ()
{
	if (timeout_id != 0) {
		g_source_remove (timeout_id);
		timeout_id = 0;
	}
}

TimeSpan
SystemTimeSource::GetNow ()
{
	return get_now ();
}

gboolean
SystemTimeSource::tick_timeout (gpointer data)
{
	SystemTimeSource *source = (SystemTimeSource *)data;

	source->SetCurrentDeployment ();
	source->Emit (TimeSource::TickEvent);
	return TRUE;
}

ManualTimeSource::ManualTimeSource ()
{
	SetObjectType (Type::MANUALTIMESOURCE);
	current_time = 0;
}

ManualTimeSource::~ManualTimeSource ()
{
}

void
ManualTimeSource::SetCurrentTime (TimeSpan current_time)
{
	this->current_time = current_time;
	g_main_context_iteration (g_main_context_default (), false);
	Emit (TimeSource::TickEvent);
	Emit (TimeSource::TickEvent);
	Emit (TimeSource::TickEvent);
}

TimeSpan
ManualTimeSource::GetNow ()
{
	return current_time;
}

