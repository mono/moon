/*
 * debug.h: 
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MONOLIGHT_DEBUG_H__
#define __MONOLIGHT_DEBUG_H__

// Define STACK_DEBUG here to enable debugging with stack frames.
// Object tracking depends on this being defined in order to work.

#define STACK_DEBUG 0

#if STACK_DEBUG

#include <glib.h>
#include <unistd.h>
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STACK_FRAMES 30

char* get_stack_trace_prefix (const char* prefix);
void print_stack_trace_prefix (const char* prefix); 

G_BEGIN_DECLS

char* get_stack_trace ();
void print_stack_trace ();
void enable_vm_stack_trace (bool enable);

G_END_DECLS

#endif

#endif

