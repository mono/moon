/*
 * pipeline-vda.h: Video Decode Acceleration Framework related parts of the pipeline for the media
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2011 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifdef INCLUDE_VDA

#ifndef __MOON_PIPELINE_VDA__
#define __MOON_PIPELINE_VDA__

#include <glib.h>
#include <pthread.h>

G_BEGIN_DECLS
#include <VideoDecodeAcceleration/VDADecoder.h>
G_END_DECLS

#include "pipeline.h"
 
namespace Moonlight {

void register_vda ();

/*
 * VDADecoder
 */
class VDADecoder : public IMediaDecoder {
private:
	void *decoder;
	void *callback;

protected:
	virtual ~VDADecoder () {}
	virtual void DecodeFrameAsyncInternal (MediaFrame* frame);
	virtual void OpenDecoderAsyncInternal ();
	
public:
	/* @SkipFactories */
	VDADecoder (Media* media, IMediaStream* stream);
	virtual void Dispose ();	
	virtual void Cleanup (MediaFrame* frame);
	virtual void CleanState ();
	virtual bool HasDelayedFrame () { return false; }
	virtual void InputEnded ();
};

/*
 * VDADecoderInfo
 */
class VDADecoderInfo : public DecoderInfo {
public:
	virtual bool Supports (const char* codec);
	virtual IMediaDecoder* Create (Media* media, IMediaStream* stream);
	virtual const char* GetName () { return "VDADecoder"; }
};

};
#endif // __MOON_PIPELINE_VDA__
#endif // INCLUDE_VDA
