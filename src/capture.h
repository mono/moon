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
#include "mutex.h"

namespace Moonlight {

/* @IncludeInKinds */
struct VideoFormat {
public:
	VideoFormat (MoonPixelFormat format, float framesPerSecond, int stride, int width, int height);
	VideoFormat (const VideoFormat &format);
	VideoFormat ();

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
	AudioFormat (MoonWaveFormatType format, int bitsPerSample, int channels, int samplesPerSecond);
	AudioFormat ();

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

/* @Namespace=None */
class SampleReadyEventArgs : public EventArgs {
public:
	SampleReadyEventArgs (gint64 sampleTime, gint64 frameDuration, void *sampleData, int sampleDataLength)
		: EventArgs (Type::SAMPLEREADYEVENTARGS),
		  sampleTime (sampleTime),
		  frameDuration (frameDuration),
		  sampleData (sampleData),
		  sampleDataLength (sampleDataLength)
	{
	}

	/* @GeneratePInvoke */
	gint64 GetSampleTime () { return sampleTime; }
	/* @GeneratePInvoke */
	gint64 GetFrameDuration () { return frameDuration; }
	/* @GeneratePInvoke */
	void* GetSampleData () { return sampleData; }
	/* @GeneratePInvoke */
	int GetSampleDataLength () { return sampleDataLength; }
	
protected:
	virtual ~SampleReadyEventArgs () { };

private:
	gint64 sampleTime;
	gint64 frameDuration;
	void *sampleData;
	int sampleDataLength;
};

/* @Namespace=None */
class CaptureFormatChangedEventArgs : public EventArgs {
public:
	CaptureFormatChangedEventArgs (VideoFormat *newFormat)
		: EventArgs (Type::CAPTUREFORMATCHANGEDEVENTARGS),
		newVideoFormat (*newFormat)
	{
	}
	CaptureFormatChangedEventArgs (AudioFormat *newFormat)
		: EventArgs (Type::CAPTUREFORMATCHANGEDEVENTARGS),
		newAudioFormat (*newFormat)
	{
	}

	/* @GeneratePInvoke */
	VideoFormat *GetNewVideoFormat () { return &newVideoFormat; }
	/* @GeneratePInvoke */
	AudioFormat *GetNewAudioFormat () { return &newAudioFormat; }
	
protected:
	virtual ~CaptureFormatChangedEventArgs () { }

private:
	VideoFormat newVideoFormat;
	AudioFormat newAudioFormat;
};

/* @Namespace=System.Windows.Media */
class CaptureSource : public DependencyObject {
public:
	/* @GeneratePInvoke */
	CaptureSource ();

	/* @PropertyType=AudioCaptureDevice,GenerateAccessors */
	const static int AudioCaptureDeviceProperty;

	/* @PropertyType=VideoCaptureDevice,GenerateAccessors */
	const static int VideoCaptureDeviceProperty;

	AudioCaptureDevice *GetAudioCaptureDevice ();
	void SetAudioCaptureDevice (AudioCaptureDevice *value);

	VideoCaptureDevice *GetVideoCaptureDevice ();
	void SetVideoCaptureDevice (VideoCaptureDevice *value);

	/* @GeneratePInvoke */
	void CaptureImageAsync ();

	/* @GeneratePInvoke */
	void Start ();

	/* @GeneratePInvoke */
	void Stop ();

	enum State {
		Stopped,
		Started,
		Failed
	};

	/* @GeneratePInvoke */
	int GetState ();

	// used by VideoBrush
	void GetSample (gint64 *sampleTime, gint64 *frameDuration, void **sampleData, int *sampleDataLength);

	/* @DelegateType=EventHandler<ExceptionRoutedEventArgs> */
	const static int CaptureFailedEvent;

	/* @DelegateType=EventHandler<CaptureImageCompletedEventArgs> */
	const static int CaptureImageCompletedEvent;

	// internal events
	/* @DelegateType=EventHandler<SampleReadyEventArgs>,ManagedAccess=Internal */
	const static int SampleReadyEvent;
	/* @DelegateType=EventHandler<CaptureFormatChangedEventArgs>,ManagedAccess=Internal */
	const static int FormatChangedEvent;
	/* @ManagedAccess=Internal */
	const static int CaptureStartedEvent;
	/* @ManagedAccess=Internal */
	const static int CaptureStoppedEvent;

	// the callee is given ownership of the sampleData buffer
	void ReportSample (gint64 sampleTime, gint64 frameDuration, void *sampleData, int sampleDataLength); // thread-safe
	void VideoFormatChanged (VideoFormat *format);
	void AudioFormatChanged (AudioFormat *format);
	void CaptureImageReportSample (gint64 sampleTime, gint64 frameDuration, void *sampleData, int sampleDataLength);

protected:

