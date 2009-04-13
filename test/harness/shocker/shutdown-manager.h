/*
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * Authors:
 *   Jackson Harper (jackson@ximian.com)
 *   Rolf Bjarne Kvinge (RKvinge@novell.com)
 *
 * Copyright 2007-2009 Novell, Inc. (http://www.novell.com)
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

G_END_DECLS

#endif  // __SHUTDOWN_MANAGER_H__

