/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 * pal-threads.cpp
 *
 * Copyright 2011 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include "config.h"

#include "pal-threads.h"

namespace Moonlight {

#if PAL_THREADS_PTHREADS

void*
MoonThread::Main (void* data)
{
	MoonThread *moon_thread = (MoonThread*)data;

	pthread_setspecific (self_tls, moon_thread);

	void* rv = moon_thread->func (moon_thread->func_arg);

	pthread_setspecific (self_tls, NULL); // this will invoke the key dtor MoonThread::Cleanup

	return rv;
}

pthread_key_t MoonThread::self_tls = 0;

int
MoonThread::Start (MoonThread **mt, MoonThread::ThreadFunc func, gpointer arg)
{
	if (self_tls == 0)
		pthread_key_create (&self_tls, NULL);

	MoonThread *moon_thread = new MoonThread (func, arg);
	int result = pthread_create (&moon_thread->pt, NULL, MoonThread::Main, moon_thread);

	if (result == 0) {
		*mt = moon_thread;
	}
	else {
		delete moon_thread;
		*mt = NULL;
	}

	return result;
}

int
MoonThread::StartJoinable (MoonThread **mt, MoonThread::ThreadFunc func, gpointer arg)
{
	if (self_tls == 0)
		pthread_key_create (&self_tls, NULL);

	MoonThread *moon_thread = new MoonThread (func, arg);

	pthread_attr_t attribs;

	pthread_attr_init (&attribs);
	pthread_attr_setdetachstate (&attribs, PTHREAD_CREATE_JOINABLE);
	int result = pthread_create (&moon_thread->pt, &attribs, MoonThread::Main, moon_thread);
	pthread_attr_destroy (&attribs);

	if (result == 0) {
		*mt = moon_thread;
	}
	else {
		delete moon_thread;
		*mt = NULL;
	}

	return result;
}

void
MoonThread::Cleanup (void *data)
{
	delete (MoonThread*)data;
}

MoonThread*
MoonThread::Self ()
{
	// FIXME we race here
	if (self_tls == 0)
		pthread_key_create (&self_tls, MoonThread::Cleanup);

	MoonThread *mt = (MoonThread*)pthread_getspecific (self_tls);
	if (mt == NULL) {
		// this happens in an unattached thread (the main thread, as well
		// as mono threads that call into moonlight)
		mt = new MoonThread ();
		pthread_setspecific (self_tls, mt);
	}

	return mt;
}

#elif PLATFORM_WINDOWS

unsigned
MoonThread::Main (void* data)
{
	MoonThread *moon_thread = (MoonThread*)data;

	TlsSetValue (self_tls, moon_thread);

	/* XXX nothing done with return value */ moon_thread->func (moon_thread->func_arg);

	TlsSetValue (self_tls, NULL);

	delete moon_thread;

	return 0;
}

int
MoonThread::Start (MoonThread **mt, ThreadFunc func, gpointer arg)
{
	MoonThread *moon_thread = new MoonThread (func, arg);
	
	int result = _beginthreadex (NULL,
				     0,
				     MoonThread::Main,
				     moon_thread,
				     0,
				     &moon_thread->thread_id);

	if (result != 0) {
		*mt = moon_thread;
	}
	else {
		delete moon_thread;
		*mt = NULL;
	}

	return result;
}

int
MoonThread::StartJoinable (MoonThread **thread, ThreadFunc func, gpointer arg)
{
	return MoonThread::Start (thread, func, arg);
}

MoonThread*
MoonThread::Self ()
{
	MoonThread *mt = (MoonThread*)TlsGetValue (self_tls);
	if (mt == NULL) {
		// this happens in an unattached thread (the main thread, as well
		// as mono threads that call into moonlight)
		mt = new MoonThread (); // FIXME are we going to leak this?  there's no tls dtors in win32...
		TlsSetValue (self_tls, mt);
	}

	return mt;
}

#endif

};
