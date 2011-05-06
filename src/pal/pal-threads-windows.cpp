/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 * pal-threads-windows.cpp
 *
 * Copyright 2011 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include "config.h"

#if PAL_THREADS_WINDOWS

#include "pal-threads.h"

namespace Moonlight {


MoonTlsKey::MoonTlsKey ()
{
	tls_index = TlsAlloc ();
}

MoonTlsKey::~MoonTlsKey ()
{
	TlsFree (tls_index);
}

bool
MoonThread::Join ()
{
	/* FIXME */
	return false;
}

bool
MoonThread::IsThread (MoonThread* other)
{
	/* FIXME */
}

gpointer
MoonThread::GetSpecific (MoonTlsKey& key)
{
	return TlsGetValue (key.tls_index);
}

void
MoonThread::SetSpecific (MoonTlsKey& key, gpointer data)
{
	TlsSetValue (key.tls_index, data);
}

MoonThread::MoonThread (ThreadFunc func, gpointer func_arg)
	: func (func), func_arg (func_arg)
{
}

MoonThread::MoonThread ()
{
	/* FIXME */
}

MoonThread::~MoonThread ()
{
}

MoonMutex::MoonMutex (bool recursive)
{
	InitializeCriticalSection (&mutex);
}

MoonMutex::~MoonMutex ()
{
	DeleteCriticalSection (&mutex);
}

void
MoonMutex::Lock ()
{
	EnterCriticalSection (&mutex);
}

void
MoonMutex::Unlock ()
{
	LeaveCriticalSection (&mutex);
}

MoonRWLock::MoonRWLock ()
{
	InitializeSRWLock (&lock);
}

MoonRWLock::~MoonRWLock ()
{
	DestroySRWLock (&lock);
}

void
MoonRWLock::ReadUnlock ()
{
	ReleaseSWRLockShared (&lock);
}
void
MoonRWLock::ReadLock ()
{
	AcquireSWRLockShared (&lock);
}

void
MoonRWLock::WriteUnlock ()
{
	ReleaseSWRLockExclusive (&lock);
}
void
MoonRWLock::WriteLock ()
{
	AcquireSWRLockExclusive (&lock);
}

MoonCond::MoonCond ()
{
	InitializeConditionVariable (&cond);
}
MoonCond::~MoonCond ()
{
	DestroyConditionVariable (&cond);
}

void
MoonCond::TimedWait (MoonMutex &mutex, timespec *ts)
{
	SleepConditionVariableCS (&cond, &mutex.mutex, ts.tv_sec * 1000 + ts.tv_nsec / 100000);
}
void
MoonCond::Wait (MoonMutex &mutex)
{
	SleepConditionVariableCS (&cond, &mutex.mutex, INFINITE);
}

void
MoonCond::Signal ()
{
	WakeConditionVariable (&cond);
}

void
MoonCond::Broadcast ()
{
	WakeAllConditionVariable (&cond);
}

};

#endif
