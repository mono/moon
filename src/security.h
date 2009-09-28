/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * security.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __SECURITY_H__
#define __SECURITY_H__

#include <string.h>
#include <glib.h>
#include <sys/stat.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/assembly.h>

G_BEGIN_DECLS

G_GNUC_INTERNAL void security_enable_coreclr (const char *platform_dir);

G_END_DECLS

#endif

