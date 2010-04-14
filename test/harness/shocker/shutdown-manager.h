/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * shutdown-manager.h
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __SHUTDOWN_MANAGER_H__
#define __SHUTDOWN_MANAGER_H__

#include <glib.h>

void shutdown_manager_wait_increment ();
void shutdown_manager_wait_decrement ();
void shutdown_manager_queue_shutdown ();

G_BEGIN_DECLS

void SignalShutdown (const char *window_name);
void TestHost_SignalShutdown (const char *window_name);

G_END_DECLS

#endif  // __SHUTDOWN_MANAGER_H__

