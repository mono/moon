/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * install-dialog.cpp: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "install-dialog-cocoa.h"
#include "application.h"
#include "network.h"
#include "runtime.h"
#include "utils.h"
#include "uri.h"
#include "debug.h"

namespace Moonlight {

void *
install_dialog_new (void *parent, Deployment *deployment, const char *install_dir, bool unattended)
{
	g_assert_not_reached ();
}

bool
install_dialog_install (void *dialog)
{
	g_assert_not_reached ();
}

};
