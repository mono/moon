/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * moonbuild.h: Compiler specific things
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2011 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOONBUILD_H__
#define __MOONBUILD_H__

#if defined _WIN32 || defined __CYGWIN__
#define MOON_DLL_EXPORT __declspec(dllexport)
#define MOON_DLL_IMPORT __declspec(dllimport)
#define MOON_DLL_LOCAL
#else
#if __GNUC__ >= 4
#define MOON_DLL_EXPORT __attribute__ ((visibility("default")))
#define MOON_DLL_IMPORT __attribute__ ((visibility("default")))
#define MOON_DLL_LOCAL  __attribute__ ((visibility("hidden")))
#else
#define MOON_DLL_EXPORT
#define MOON_DLL_IMPORT
#define MOON_DLL_LOCAL
#endif
#endif

#if BUILDING_MOONLIGHT
#define MOON_API MOON_DLL_EXPORT
#define MOON_LOCAL MOON_DLL_LOCAL
#else
#define MOON_API MOON_DLL_IMPORT
#define MOON_LOCAL
#endif

#endif // __MOONBUILD_H__
