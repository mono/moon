/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * mutex.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */
 
#ifndef __MOON_MUTEX_H__
#define __MOON_MUTEX_H__
 
#include <pthread.h>
 
class Mutex {
private:
	pthread_mutex_t mutex;

public:
	Mutex ()
	{
		pthread_mutex_init (&mutex, NULL);
	}
	Mutex (bool recursive)
	{
		pthread_mutexattr_t attribs;
		pthread_mutexattr_init (&attribs);
		pthread_mutexattr_settype (&attribs, recursive ? PTHREAD_MUTEX_RECURSIVE : PTHREAD_MUTEX_DEFAULT);
		pthread_mutex_init (&mutex, &attribs);
		pthread_mutexattr_destroy (&attribs);
	}
	~Mutex ()
	{
		pthread_mutex_destroy (&mutex);
	}
	void Lock ()
	{
		pthread_mutex_lock (&mutex);
	}
	void Unlock ()
	{
		pthread_mutex_unlock (&mutex);
	}
};

#endif /* __MOON_MUTEX_H */
