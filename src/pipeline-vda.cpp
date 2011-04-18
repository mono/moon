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
#include "pipeline-mp4.h"
#include "pipeline.h"
#include "mp3.h"
#include "clock.h"
#include "debug.h"
#include "deployment.h"

namespace Moonlight {

void
register_vda ()
{
	Media::RegisterDecoder (new MoonVDADecoderInfo ());
}

/*
 * MoonVDADecoder
 */

MoonVDADecoder::MoonVDADecoder (Media* media, IMediaStream* stream)
	: IMediaDecoder (Type::MOONVDADECODER, media, stream)
{
	this->decoder = NULL;
}

void
MoonVDADecoder::InputEnded ()
{
	GetStream ()->SetOutputEnded (true);
}

static void
VDADecoderCallback (void *decompressionOutputRefCon, CFDictionaryRef frameInfo, OSStatus status, uint32_t infoFlags, CVImageBufferRef imageBuffer)
{
	MoonVDADecoder *decoder = (MoonVDADecoder *) decompressionOutputRefCon;
	VideoStream *vs = (VideoStream *) decoder->GetStream ();

	// FIXME: Is this always 1 thread?  Can we optimize this
	decoder->GetDeployment ()->RegisterThread ();

	Deployment::SetCurrent (decoder->GetDeployment ());

	if (imageBuffer == NULL) {
		return;
	}

	OSType format_type = CVPixelBufferGetPixelFormatType (imageBuffer);
	if (format_type != kCVPixelFormatType_422YpCbCr8) {
		g_warning ("Mismatched format in VDA");
		return;
	}

	MediaFrame *mf = (MediaFrame *) CFDictionaryGetValue (frameInfo, CFSTR ("MoonMediaFrame"));

	mf->AddState (MediaFramePlanar);
	mf->FreeBuffer ();
	mf->SetBufLen (0);

	mf->srcSlideY = 0;
	mf->srcSlideH = vs->GetHeight ();

	mf->width = vs->GetWidth ();
	mf->height = vs->GetHeight ();

	CVPixelBufferLockBaseAddress (imageBuffer, 0);

	mf->data_stride [0] = (uint8_t *) CVPixelBufferGetBaseAddress (imageBuffer);
	mf->srcStride [0] = CVPixelBufferGetBytesPerRow (imageBuffer);

	mf->AddState (MediaFrameDecoded);

	mf->decoder_specific_data = imageBuffer;
	CVPixelBufferRetain (imageBuffer);

	decoder->ReportDecodeFrameCompleted (mf);

	mf->unref ();
}

OSStatus
MoonVDADecoder::CreateDecoder (SInt32 inHeight, SInt32 inWidth, OSType inSourceFormat, CFDataRef inAVCCData)
{
	OSStatus status;

	CFMutableDictionaryRef decoderConfiguration = NULL;
	CFMutableDictionaryRef destinationImageBufferAttributes = NULL;
	CFDictionaryRef emptyDictionary;

	CFNumberRef height = NULL;
	CFNumberRef width= NULL;
	CFNumberRef sourceFormat = NULL;
	CFNumberRef pixelFormat = NULL;

	// source must be H.264
	if (inSourceFormat != 'avc1') {
		fprintf (stderr, "Source format is not H.264!\n");
		return paramErr;
	}

	// the avcC data chunk from the bitstream must be present
	if (inAVCCData == NULL) {
		fprintf (stderr, "avc1 decoder configuration data cannot be NULL!\n");
		return paramErr;
	}

	decoderConfiguration = CFDictionaryCreateMutable (kCFAllocatorDefault, 4, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

	height = CFNumberCreate (kCFAllocatorDefault, kCFNumberSInt32Type, &inHeight);
	width = CFNumberCreate (kCFAllocatorDefault, kCFNumberSInt32Type, &inWidth);
	sourceFormat = CFNumberCreate (kCFAllocatorDefault, kCFNumberSInt32Type, &inSourceFormat);

	CFDictionarySetValue (decoderConfiguration, kVDADecoderConfiguration_Height, height);
	CFDictionarySetValue (decoderConfiguration, kVDADecoderConfiguration_Width, width);
	CFDictionarySetValue (decoderConfiguration, kVDADecoderConfiguration_SourceFormat, sourceFormat);
	CFDictionarySetValue (decoderConfiguration, kVDADecoderConfiguration_avcCData, inAVCCData);

	destinationImageBufferAttributes = CFDictionaryCreateMutable (kCFAllocatorDefault, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

	OSType cvPixelFormatType = kCVPixelFormatType_422YpCbCr8;
	pixelFormat = CFNumberCreate (kCFAllocatorDefault, kCFNumberSInt32Type, &cvPixelFormatType);
	emptyDictionary = CFDictionaryCreate (kCFAllocatorDefault, NULL, NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

	CFDictionarySetValue (destinationImageBufferAttributes, kCVPixelBufferPixelFormatTypeKey, pixelFormat);
	CFDictionarySetValue (destinationImageBufferAttributes, kCVPixelBufferIOSurfacePropertiesKey, emptyDictionary);

	status = VDADecoderCreate (decoderConfiguration, destinationImageBufferAttributes, (VDADecoderOutputCallback*) VDADecoderCallback, this, (VDADecoder*) &decoder);

	if (decoderConfiguration) CFRelease (decoderConfiguration);
	if (destinationImageBufferAttributes) CFRelease (destinationImageBufferAttributes);
	if (emptyDictionary) CFRelease (emptyDictionary);

	return status;
}

void
MoonVDADecoder::OpenDecoderAsyncInternal ()
{
	IMediaStream *stream = GetStream ();
	VideoStream *vs = (VideoStream *) stream;
	int format = 'avc1';

	CFDataRef avcCData = CFDataCreate (kCFAllocatorDefault, (const uint8_t*) stream->GetRawExtraData (), stream->GetRawExtraDataSize ());
	OSStatus status = CreateDecoder ((SInt32) vs->GetHeight (), (SInt32) vs->GetWidth (), (OSType) format, avcCData);

	if (avcCData) CFRelease (avcCData);

	if (status == kVDADecoderNoErr) {
		SetPixelFormat (MoonPixelFormat422YpCbCr8);

		ReportOpenDecoderCompleted ();
	} else {
		char *str = g_strdup_printf ("MoonVDADecoder failed to open codec (result: %d)", status);
		ReportErrorOccurred (str);
		g_free (str);
	}
}

void
MoonVDADecoder::Dispose ()
{
	VDADecoderDestroy (decoder);
}

void
MoonVDADecoder::Cleanup (MediaFrame *frame)
{
	CVPixelBufferUnlockBaseAddress ((CVPixelBufferRef) frame->decoder_specific_data, 0);
	CVPixelBufferRelease ((CVPixelBufferRef) frame->decoder_specific_data);
}

void
MoonVDADecoder::CleanState ()
{
	VDADecoderFlush (decoder, 0);
}

void
MoonVDADecoder::DecodeFrameAsyncInternal (MediaFrame *mf)
{
	CFDictionaryValueCallBacks valueCallbacks = {0, 0, 0, 0, 0};
	CFMutableDictionaryRef frameInfo = CFDictionaryCreateMutable (kCFAllocatorDefault, 1, &kCFTypeDictionaryKeyCallBacks, &valueCallbacks);
	CFDataRef compressedBuffer = CFDataCreate (kCFAllocatorDefault, (const uint8_t*) mf->GetBuffer (), mf->GetBufLen ());
	
	CFDictionarySetValue (frameInfo, CFSTR ("MoonMediaFrame"), mf);

	OSStatus status = VDADecoderDecode (decoder, 0, compressedBuffer, frameInfo);

	if (status != kVDADecoderNoErr) {
		g_warning ("VDADecoderDecode returned: %i\n", status);
	}

	if (compressedBuffer) CFRelease (compressedBuffer);
	if (frameInfo) CFRelease (frameInfo);

	mf->ref ();
}

/*
 * VDADecoderInfo
 */

bool
MoonVDADecoderInfo::Supports (const char* codec)
{
	return codec != NULL && strcmp (codec, "h264") == 0;
}

IMediaDecoder*
MoonVDADecoderInfo::Create (Media* media, IMediaStream* stream)
{
	Mp4Demuxer *demuxer = (Mp4Demuxer *) stream->GetDemuxerReffed ();

	demuxer->SetNeedsRawFrames (true);

	demuxer->unref ();
	return new MoonVDADecoder (media, stream);
}

};
