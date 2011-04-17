/*
 * pipeline-vda.cpp: Video Decode Acceleration Framework related parts of the pipeline for the media
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

/*
 *	FFmpegDecoder
 */

#include <config.h>

#include <stdlib.h>
#include <glib.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#include "pipeline-vda.h"
#include "pipeline.h"
#include "mp3.h"
#include "clock.h"
#include "debug.h"
#include "deployment.h"

namespace Moonlight {

void
register_vda ()
{
	Media::RegisterDecoder (new VDADecoderInfo ());
}

/*
 * VDADecoder
 */

VDADecoder::VDADecoder (Media* media, IMediaStream* stream) 
	: IMediaDecoder (Type::VDADECODER, media, stream)
{
}

void
VDADecoder::InputEnded ()
{
	GetStream ()->SetOutputEnded (true);
}

static void
VDADecoderCallback (void *decompressionOutputRefCon, CFDictionaryRef frameInfo, OSStatus status, uint32_t infoFlags, CVImageBufferRef imageBuffer)
{
	VDADecoder *decoder = (VDADecoder *) decompressionOutputRefCon;
		
	if (imageBuffer == NULL) {
		return;
	}

	OSType format_type = CVPixelBufferGetPixelFormatType(imageBuffer);
	if (format_type != kCVPixelFormatType_422YpCbCr8) {
		g_warning ("Mismatched format in VDA");
		return;
	}
		
	g_warning ("Decoder: %p\n", decoder);
}

void
VDADecoder::OpenDecoderAsyncInternal ()
{
	IMediaStream *stream = GetStream ();

	int w = ((VideoStream *) stream)->GetWidth ();
	int h = ((VideoStream *) stream)->GetHeight ();
	int format = 'avc1';

	CFMutableDictionaryRef config = CFDictionaryCreateMutable (kCFAllocatorDefault, 4, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

	CFDataRef cfData = CFDataCreate (kCFAllocatorDefault, (const uint8_t*) stream->GetExtraData (), stream->GetExtraDataSize ());
	CFNumberRef cfWidth  = CFNumberCreate (kCFAllocatorDefault, kCFNumberSInt32Type, &w);
	CFNumberRef cfHeight = CFNumberCreate (kCFAllocatorDefault, kCFNumberSInt32Type, &h);
	CFNumberRef cfFormat = CFNumberCreate (kCFAllocatorDefault, kCFNumberSInt32Type, &format);

	CFDictionarySetValue (config, kVDADecoderConfiguration_Height, cfHeight);
	CFDictionarySetValue (config, kVDADecoderConfiguration_Width,  cfWidth);
	CFDictionarySetValue (config, kVDADecoderConfiguration_SourceFormat, cfFormat);
	CFDictionarySetValue (config, kVDADecoderConfiguration_avcCData, cfData);

	CFRelease (cfWidth);
	CFRelease (cfHeight);
	CFRelease (cfFormat);
	CFRelease (cfData);
									
	CFMutableDictionaryRef attr = CFDictionaryCreateMutable (kCFAllocatorDefault, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
									
	OSType cvPixelFormatType = kCVPixelFormatType_422YpCbCr8;
	CFNumberRef pixelFormat  = CFNumberCreate (kCFAllocatorDefault, kCFNumberSInt32Type, &cvPixelFormatType);
	CFDictionarySetValue (attr, kCVPixelBufferPixelFormatTypeKey, pixelFormat);
									
	callback = (void *) VDADecoderCallback;
	g_warning ("callback: %p width: %i height: %i format: %i\n", callback, w, h, format);
	OSStatus status = VDADecoderCreate (config, attr, (VDADecoderOutputCallback *) &callback, this, (OpaqueVDADecoder**) &decoder);

	CFRelease (config);
	CFRelease (attr);

	if (status != kVDADecoderNoErr) {
		g_warning ("VDADecoderCreate returned an error: %i", status);
		return;
	}
}

void
VDADecoder::Dispose ()
{
	g_assert_not_reached ();
}

void
VDADecoder::Cleanup (MediaFrame *frame)
{
	g_assert_not_reached ();
}

void
VDADecoder::CleanState ()
{
	g_assert_not_reached ();
}

void
VDADecoder::DecodeFrameAsyncInternal (MediaFrame *mf)
{
	g_assert_not_reached ();
}

/*
 * VDADecoderInfo
 */

bool
VDADecoderInfo::Supports (const char* codec)
{
	return codec != NULL && strcmp (codec, "h264") == 0;
}

IMediaDecoder*
VDADecoderInfo::Create (Media* media, IMediaStream* stream)
{
	return new VDADecoder (media, stream);
}

};
