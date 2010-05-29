/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * runtime.h: Core surface.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_CAPTURE_H__
#define __MOON_CAPTURE_H__

#include "dependencyobject.h"
#include "collection.h"
#include "pal.h"

/* @IncludeInKinds */
struct VideoFormat {
public:
	VideoFormat (MoonVideoFormat *pal_format);
	VideoFormat (const VideoFormat &format);

	float framesPerSecond;
	int height;
	int width;
	int stride;
	MoonPixelFormat pixelFormat;

	bool operator == (const VideoFormat &format)
	{
		return format.framesPerSecond == framesPerSecond && format.height == height && format.width == width && format.stride == stride && format.pixelFormat == pixelFormat;
	}

	bool operator != (const VideoFormat &format)
	{
		return !(*this == format);
	}

};

/* @IncludeInKinds */
struct AudioFormat {
public:
	AudioFormat (MoonAudioFormat *pal_format);

	int bitsPerSample;
	int channels;
	int samplesPerSecond;
	MoonWaveFormatType waveFormat;

	bool operator == (const AudioFormat &format)
	{
		return format.bitsPerSample == bitsPerSample && format.channels == channels && format.samplesPerSecond == samplesPerSecond && format.waveFormat == waveFormat;
	}

	bool operator != (const AudioFormat &format)
	{
		return !(*this == format);
	}
};

class SampleReadyEventArgs : public EventArgs {
public:
	SampleReadyEventArgs (gint64 sampleTime, gint64 frameDuration, guint8 *sampleData, int sampleDataLength)
		: sampleTime (sampleTime),
		  frameDuration (frameDuration),
		  sampleData (sampleData),
		  sampleDataLength (sampleDataLength)
	{
		SetObjectType (Type::SAMPLEREADYEVENTARGS);
	}

	/* @GeneratePInvoke,GenerateCBinding */
	gint64 GetSampleTime () { return sampleTime; }
	/* @GeneratePInvoke,GenerateCBinding */
	gint64 GetFrameDuration () { return frameDuration; }
	/* @GeneratePInvoke,GenerateCBinding */
	guint8* GetSampleData () { return sampleData; }
	/* @GeneratePInvoke,GenerateCBinding */
	int GetSampleDataLength () { return sampleDataLength; }
	
protected:
	virtual ~SampleReadyEventArgs () { };

private:
	gint64 sampleTime;
	gint64 frameDuration;
	guint8 *sampleData;
	int sampleDataLength;
};

class VideoFormatChangedEventArgs : public EventArgs {
public:
	VideoFormatChangedEventArgs (VideoFormat *newFormat)
	{
		SetObjectType (Type::VIDEOFORMATCHANGEDEVENTARGS);

		this->newFormat = new VideoFormat (*newFormat);
	}

	/* @GeneratePInvoke,GenerateCBinding */
	VideoFormat *GetNewFormat () { return newFormat; }
	
protected:
	virtual ~VideoFormatChangedEventArgs () { delete newFormat; };

private:
	VideoFormat *newFormat;
};
       
/* @Namespace=System.Windows.Media */
class CaptureSource : public DependencyObject {
protected:
	virtual ~CaptureSource ();

public:
	/* @GeneratePInvoke,GenerateCBinding */
	CaptureSource ();

	/* @PropertyType=AudioCaptureDevice,GenerateAccessors */
	const static int AudioCaptureDeviceProperty;

	/* @PropertyType=VideoCaptureDevice,GenerateAccessors */
	const static int VideoCaptureDeviceProperty;

	AudioCaptureDevice *GetAudioCaptureDevice ();
	void SetAudioCaptureDevice (AudioCaptureDevice *value);

	VideoCaptureDevice *GetVideoCaptureDevice ();
	void SetVideoCaptureDevice (VideoCaptureDevice *value);

	/* @GeneratePInvoke,GenerateCBinding */
	void CaptureImageAsync ();

	/* @GeneratePInvoke,GenerateCBinding */
	void Start ();

	/* @GeneratePInvoke,GenerateCBinding */
	void Stop ();

	enum State {
		Stopped,
		Started,
		Failed
	};

	/* @GeneratePInvoke,GenerateCBinding */
	int GetState ();

	// used by VideoBrush
	void GetSample (gint64 *sampleTime, gint64 *frameDuration, guint8 **sampleData, int *sampleDataLength);

	/* @DelegateType=EventHandler<ExceptionRoutedEventArgs> */
	const static int CaptureFailedEvent;

	/* @DelegateType=EventHandler<CaptureImageCompletedEventArgs> */
	const static int CaptureImageCompletedEvent;

	// internal events
	/* @DelegateType=EventHandler<SampleReadyEventArgs>,ManagedAccess=Internal */
	const static int SampleReadyEvent;
	/* @DelegateType=EventHandler<VideoFormatChangedEventArgs>,ManagedAccess=Internal */
	const static int FormatChangedEvent;
	/* @ManagedAccess=Internal */
	const static int CaptureStartedEvent;
	/* @ManagedAccess=Internal */
	const static int CaptureStoppedEvent;

private:
	static void ReportSampleCallback (gint64 sampleTime, gint64 frameDuration, guint8 *sampleData, int sampleDataLength, gpointer data);
	void ReportSample (gint64 sampleTime, gint64 frameDuration, guint8 *sampleData, int sampleDataLength);

