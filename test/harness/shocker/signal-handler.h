/*
 * signal-handler.h
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <glib.h>

G_BEGIN_DECLS

void shocker_install_signal_handlers ();
void shocker_handle_native_sigsegv (int signal);
G_END_DECLS