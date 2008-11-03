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

#include "runtime.h"

#define MAX_STACK_FRAMES 30

char* get_stack_trace_prefix (const char* prefix);
void print_stack_trace_prefix (const char* prefix); 

G_BEGIN_DECLS

char* get_stack_trace ();
void print_stack_trace ();
void enable_vm_stack_trace ();
void print_gdb_trace ();
G_END_DECLS

#define LOG_ALSA(...)				if (debug_flags & RUNTIME_DEBUG_ALSA) printf (__VA_ARGS__);
#define LOG_ALSA_EX(...)			if (debug_flags & RUNTIME_DEBUG_ALSA_EX) printf (__VA_ARGS__);
#define LOG_AUDIO(...)				if (debug_flags & RUNTIME_DEBUG_AUDIO) printf (__VA_ARGS__);
#define LOG_AUDIO_EX(...)			if (debug_flags & RUNTIME_DEBUG_AUDIO_EX) printf (__VA_ARGS__);
#define LOG_PULSE(...)				if (debug_flags & RUNTIME_DEBUG_PULSE) printf (__VA_ARGS__);
#define LOG_PULSE_EX(...)			if (debug_flags & RUNTIME_DEBUG_PULSE_EX) printf (__VA_ARGS__);
#define LOG_HTTPSTREAMING(...)			if (debug_flags & RUNTIME_DEBUG_HTTPSTREAMING) printf (__VA_ARGS__);
#define LOG_MARKERS(...)			if (debug_flags & RUNTIME_DEBUG_MARKERS) printf (__VA_ARGS__);
#define LOG_MARKERS_EX(...)			if (debug_flags & RUNTIME_DEBUG_MARKERS_EX)printf (__VA_ARGS__);
#define LOG_MMS(...)				if (debug_flags & RUNTIME_DEBUG_MMS)printf (__VA_ARGS__);
#define LOG_MEDIAPLAYER(...)			if (debug_flags & RUNTIME_DEBUG_MEDIAPLAYER) printf (__VA_ARGS__);
#define LOG_MEDIAPLAYER_EX(...)			if (debug_flags & RUNTIME_DEBUG_MEDIAPLAYER_EX) printf (__VA_ARGS__);
#define LOG_PIPELINE(...)			if (debug_flags & RUNTIME_DEBUG_PIPELINE) printf (__VA_ARGS__);
#define LOG_PIPELINE_ERROR(...)			if (debug_flags & RUNTIME_DEBUG_PIPELINE_ERROR) printf (__VA_ARGS__);
#define LOG_PIPELINE_ERROR_CONDITIONAL(x, ...) if (x && debug_flags & PIPELINE_ERROR) printf (__VA_ARGS__);
#define LOG_FRAMEREADERLOOP(...)		if (debug_flags & RUNTIME_DEBUG_FRAMEREADERLOOP) printf (__VA_ARGS__);
#define LOG_FFMPEG(...)				if (debug_flags & RUNTIME_DEBUG_FFMPEG) printf(__VA_ARGS__);
#define LOG_UI(...)				if (debug_flags & RUNTIME_DEBUG_UI) printf (__VA_ARGS__);
#define LOG_CODECS(...)				if (debug_flags & RUNTIME_DEBUG_CODECS) printf (__VA_ARGS__);
#define LOG_DP(...)				if (debug_flags & RUNTIME_DEBUG_DP) printf (__VA_ARGS__);
#define LOG_DOWNLOADER(...)			if (debug_flags & RUNTIME_DEBUG_DOWNLOADER) printf (__VA_ARGS__);
#define LOG_FONT(...)				if (debug_flags & RUNTIME_DEBUG_FONT) fprintf (__VA_ARGS__);
#define LOG_LAYOUT(...)				if (debug_flags & RUNTIME_DEBUG_LAYOUT) fprintf (__VA_ARGS__);
#define LOG_MEDIA(...)				if (debug_flags & RUNTIME_DEBUG_MEDIA) printf (__VA_ARGS__);
#define LOG_MEDIAELEMENT(...)			if (debug_flags & RUNTIME_DEBUG_MEDIAELEMENT) printf (__VA_ARGS__);
#define LOG_MEDIAELEMENT_EX(...)		if (debug_flags & RUNTIME_DEBUG_MEDIAELEMENT_EX) printf (__VA_ARGS__);
#define LOG_BUFFERING(...)			if (debug_flags & RUNTIME_DEBUG_BUFFERING) printf (__VA_ARGS__);
#define LOG_PIPELINE_ASF(...)			if (debug_flags & RUNTIME_DEBUG_ASF) printf (__VA_ARGS__);
#define LOG_PLAYLIST(...)			if (debug_flags & RUNTIME_DEBUG_PLAYLIST) printf (__VA_ARGS__);
#define LOG_PLAYLIST_WARN(...)			if (debug_flags & RUNTIME_DEBUG_PLAYLIST_WARN) printf (__VA_ARGS__);
#define LOG_TEXT(...)				if (debug_flags & RUNTIME_DEBUG_TEXT) fprintf (__VA_ARGS__);
#define LOG_XAML(...)				if (debug_flags & RUNTIME_DEBUG_XAML) printf (__VA_ARGS__);

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
#define LOG_BUFFERING(...)
#define LOG_PIPELINE_ASF(...)
#define LOG_PLAYLIST(...)
#define LOG_PLAYLIST_WARN(...)
#define LOG_TEXT(...)
#define LOG_XAML(...)




#define print_stack_trace()
#define print_gdb_trace()
#define enable_vm_stack_trace()

#endif /* DEBUG */

#endif /* __MOONLIGHT_DEBUG_H */

