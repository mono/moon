/*
 * text.cpp: Support for Text (Inline, Inlines, TextBlock)
 *
 * Author:
 *   Miguel de Icaza (miguel@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */
#define __STDC_CONSTANT_MACROS
#include <unistd.h>
#include <string.h>
#include <gtk/gtk.h>
#include <cairo.h>
#include <malloc.h>
#include <glib.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include "runtime.h"
#include "text.h"

