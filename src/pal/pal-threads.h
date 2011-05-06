/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef MOON_PAL_THREADS_H
#define MOON_PAL_THREADS_H

#include <glib.h>

#if PAL_THREADS_PTHREADS
#include <pthread.h>
#endif

namespace Moonlight {

class MoonMutex;
class MoonCond;
class MoonThread;
class MoonTlsKey;

#if PAL_THREADS_PTHREADS

class MoonTlsKey {
public:
	MoonTlsKey () { pthread_key_create (&tls_key, NULL); }
private:
	friend class MoonThread;
	pthread_key_t tls_key;
};

class MoonThread {
public:
	typedef gpointer (*ThreadFunc) (gpointer);

	bool Join () { return pthread_join (pt, NULL); }

	static int Start (MoonThread **thread, ThreadFunc func, gpointer arg = NULL);
	static int StartJoinable (MoonThread **thread, ThreadFunc func, gpointer arg = NULL);

	static bool IsThread (MoonThread* other) { return pthread_equal (pthread_self(), other->pt); }

	static MoonThread* Self ();

	static gpointer GetSpecific (MoonTlsKey& key) { return (gpointer)pthread_getspecific (key.tls_key); }
	static void SetSpecific (MoonTlsKey& key, gpointer data) { pthread_setspecific (key.tls_key, data); }

private:
	pthread_t pt;
	static pthread_key_t self_tls;
	ThreadFunc func;
	gpointer func_arg;

	MoonThread (ThreadFunc func, gpointer func_arg) : func (func), func_arg (func_arg) {}
	MoonThread () { pt = pthread_self (); }

	~MoonThread () { }

	static void Cleanup (void *data);
	static void* Main (void* data);
};

class MoonMutex {
public:
	MoonMutex (bool recursive = false) {
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
	~MoonMutex () { pthread_mutex_destroy (&mutex); }

	void Lock () { pthread_mutex_lock (&mutex); }
	void Unlock () { pthread_mutex_unlock (&mutex); }

private:
	friend class MoonCond;
	pthread_mutex_t mutex;
};

class MoonRWLock {
public:
	MoonRWLock () {
#if HAVE_PTHREAD_RWLOCK_RDLOCK
		pthread_rwlock_init (&lock, NULL);
#else
		pthread_mutex_init (&lock, NULL);
#endif
	}

	~MoonRWLock () {
#if HAVE_PTHREAD_RWLOCK_RDLOCK
		pthread_rwlock_destroy (&lock);
#else
		pthread_mutex_destroy (&lock);
#endif
	}

	void ReadUnlock () {
#if HAVE_PTHREAD_RWLOCK_RDLOCK
		pthread_rwlock_unlock (&lock);
#else
		pthread_mutex_unlock (&lock);
#endif
	}

	void WriteUnlock () {
#if HAVE_PTHREAD_RWLOCK_RDLOCK
		pthread_rwlock_unlock (&lock);
#else
		pthread_mutex_unlock (&lock);
#endif
	}

	void ReadLock () {
#if HAVE_PTHREAD_RWLOCK_RDLOCK
		pthread_rwlock_rdlock (&lock);
#else
		pthread_mutex_lock (&lock);
#endif
	}

	void WriteLock () {
#if HAVE_PTHREAD_RWLOCK_RDLOCK
		pthread_rwlock_wrlock (&lock);
#else
		pthread_mutex_lock (&lock);
#endif
	}

private:
#if HAVE_PTHREAD_RWLOCK_RDLOCK
	pthread_rwlock_t lock;
#else
	pthread_mutex_t lock;
#endif

};

class MoonCond {
public:

	MoonCond () { pthread_cond_init (&cond, NULL); }
	~MoonCond () { pthread_cond_destroy (&cond); }

	void TimedWait (MoonMutex &mutex, timespec *ts) { pthread_cond_timedwait (&cond, &mutex.mutex, ts); }
	void Wait (MoonMutex &mutex) { pthread_cond_wait (&cond, &mutex.mutex); }
	void Signal () { pthread_cond_signal (&cond); }
	void Broadcast () { pthread_cond_broadcast (&cond); }

private:
	pthread_cond_t cond;
};

#elif PLATFORM_WINDOWS

class MoonTlsKey {
public:
	MoonTlsKey () { tls_index = TlsAlloc (); }

	~MoonTlsKey () { TlsFree (tls_index); }
private:
	friend class MoonThread;
	DWORD tls_index;
};

class MoonThread {
public:
	typedef gpointer (*ThreadFunc) (gpointer);

	bool Join () { /* FIXME */ }

	static int Start (MoonThread **thread, ThreadFunc func, gpointer arg = NULL);
	static int StartJoinable (MoonThread **thread, ThreadFunc func, gpointer arg = NULL);

	static bool IsThread (MoonThread* other) { /* FIXME */ }

	static MoonThread* Self ();

	static gpointer GetSpecific (MoonTlsKey& key) { return TlsGetValue (key.tls_index); }
	static void SetSpecific (MoonTlsKey& key, gpointer data) { TlsSetValue (key.tls_index, data); }

private:
	MoonThread (ThreadFunc func, gpointer func_arg) : func (func), func_arg (func_arg) {}
	MoonThread () { /* FIXME */ }

	~MoonThread () { }

	static void Cleanup (void *data);
	static unsigned int Main (void* data);
};

class MoonMutex {
public:
	MoonMutex (bool recursive = false) { InitializeCriticalSection (&mutex); }
	~MoonMutex () { DeleteCriticalSection (&mutex); }

	void Lock () { EnterCriticalSection (&mutex); }
	void Unlock () { LeaveCriticalSection (&mutex); }

private:
	friend class MoonCond;
	CRITICAL_SECTION mutex;
};

class MoonRWLock {
public:
	MoonRWLock () { InitializeSRWLock (&lock); }

	~MoonRWLock () { DestroySRWLock (&lock); }

	void ReadUnlock () { ReleaseSWRLockShared (&lock); }
	void ReadLock () { AcquireSWRLockShared (&lock); }

	void WriteUnlock () { ReleaseSWRLockExclusive (&lock); }
	void WriteLock () { AcquireSWRLockExclusive (&lock); }

private:
	SRWLOCK lock;
};

class MoonCond {
public:
	MoonCond () { InitializeConditionVariable (&cond); }
	~MoonCond () { DestroyConditionVariable (&cond); }

	void TimedWait (MoonMutex &mutex, timespec *ts) { SleepConditionVariableCS (&cond, &mutex.mutex, ts.tv_sec * 1000 + ts.tv_nsec / 100000); }
	void Wait (MoonMutex &mutex) { SleepConditionVariableCS (&cond, &mutex.mutex, INFINITE); }

	void Signal () { WakeConditionVariable (&cond); }
	void Broadcast () { WakeAllConditionVariable (&cond); }

private:
	CONDITION_VARIABLE cond;
};

#endif

};

#endif
