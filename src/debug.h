/*
 * debug.h: 
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOONLIGHT_DEBUG_H__
#define __MOONLIGHT_DEBUG_H__

#if DEBUG

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
void enable_vm_stack_trace ();
void print_gdb_trace ();
G_END_DECLS

#endif

#endif