	static void VideoFormatChangedCallback (MoonVideoFormat *format, gpointer data);
	void VideoFormatChanged (MoonVideoFormat *format);

	static void CaptureImageReportSampleCallback (gint64 sampleTime, gint64 frameDuration, guint8 *sampleData, int sampleDataLength, gpointer data);
	void CaptureImageReportSample (gint64 sampleTime, gint64 frameDuration, guint8 *sampleData, int sampleDataLength);

	static void CaptureImageVideoFormatChangedCallback (MoonVideoFormat *format, gpointer data);
	void CaptureImageVideoFormatChanged (MoonVideoFormat *format);

	State current_state;
	guint8 *cached_sampleData;
	int cached_sampleDataLength;
	gint64 cached_sampleTime;
	gint64 cached_frameDuration;

	bool need_image_capture;
	VideoFormat *capture_format;
};


/* @Namespace=System.Windows.Media */
class CaptureDevice : public DependencyObject {
public:
	/* @ManagedAccess=Internal,GeneratePInvoke,GenerateCBinding */
	CaptureDevice ();

	/* @PropertyType=string,GenerateAccessors */
	const static int FriendlyNameProperty;

	/* @PropertyType=bool,GenerateAccessors */
	const static int IsDefaultDeviceProperty;

	const char *GetFriendlyName ();
	void SetFriendlyName (const char *value);

	bool GetIsDefaultDevice ();
	void SetIsDefaultDevice (bool value);

	void Start ();
	void Stop ();

	MoonCaptureDevice* GetPalDevice () { return pal_device; }
	/* @GenerateCBinding,GeneratePInvoke */
	virtual void SetPalDevice (MoonCaptureDevice *device) { pal_device = device; }
protected:
	virtual ~CaptureDevice () {}

private:
	MoonCaptureDevice* pal_device;
};

/* @Namespace=System.Windows.Media */
class AudioFormatCollection : public Collection {
public:
	/* @GenerateCBinding,GeneratePInvoke */
	AudioFormatCollection ()
	{
		SetObjectType (Type::AUDIOFORMAT_COLLECTION);
	}

	virtual Type::Kind GetElementType () { return Type::AUDIOFORMAT; }

protected:
	virtual ~AudioFormatCollection () { }
};

/* @Namespace=System.Windows.Media */
class AudioCaptureDevice : public CaptureDevice {
public:
	/* @ManagedAccess=Internal,GeneratePInvoke,GenerateCBinding */
	AudioCaptureDevice ();

	/* @PropertyType=gint32,GenerateAccessors */
	const static int AudioFrameSizeProperty;

	/* @PropertyType=AudioFormatCollection,ManagedPropertyType=PresentationFrameworkCollection<AudioFormat>,ManagedFieldAccess=Private,GenerateManagedAccessors=false,GenerateAccessors */
	const static int SupportedFormatsProperty;

	/* @PropertyType=AudioFormat,ManagedFieldAccess=Private,GenerateAccessors */
	const static int DesiredFormatProperty;

	int GetAudioFrameSize ();
	void SetAudioFrameSize (int value);

	AudioFormatCollection* GetSupportedFormats ();
	void SetSupportedFormats (AudioFormatCollection *formats);

	AudioFormat* GetDesiredFormat ();
	void SetDesiredFormat (AudioFormat *format);

	virtual void SetPalDevice (MoonCaptureDevice *device);

	void Start ();
	void Stop ();

protected:
	virtual ~AudioCaptureDevice ();
};

/* @Namespace=System.Windows.Media */
class VideoFormatCollection : public Collection {
public:
	/* @GenerateCBinding,GeneratePInvoke */
	VideoFormatCollection ()
	{
		SetObjectType (Type::VIDEOFORMAT_COLLECTION);
	}

	virtual Type::Kind GetElementType () { return Type::VIDEOFORMAT; }

protected:
	virtual ~VideoFormatCollection () { }
};

/* @Namespace=System.Windows.Media */
class VideoCaptureDevice : public CaptureDevice {
public:
	/* @ManagedAccess=Internal,GeneratePInvoke,GenerateCBinding */
	VideoCaptureDevice ();

	/* @PropertyType=VideoFormatCollection,ManagedPropertyType=PresentationFrameworkCollection<VideoFormat>,ManagedFieldAccess=Private,GenerateManagedAccessors=false,GenerateAccessors */
	const static int SupportedFormatsProperty;

	/* @PropertyType=VideoFormat,ManagedFieldAccess=Private,GenerateAccessors */
	const static int DesiredFormatProperty;

	VideoFormat* GetDesiredFormat ();
	void SetDesiredFormat (VideoFormat *format);

	VideoFormatCollection* GetSupportedFormats ();
	void SetSupportedFormats (VideoFormatCollection *formats);

	void SetPalDevice (MoonCaptureDevice *device);

	void SetCallbacks (MoonReportSampleFunc report_sample,
			   MoonFormatChangedFunc format_changed,
			   gpointer data);

	void Start ();
	void Stop ();

protected:
	virtual ~VideoCaptureDevice ();

private:

	Collection *supported_formats;
};

#endif /* __MOON_CAPTURE_H__ */
