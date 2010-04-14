/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * netscape.h:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */
 
#ifndef __NETSCAPE_H__
#define __NETSCAPE_H__


#include <stdio.h>
#include "npapi.h"
#include "npfunctions.h"


#define NP_VERSION_MIN_SUPPORTED  13
#define NP_VERSION_HAS_RUNTIME    14
#define NP_VERSION_HAS_POPUP      16

#define STR_FROM_VARIANT(v) ((char *) NPVARIANT_TO_STRING (v).utf8characters)

#endif  // __NETSCAPE_H__


