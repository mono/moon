/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
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

#if defined (DEBUG) || LOGGING

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#endif /* DEBUG || LOGGING */

#include "moonbuild.h"
#include "runtime.h"

/*
 * Stacktrace (debug) stuff
 */


#if DEBUG

#define MAX_STACK_FRAMES 10

char* get_stack_trace_prefix (const char* prefix, int maxframes = MAX_STACK_FRAMES);
void print_stack_trace_prefix (const char* prefix, int maxframes = MAX_STACK_FRAMES); 

void print_reftrace (const char * type, const char * typname, int refcount, bool keep);
void dump_frames (void);

G_BEGIN_DECLS

char* get_stack_trace (void);
void MOON_API print_stack_trace (void);
void enable_vm_stack_trace (void);
void hexdump_addr (void *addr, size_t n);

G_END_DECLS

#else

#define print_stack_trace()
#define enable_vm_stack_trace()
#define print_reftrace (const char * type, const char * typname, int refcount, bool keep)
#define dump_frames (void)

#endif /* DEBUG */

/*
 * Logging stuff
 */

#if LOGGING

#define LOG_ALSA(...)				if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_ALSA)) printf (__VA_ARGS__);
#define LOG_ALSA_EX(...)			if (G_UNLIKELY (debug_flags_ex & RUNTIME_DEBUG_ALSA_EX)) printf (__VA_ARGS__);
#define LOG_AUDIO(...)				if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_AUDIO)) printf (__VA_ARGS__);
#define LOG_AUDIO_EX(...)			if (G_UNLIKELY (debug_flags_ex & RUNTIME_DEBUG_AUDIO_EX)) printf (__VA_ARGS__);
#define LOG_PULSE(...)				if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_PULSE)) printf (__VA_ARGS__);
#define LOG_PULSE_EX(...)			if (G_UNLIKELY (debug_flags_ex & RUNTIME_DEBUG_PULSE_EX)) printf (__VA_ARGS__);
#define LOG_HTTPSTREAMING(...)			if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_HTTPSTREAMING)) printf (__VA_ARGS__);
#define LOG_MARKERS(...)			if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_MARKERS)) printf (__VA_ARGS__);
#define LOG_MARKERS_EX(...)			if (G_UNLIKELY (debug_flags_ex & RUNTIME_DEBUG_MARKERS_EX)) printf (__VA_ARGS__);
#define LOG_MMS(...)				if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_MMS)) printf (__VA_ARGS__);
#define LOG_MEDIAPLAYER(...)			if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_MEDIAPLAYER)) printf (__VA_ARGS__);
#define LOG_MEDIAPLAYER_EX(...)			if (G_UNLIKELY (debug_flags_ex & RUNTIME_DEBUG_MEDIAPLAYER_EX)) printf (__VA_ARGS__);
#define LOG_PIPELINE(...)			if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_PIPELINE)) printf (__VA_ARGS__);
#define LOG_PIPELINE_EX(...)		if (G_UNLIKELY (debug_flags_ex & RUNTIME_DEBUG_PIPELINE_EX)) printf (__VA_ARGS__);
#define LOG_PIPELINE_ERROR(...)			if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_PIPELINE_ERROR)) printf (__VA_ARGS__);
#define LOG_PIPELINE_ERROR_CONDITIONAL(x, ...) if (G_UNLIKELY (x && debug_flags & PIPELINE_ERROR)) printf (__VA_ARGS__);
#define LOG_FRAMEREADERLOOP(...)		if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_FRAMEREADERLOOP)) printf (__VA_ARGS__);
#define LOG_FFMPEG(...)				if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_FFMPEG)) printf(__VA_ARGS__);
#define LOG_UI(...)				if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_UI)) printf (__VA_ARGS__);
#define LOG_CODECS(...)				if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_CODECS)) printf (__VA_ARGS__);
#define LOG_DP(...)				if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_DP)) printf (__VA_ARGS__);
#define LOG_DOWNLOADER(...)			if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_DOWNLOADER)) printf (__VA_ARGS__);
#define LOG_FONT(...)				if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_FONT)) fprintf (__VA_ARGS__);
#define LOG_LAYOUT(...)				if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_LAYOUT)) printf (__VA_ARGS__);
#define LOG_MEDIA(...)				if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_MEDIA)) printf (__VA_ARGS__);
#define LOG_MEDIAELEMENT(...)			if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_MEDIAELEMENT)) printf (__VA_ARGS__);
#define LOG_MEDIAELEMENT_EX(...)		if (G_UNLIKELY (debug_flags_ex & RUNTIME_DEBUG_MEDIAELEMENT_EX)) printf (__VA_ARGS__);
#define LOG_MSI(...)				if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_MSI)) printf (__VA_ARGS__);
#define LOG_BUFFERING(...)			if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_BUFFERING)) printf (__VA_ARGS__);
#define LOG_PIPELINE_ASF(...)			if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_ASF)) printf (__VA_ARGS__);
#define LOG_PLAYLIST(...)			if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_PLAYLIST)) printf (__VA_ARGS__);
#define LOG_PLAYLIST_WARN(...)			if (G_UNLIKELY (debug_flags_ex & RUNTIME_DEBUG_PLAYLIST_EX)) printf (__VA_ARGS__);
#define LOG_TEXT(...)				if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_TEXT)) fprintf (__VA_ARGS__);
#define LOG_XAML(...)				if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_XAML)) printf (__VA_ARGS__);
#define LOG_DEPLOYMENT(...)		if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_DEPLOYMENT)) printf (__VA_ARGS__);
#define LOG_MP3(...)				if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_MP3)) printf (__VA_ARGS__);
#define LOG_ASF(...)			if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_ASF)) printf (__VA_ARGS__);
#define LOG_VALUE(...)			if (G_UNLIKELY (debug_flags & RUNTIME_DEBUG_VALUE)) printf (__VA_ARGS__);