	virtual ~CaptureSource ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

private:
	State current_state;
	void *cached_sampleData;
	int cached_sampleDataLength;
	gint64 cached_sampleTime;
	gint64 cached_frameDuration;

	bool need_image_capture;
	VideoFormat *video_capture_format;

	static void OnSampleReadyCallback (EventObject *obj);
	void OnSampleReady ();

	List samples;
	Mutex mutex;

	class SampleData : public List::Node {
	public:
		gint64 sampleTime;
		gint64 frameDuration;
		void *sampleData;
		gsize sampleDataLength;
		bool copy_data;

		virtual ~SampleData ()
		{
			if (copy_data)
				g_free (sampleData);
		}
	};
};

/* @Namespace=System.Windows.Media */
class CaptureDevice : public DependencyObject {
public:
	/* @PropertyType=string,GenerateAccessors,ReadOnly */
	const static int FriendlyNameProperty;

	/* @PropertyType=bool,GenerateAccessors,ReadOnly,DefaultValue=false */
	const static int IsDefaultDeviceProperty;

	const char *GetFriendlyName ();
	void SetFriendlyName (const char *value);

	bool GetIsDefaultDevice ();
	void SetIsDefaultDevice (bool value);

	void Start ();
	void Stop ();

	MoonCaptureDevice* GetPalDevice () { return pal_device; }
	/* @GeneratePInvoke */
	virtual void SetPalDevice (MoonCaptureDevice *device) { pal_device = device; }
	void SetCaptureSource (CaptureSource *value);
	CaptureSource *GetCaptureSource () { return capture_source; }

protected:
	/* @ManagedAccess=None */
	CaptureDevice (Type::Kind object_type);

	virtual ~CaptureDevice ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

private:
	MoonCaptureDevice* pal_device;
	CaptureSource *capture_source;

	/* @ManagedAccess=None */
	CaptureDevice () {}
};

/* @Namespace=System.Windows.Media */
class AudioFormatCollection : public Collection {
public:
	virtual Type::Kind GetElementType () { return Type::AUDIOFORMAT; }

protected:
	/* @GeneratePInvoke */
	AudioFormatCollection () : Collection (Type::AUDIOFORMAT_COLLECTION) { }

	virtual ~AudioFormatCollection () { }

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Media */
class AudioCaptureDevice : public CaptureDevice {
public:
	/* @PropertyType=gint32,GenerateAccessors,DefaultValue=1000 */
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
	/* @ManagedAccess=Internal,GeneratePInvoke */
	AudioCaptureDevice ();

	virtual ~AudioCaptureDevice ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Media */
class VideoFormatCollection : public Collection {
public:
	virtual Type::Kind GetElementType () { return Type::VIDEOFORMAT; }

protected:
	/* @GeneratePInvoke */
	VideoFormatCollection () : Collection (Type::VIDEOFORMAT_COLLECTION) { }

	virtual ~VideoFormatCollection () { }

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Media */
class VideoCaptureDevice : public CaptureDevice {
public:
	/* @PropertyType=VideoFormatCollection,ManagedPropertyType=PresentationFrameworkCollection<VideoFormat>,ManagedFieldAccess=Private,GenerateManagedAccessors=false,GenerateAccessors */
	const static int SupportedFormatsProperty;

	/* @PropertyType=VideoFormat,ManagedFieldAccess=Private,GenerateAccessors */
	const static int DesiredFormatProperty;

	VideoFormat* GetDesiredFormat ();
	void SetDesiredFormat (VideoFormat *format);

	VideoFormatCollection* GetSupportedFormats ();
	void SetSupportedFormats (VideoFormatCollection *formats);

	void SetPalDevice (MoonCaptureDevice *device);

	void Start ();
	void Stop ();

protected:
	/* @ManagedAccess=Internal,GeneratePInvoke */
	VideoCaptureDevice ();

	virtual ~VideoCaptureDevice ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

private:

	Collection *supported_formats;
};

class CaptureDeviceConfiguration {
private:
	static CaptureDevice *SelectDefaultDevice (DependencyObjectCollection *col, const char *type);

public:
	// the caller must unref the returned object
	/* @GeneratePInvoke */
	static AudioCaptureDevice* GetDefaultAudioCaptureDevice ();
	// the caller must unref the returned object
	/* @GeneratePInvoke */
	static VideoCaptureDevice* GetDefaultVideoCaptureDevice ();

	/* @GeneratePInvoke */
	static void GetAvailableVideoCaptureDevices (VideoCaptureDeviceCollection *col);

	/* @GeneratePInvoke */
	static void GetAvailableAudioCaptureDevices (AudioCaptureDeviceCollection *col);

	/* @GeneratePInvoke */
	static bool RequestSystemAccess ();
};

};
#endif /* __MOON_CAPTURE_H__ */
