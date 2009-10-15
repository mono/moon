/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * codec-url.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __CODEC_URL_H__
#define __CODEC_URL_H__

#if defined (__linux__)
#  if defined (__i386__)
#    define CODEC_URL "http://192.168.1.4:8080/silverlight-media-pack-linux-x86-15-1.so"
#  endif
#  if defined (__x86_64__)
#    define CODEC_URL "http://192.168.1.4:8080/silverlight-media-pack-linux-x64-15-1.so"
#  endif
#endif

#endif
