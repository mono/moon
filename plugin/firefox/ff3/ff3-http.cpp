/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * ff3-http.cpp: Firefox 3.x bridge
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <config.h>
// define this here so that protypes.h isn't included (and doesn't
// muck with our npapi.h)
#define NO_NSPR_10_SUPPORT
#ifndef NS_WARN_UNUSED_RESULT
#define NS_WARN_UNUSED_RESULT
#endif
#ifndef NS_ATTR_MALLOC
#define NS_ATTR_MALLOC
#endif
#include "plugin.h"

#include "ff3-bridge.h"

#define CONCAT(x,y) x##y
#define GECKO_SYM(x) CONCAT(FF3,x)
#include "../browser-http.inc"
