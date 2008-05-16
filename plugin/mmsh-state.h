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

#ifndef __MMSH_STATE_H
#define __MMSH_STATE_H

class MmshState;

#include "moonlight.h"
#include "plugin-downloader.h"

class MmshState {
private:
	PluginDownloader *pd;
	int32_t audio_streams[128];
	int32_t video_streams[128];
	uint32_t p_packet_count;
	bool describing;
	char *streams;

	uint32_t max_audio_rate;
	uint32_t max_video_rate;

public:
	MmshState (PluginDownloader *pd) {
		this->pd = pd;
		this->p_packet_count = 0;
		this->describing = false;
 		this->max_audio_rate = UINT32_MAX;
 		this->max_video_rate = UINT32_MAX;
		memset (audio_streams, 0xff, 128*4);
		memset (video_streams, 0xff, 128*4);
	}

	void AddAudioStream (int index, int bitrate) { audio_streams [index] = bitrate; }
	void AddVideoStream (int index, int bitrate) { video_streams [index] = bitrate; }

	void SetDescribing (bool val) { describing = val; }
	bool IsDescribing () { return describing; }

	int GetAudioStream ();
	int GetVideoStream ();

	void RestartRequest ();
	uint32_t GetPPacketCount () { return p_packet_count; }
	void LogPPacket (uint32_t size);
};

#endif
