/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 * pal-threads-pthreads.cpp
 *
 * Copyright 2011 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include "config.h"

#if PAL_THREADS_PTHREADS

#include "pal-threads.h"

namespace Moonlight {

MoonTlsKey::MoonTlsKey ()
{
	pthread_key_create (&tls_key, NULL);
}

MoonTlsKey::~MoonTlsKey ()
{
}


MoonThread::MoonThread (ThreadFunc func, gpointer func_arg)
	: func (func), func_arg (func_arg)
{
}

MoonThread::MoonThread ()
{
	pt = pthread_self ();
}

MoonThread::~MoonThread ()
{
}

bool
MoonThread::Join ()
{
	return pthread_join (pt, NULL);
}

bool
MoonThread::IsThread (MoonThread* other)
{
	return pthread_equal (pthread_self(), other->pt);
}

gpointer
MoonThread::GetSpecific (MoonTlsKey& key)
{
	return (gpointer)pthread_getspecific (key.tls_key);
}

void
MoonThread::SetSpecific (MoonTlsKey& key, gpointer data)
{
	pthread_setspecific (key.tls_key, data);
}

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


MoonMutex::MoonMutex (bool recursive)
{
	if (!recursive) {
		pthread_mutex_init (&mutex, NULL);
	}
	else {
		pthread_mutexattr_t mta;
		pthread_mutexattr_init (&mta);
		pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init (&mutex, &mta);
		pthread_mutexattr_destroy (&mta);
	}
}

MoonMutex::~MoonMutex ()
{
	pthread_mutex_destroy (&mutex);
}

void
MoonMutex::Lock ()
{
	pthread_mutex_lock (&mutex);
}

void
MoonMutex::Unlock ()
{
	pthread_mutex_unlock (&mutex);
}


MoonRWLock::MoonRWLock ()
{
#if HAVE_PTHREAD_RWLOCK_RDLOCK
	pthread_rwlock_init (&lock, NULL);
#else
	pthread_mutex_init (&lock, NULL);
#endif
}

MoonRWLock::~MoonRWLock ()
{
#if HAVE_PTHREAD_RWLOCK_RDLOCK
	pthread_rwlock_destroy (&lock);
#else
	pthread_mutex_destroy (&lock);
#endif
}

void
MoonRWLock::ReadUnlock ()
{
#if HAVE_PTHREAD_RWLOCK_RDLOCK
	pthread_rwlock_unlock (&lock);
#else
	pthread_mutex_unlock (&lock);
#endif
}

void
MoonRWLock::WriteUnlock ()
{
#if HAVE_PTHREAD_RWLOCK_RDLOCK
	pthread_rwlock_unlock (&lock);
#else
	pthread_mutex_unlock (&lock);
#endif
}

void
MoonRWLock::ReadLock ()
{
#if HAVE_PTHREAD_RWLOCK_RDLOCK
	pthread_rwlock_rdlock (&lock);
#else
	pthread_mutex_lock (&lock);
#endif
}

void
MoonRWLock::WriteLock ()
{
#if HAVE_PTHREAD_RWLOCK_RDLOCK
	pthread_rwlock_wrlock (&lock);
#else
	pthread_mutex_lock (&lock);
#endif
}

MoonCond::MoonCond ()
{
	pthread_cond_init (&cond, NULL);
}

MoonCond::~MoonCond ()
{
	pthread_cond_destroy (&cond);
}

void
MoonCond::TimedWait (MoonMutex &mutex, timespec *ts)
{
	pthread_cond_timedwait (&cond, &mutex.mutex, ts);
}

void
MoonCond::Wait (MoonMutex &mutex)
{
	pthread_cond_wait (&cond, &mutex.mutex);
}

void
MoonCond::Signal ()
{
	pthread_cond_signal (&cond);
}

void
MoonCond::Broadcast ()
{
	pthread_cond_broadcast (&cond);
}


};

#endif
