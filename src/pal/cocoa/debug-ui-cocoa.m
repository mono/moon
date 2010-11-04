/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * debug-ui.cpp: debugging/inspection support for cocoa+
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include <config.h>

#include <cocoa/cocoa.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "animation.h"
#include "namescope.h"

#include "debug-ui-cocoa.h"
#include "utils.h"
#include "uri.h"
#include "grid.h"
#include "playlist.h"
#include "mediaelement.h"
#include "mediaplayer.h"
#include "pipeline-asf.h"

using namespace Moonlight;

#ifdef DEBUG

void
show_debug (MoonWindowCocoa* window)
{
	g_assert_not_reached ();
}

void
show_sources (MoonWindowCocoa *window)
{	
	g_assert_not_reached ();
}

void
debug_info (MoonWindowCocoa *window)
{
	g_assert_not_reached ();
}

#if OBJECT_TRACKING
void
debug_media (MoonWindowCocoa *window)
{
	g_assert_not_reached ();
}

void
dump_media_elements ()
{
	g_assert_not_reached ();
}
#endif /* OBJECT_TRACKING */
#endif
