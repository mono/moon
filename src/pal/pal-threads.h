/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef MOON_PAL_THREADS_H
#define MOON_PAL_THREADS_H

#include <glib.h>

#if PAL_THREADS_PTHREADS
#include <pthread.h>
#elif PAL_THREADS_WINDOWS
#error "windows support not finished"
#else
#error "config.h not included before #including pal-threads.h"
#endif

namespace Moonlight {

class MoonMutex;
class MoonCond;
class MoonThread;
class MoonTlsKey;

class MoonTlsKey {
public:
	MoonTlsKey ();
	~MoonTlsKey ();
private:
	friend class MoonThread;
#if PAL_THREADS_PTHREADS
	pthread_key_t tls_key;
#elif PAL_THREADS_WINDOWS
	DWORD tls_index;
#endif
};

class MoonThread {
public:
	typedef gpointer (*ThreadFunc) (gpointer);

	bool Join ();

	static int Start (MoonThread **thread, ThreadFunc func, gpointer arg = NULL);
	static int StartJoinable (MoonThread **thread, ThreadFunc func, gpointer arg = NULL);

	static bool IsThread (MoonThread* other);

	static MoonThread* Self ();

	static gpointer GetSpecific (MoonTlsKey& key);
	static void SetSpecific (MoonTlsKey& key, gpointer data);

private:
	// we want to outlaw copy ctors and operator= since the platform types might
	// not be copyable/refcounted/etc, and the dtor called on the other value will
	// destroy the platform type.
	MoonThread (const MoonThread&) { }
	// same with operator=
	MoonThread& operator=(const MoonThread& rhs) { return *this; }

	ThreadFunc func;
	gpointer func_arg;

	MoonThread (ThreadFunc func, gpointer func_arg);
	MoonThread ();

	~MoonThread ();

#if PAL_THREADS_PTHREADS
	pthread_t pt;
	static pthread_key_t self_tls;
	static void* Main (void* data);
	static void Cleanup (void *data);
#elif PAL_THREADS_WINDOWS
	static unsigned int Main (void* data);
#endif
};

class MoonMutex {
public:
	MoonMutex (bool recursive = false);
	~MoonMutex ();

	void Lock ();
	void Unlock ();

private:
	// we want to outlaw copy ctors and operator= since the platform types might
	// not be copyable/refcounted/etc, and the dtor called on the other value will
	// destroy the platform type.
	MoonMutex (const MoonMutex&) { }
	// same with operator=
	MoonMutex& operator=(const MoonMutex& rhs) { return *this; }

	friend class MoonCond;
#if PAL_THREADS_PTHREADS
	pthread_mutex_t mutex;
#elif PAL_THREADS_WINDOWS
	CRITICAL_SECTION mutex;
#endif
};

class MoonRWLock {
public:
	MoonRWLock ();
	~MoonRWLock ();

	void ReadLock ();
	void ReadUnlock ();

	void WriteUnlock ();
	void WriteLock ();

private:
	// we want to outlaw copy ctors and operator= since the platform types might
	// not be copyable/refcounted/etc, and the dtor called on the other value will
	// destroy the platform type.
	MoonRWLock (const MoonRWLock&) { }
	// same with operator=
	MoonRWLock& operator=(const MoonRWLock& rhs) { return *this; }

#if PAL_THREADS_PTHREADS
#if HAVE_PTHREAD_RWLOCK_RDLOCK
	pthread_rwlock_t lock;
#else
	pthread_mutex_t lock;
#endif
#elif PAL_THREADS_WINDOWS
	SRWLOCK lock;
#endif
};

class MoonCond {
public:

	MoonCond ();
	~MoonCond ();

	void TimedWait (MoonMutex &mutex, timespec *ts);
	void Wait (MoonMutex &mutex);
	void Signal ();
	void Broadcast ();

private:
	// we want to outlaw copy ctors and operator= since the platform types might
	// not be copyable/refcounted/etc, and the dtor called on the other value will
	// destroy the platform type.
	MoonCond (const MoonCond&) { }
	// same with operator=
	MoonCond& operator=(const MoonCond& rhs) { return *this; }

#if PAL_THREADS_PTHREADS
	pthread_cond_t cond;
#elif PAL_THREADS_WINDOWS
	CONDITION_VARIABLE cond;
#endif
};


};

#endif
