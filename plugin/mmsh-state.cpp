/*
 * mmsh-state.h: Moonlight plugin routines for tracking results in the mmsh
 * state.  Stream information, bandwidth, etc.
 *
 * Author:
 *   Geoff Norton  (gnorton@novell.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include "mmsh-state.h"

int
MmshState::GetVideoStream () {
	int i;
	uint32_t video_stream_rate = 0;
	uint32_t video_stream_idx = 0;

	for (i = 0; i < 127; i++) {
		if (video_streams [i] > video_stream_rate && video_streams [i] < max_video_rate) {
			video_stream_idx = i; 
			video_stream_rate = video_streams [i];
		}
	}

	return video_stream_idx;
}

int
MmshState::GetAudioStream () {
	int i;
	uint32_t audio_stream_rate = 0;
	uint32_t audio_stream_idx = 0;

	for (i = 0; i < 127; i++) {
		if (audio_streams [i] > audio_stream_rate && audio_streams [i] < max_audio_rate) {
			audio_stream_idx = i; 
			audio_stream_rate = audio_streams [i];
		}
	}

	return audio_stream_idx;
}

void
MmshState::RestartRequest () {
	pd->dl->Send ();
}

void
MmshState::LogPPacket (uint32_t size) {
	p_packet_count++;
}