#else

#define LOG_ALSA(...)
#define LOG_ALSA_EX(...)
#define LOG_AUDIO(...)
#define LOG_AUDIO_EX(...)
#define LOG_PULSE(...)
#define LOG_PULSE_EX(...)
#define LOG_HTTPSTREAMING(...) 
#define LOG_MARKERS(...)
#define LOG_MARKERS_EX(...)
#define LOG_MMS(...)
#define LOG_MEDIAPLAYER(...)
#define LOG_MEDIAPLAYER_EX(...)
#define LOG_PIPELINE(...)
#define LOG_PIPELINE_EX(...)
#define LOG_PIPELINE_ERROR(...)
#define LOG_PIPELINE_ERROR_CONDITIONAL(x, ...)
#define LOG_FRAMEREADERLOOP(...)
#define LOG_FFMPEG(...)
#define LOG_UI(...)
#define LOG_CODECS(...)
#define LOG_DP(...)
#define LOG_DOWNLOADER(...)
#define LOG_FONT(...)
#define LOG_LAYOUT(...)
#define LOG_MEDIA(...)
#define LOG_MEDIAELEMENT(...)
#define LOG_MEDIAELEMENT_EX(...)
#define LOG_MSI(...)
#define LOG_BUFFERING(...)
#define LOG_PIPELINE_ASF(...)
#define LOG_PLAYLIST(...)
#define LOG_PLAYLIST_WARN(...)
#define LOG_TEXT(...)
#define LOG_XAML(...)
#define LOG_DEPLOYMENT(...)
#define LOG_MP3(...)
#define LOG_ASF(...)
#define LOG_VALUE(...)

#endif /* LOGGING */

#if SANITY && defined (DEBUG)
G_BEGIN_DECLS
void moonlight_install_signal_handlers ();
G_END_DECLS
#else
#define moonlight_install_signal_handlers() 
#endif /* SANITY */

#endif /* __MOONLIGHT_DEBUG_H */

