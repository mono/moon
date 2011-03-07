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
class VideoFormat {
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
class AudioFormat {
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

	// Returns the number of bytes required to hold the specified duration of audio
	double GetByteCount (guint32 milliseconds)
	{
		return (milliseconds * (bitsPerSample / 8) * channels * samplesPerSecond) / (double) 1000;
	}

	// Returns the duration (in pts) of the specifed number of bytes.
	guint64 GetDuration (guint32 bytes)
	{
		return bytes * 10000000ULL / channels / (bitsPerSample / 8) / samplesPerSecond;
	}
};

/* @Namespace=None */
class SampleReadyEventArgs : public EventArgs {
public:
	SampleReadyEventArgs (gint64 sampleTime, gint64 frameDuration, void *sampleData, int sampleDataLength, AudioFormat *aformat, VideoFormat *vformat)
		: EventArgs (Type::SAMPLEREADYEVENTARGS),
		  sampleTime (sampleTime),
		  frameDuration (frameDuration),
		  sampleData (sampleData),
		  sampleDataLength (sampleDataLength),
		  video_format (vformat),
		  audio_format (aformat)
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

	VideoFormat *GetVideoFormat () { return video_format; }
	AudioFormat *GetAudioFormat () { return audio_format; }

protected:
	virtual ~SampleReadyEventArgs () { };

private:
	gint64 sampleTime;
	gint64 frameDuration;
	void *sampleData;
	int sampleDataLength;
	VideoFormat *video_format;
	AudioFormat *audio_format;
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

	EVENTHANDLER (CaptureSource, SampleReady, CaptureDevice, SampleReadyEventArgs);
	EVENTHANDLER (CaptureSource, FormatChanged, CaptureDevice, CaptureFormatChangedEventArgs);
	EVENTHANDLER (CaptureSource, CaptureStarted, CaptureDevice, EventArgs);

	virtual void OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error);

protected:
	virtual ~CaptureSource () {}

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

private:
	State current_state;

	bool pending_capture;

	void EmitAudioSample (AudioFormat *format, void *sampleData, int sampleDataLength);
};

/* @Namespace=System.Windows.Media */
class CaptureDevice : public DependencyObject {
public:
	/* @PropertyType=string,GenerateAccessors,ReadOnly */
	const static int FriendlyNameProperty;

	/* @PropertyType=bool,GenerateAccessors,ReadOnly,DefaultValue=false */
	const static int IsDefaultDeviceProperty;

	// properties
	const char *GetFriendlyName ();
	void SetFriendlyName (const char *value);

	bool GetIsDefaultDevice ();
	void SetIsDefaultDevice (bool value);

	const static int SampleReadyEvent;
	const static int FormatChangedEvent;
	const static int CaptureStartedEvent;

	void Start ();
	void Stop ();

	// the callee is given ownership of the sampleData buffer for both audio and video
	void EmitAudioSampleReady (AudioFormat *format, void *sampleData, int sampleDataLength); // thread-safe
	void EmitVideoSampleReady (VideoFormat *format, void *sampleData, int sampleDataLength); // thread-safe
	void EmitCaptureStarted (); // thread-safe
	void EmitVideoFormatChanged (VideoFormat *format); // thread-safe
	void EmitAudioFormatChanged (AudioFormat *format); // thread-safe

	virtual void SetPalDevice (MoonCaptureDevice *device);

	virtual void Dispose ();

protected:
	/* @ManagedAccess=None */
	CaptureDevice (Type::Kind object_type);
	virtual ~CaptureDevice ();

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

private:
	MoonCaptureDevice* pal_device; // main thread only
	List events; // list of events to be emitted (on the main thread). thread-safe
	Mutex events_mutex;

	guint8 *audio_buffer;
	guint32 audio_buffer_size;
	guint32 audio_buffer_used;
	guint64 audio_position;

	guint64 start_position;

	MoonCaptureDevice* GetPalDevice ();

	/* @ManagedAccess=None */
	CaptureDevice () {}

	class EventData : public List::Node {
	public:
		int event;
		void *sampleData;
		int sampleDataLength;
		VideoFormat *vformat;
		AudioFormat *aformat;

		EventData (int event)                                                               : event (event), sampleData (NULL), sampleDataLength (0), vformat (NULL), aformat (NULL) {}
		EventData (int event, VideoFormat *format)                                          : event (event), sampleData (NULL), sampleDataLength (0), vformat (format), aformat (NULL) {}
		EventData (int event, AudioFormat *format)                                          : event (event), sampleData (NULL), sampleDataLength (0), vformat (NULL), aformat (format) {}
		EventData (int event, VideoFormat *format, void *sampleData, int sampleDataLength) : event (event), sampleData (sampleData), sampleDataLength (sampleDataLength), vformat (format), aformat (NULL) {}
		EventData (int event, AudioFormat *format, void *sampleData, int sampleDataLength) : event (event), sampleData (sampleData), sampleDataLength (sampleDataLength), vformat (NULL), aformat (format) {}

		virtual ~EventData ()
		{
			g_free (sampleData);
			delete vformat;
			delete aformat;
		}
	};

	static void EmitEventCallback (EventObject *sender);
	void EmitEvent ();
};

/* @Namespace=System.Windows.Media */
class AudioFormatCollection : public Collection {
public:
	virtual Type::Kind GetElementType () { return Type::AUDIOFORMAT; }

protected:
	/* @GeneratePInvoke */
	AudioFormatCollection () : Collection (Type::AUDIOFORMAT_COLLECTION) { }
	virtual ~AudioFormatCollection () {}

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;
};

/* @Namespace=System.Windows.Media */
class AudioCaptureDevice : public CaptureDevice {
public:
	// We do not generate accessors here, to force the usage of GetClampedAudioFrameSize */
	/* @PropertyType=gint32,DefaultValue=1000 */
	const static int AudioFrameSizeProperty;

	/* @PropertyType=AudioFormatCollection,ManagedPropertyType=PresentationFrameworkCollection<AudioFormat>,ManagedFieldAccess=Private,GenerateManagedAccessors=false,GenerateAccessors */
	const static int SupportedFormatsProperty;

	/* @PropertyType=AudioFormat,ManagedFieldAccess=Private,GenerateAccessors */
	const static int DesiredFormatProperty;

	AudioFormatCollection* GetSupportedFormats ();
	void SetSupportedFormats (AudioFormatCollection *formats);

	AudioFormat* GetDesiredFormat ();
	void SetDesiredFormat (AudioFormat *format);

	virtual void SetPalDevice (MoonCaptureDevice *device);

	int GetClampedAudioFrameSize ();

protected:
	/* @ManagedAccess=Internal,GeneratePInvoke */
	AudioCaptureDevice ();
	virtual ~AudioCaptureDevice () {}

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
	virtual ~VideoFormatCollection () {}

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

	virtual void SetPalDevice (MoonCaptureDevice *device);

protected:
	/* @ManagedAccess=Internal,GeneratePInvoke */
	VideoCaptureDevice ();

	virtual ~VideoCaptureDevice () {}

	friend class MoonUnmanagedFactory;
	friend class MoonManagedFactory;

private:

	Collection *supported_formats;
};

class MOON_API CaptureDeviceConfiguration {
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
};

};
#endif /* __MOON_CAPTURE_H__ */
