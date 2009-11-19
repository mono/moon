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

#ifndef MOON_TIMESOURCE_H
#define MOON_TIMESOURCE_H

#include <glib.h>
#include "clock.h"
#include "deployment.h"

#define MOON_PRIORITY_HIGH G_PRIORITY_DEFAULT
#define MOON_PRIORITY_DEFAULT G_PRIORITY_DEFAULT_IDLE
#define MOON_PRIORITY_IDLE G_PRIORITY_DEFAULT_IDLE

/* @Namespace=None,ManagedEvents=Manual */
class TimeSource : public EventObject {
 protected:
	virtual ~TimeSource ();

 public:
	TimeSource ();
	TimeSource (Deployment *deployment);

	virtual void Start ();
	virtual void Stop ();
	virtual void SetTimerFrequency (int timeout);
	
	virtual TimeSpan GetNow ();

	const static int TickEvent;
};

class SystemTimeSource : public TimeSource {
 protected:
	virtual ~SystemTimeSource ();

 public:
	SystemTimeSource ();
	SystemTimeSource (Deployment *deployment);

	virtual void Start ();
	virtual void Stop ();
	virtual void SetTimerFrequency (int timeout);
	
	virtual TimeSpan GetNow ();
	
 private:
	guint timeout_id;
	int frequency;
	static gboolean tick_timeout (gpointer data);
};

class ManualTimeSource : public TimeSource {
 protected:
	virtual ~ManualTimeSource ();

 public:
	ManualTimeSource ();

	virtual TimeSpan GetNow ();

	void SetCurrentTime (TimeSpan current_time);

 private:
	TimeSpan current_time;
};

G_BEGIN_DECLS

/* useful for timing things */
TimeSpan get_now (void);

G_END_DECLS

#endif /* MOON_TIMESOURCE_H */
