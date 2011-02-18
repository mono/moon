// 
// CaptureDeviceConfiguration.cs
// 
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
// 
// Copyright 2010 Novell, Inc.
// 
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// 

using System;
using System.Collections.ObjectModel;
using System.Runtime.InteropServices;
using Mono;

namespace System.Windows.Media {
	public static class CaptureDeviceConfiguration {
		static bool last_device_access_response = false;

		public static ReadOnlyCollection<AudioCaptureDevice> GetAvailableAudioCaptureDevices ()
		{
			AudioCaptureDeviceCollection col = new AudioCaptureDeviceCollection ();
			NativeMethods.capture_device_configuration_get_available_audio_capture_devices (col.native);
			return new ReadOnlyCollection<AudioCaptureDevice> (col);
		}

		public static ReadOnlyCollection<VideoCaptureDevice> GetAvailableVideoCaptureDevices ()
		{
			VideoCaptureDeviceCollection col = new VideoCaptureDeviceCollection ();
			NativeMethods.capture_device_configuration_get_available_video_capture_devices (col.native);
			return new ReadOnlyCollection<VideoCaptureDevice> (col);
		}

		public static AudioCaptureDevice GetDefaultAudioCaptureDevice ()
		{
			AudioCaptureDevice device;

			IntPtr audio_device = NativeMethods.capture_device_configuration_get_default_audio_capture_device ();
			if (audio_device == IntPtr.Zero)
				return null;

			device = (AudioCaptureDevice) NativeDependencyObjectHelper.FromIntPtr (audio_device);

			NativeMethods.event_object_unref (audio_device);

			return device;
		}

		public static VideoCaptureDevice GetDefaultVideoCaptureDevice ()
		{
			VideoCaptureDevice device;

			IntPtr video_device = NativeMethods.capture_device_configuration_get_default_video_capture_device ();
			if (video_device == IntPtr.Zero)
				return null;

			device = (VideoCaptureDevice) NativeDependencyObjectHelper.FromIntPtr (video_device);

			NativeMethods.event_object_unref (video_device);

			return device;
		}

		public static bool RequestDeviceAccess ()
		{
			last_device_access_response = NativeMethods.capture_device_configuration_request_system_access ();
			return last_device_access_response;
		}

		public static bool AllowedDeviceAccess {
			get {
				return last_device_access_response;
			}
		}
	}
}

