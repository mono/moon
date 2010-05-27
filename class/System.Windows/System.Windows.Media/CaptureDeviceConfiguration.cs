// 
// CaptureDeviceConfiguration.cs.cs
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

		private static IntPtr GetAudioCaptureService ()
		{
			IntPtr capture_service = NativeMethods.runtime_get_capture_service ();
			if (capture_service == IntPtr.Zero)
				return IntPtr.Zero;
			return NativeMethods.moon_capture_service_get_audio_capture_service (capture_service);
		}

		private static IntPtr GetVideoCaptureService ()
		{
			IntPtr capture_service = NativeMethods.runtime_get_capture_service ();
			if (capture_service == IntPtr.Zero)
				return IntPtr.Zero;
			return NativeMethods.moon_capture_service_get_video_capture_service (capture_service);
		}

		public static Collection<AudioCaptureDevice> GetAvailableAudioCaptureDevices ()
		{
			IntPtr audio_service = GetAudioCaptureService ();
			if (audio_service == IntPtr.Zero)
				return new Collection<AudioCaptureDevice>(); // FIXME readonly?

			Collection<AudioCaptureDevice> col = new Collection<AudioCaptureDevice>();
			int num_devices;
			IntPtr devices = NativeMethods.moon_audio_capture_service_get_available_capture_devices (audio_service, out num_devices);
			for (int i = 0; i < num_devices; i ++)
				col.Add (new AudioCaptureDevice (Marshal.ReadIntPtr (devices, i * IntPtr.Size)));

			return col;
		}

		public static Collection<VideoCaptureDevice> GetAvailableVideoCaptureDevices ()
		{
			IntPtr video_service = GetVideoCaptureService ();
			if (video_service == IntPtr.Zero)
				return new Collection<VideoCaptureDevice>(); // FIXME readonly?

			Collection<VideoCaptureDevice> col = new Collection<VideoCaptureDevice>();
			int num_devices;
			IntPtr devices = NativeMethods.moon_video_capture_service_get_available_capture_devices (video_service, out num_devices);

			for (int i = 0; i < num_devices; i ++)
				col.Add (new VideoCaptureDevice (Marshal.ReadIntPtr (devices, i * IntPtr.Size)));

			return col;
		}

		public static AudioCaptureDevice GetDefaultAudioCaptureDevice ()
		{
			IntPtr audio_service = GetAudioCaptureService ();

			if (audio_service == IntPtr.Zero)
				return null;

			IntPtr audio_device = NativeMethods.moon_audio_capture_service_get_default_capture_device (audio_service);
			if (audio_device == IntPtr.Zero)
				return null;

			return new AudioCaptureDevice (audio_device);
		}

		public static VideoCaptureDevice GetDefaultVideoCaptureDevice ()
		{
			IntPtr video_service = GetVideoCaptureService ();

			if (video_service == IntPtr.Zero)
				return null;

			IntPtr video_device = NativeMethods.moon_video_capture_service_get_default_capture_device (video_service);
			if (video_device == IntPtr.Zero)
				return null;

			return new VideoCaptureDevice (video_device);
		}

		static bool last_device_access_response = false;

		public static bool RequestDeviceAccess ()
		{
			IntPtr capture_service = NativeMethods.runtime_get_capture_service ();
			if (capture_service == IntPtr.Zero)
				last_device_access_response = false;
			else
				last_device_access_response = NativeMethods.moon_capture_service_request_system_access (capture_service);
			return last_device_access_response;
		}

		public static bool AllowedDeviceAccess {
			get {
				return last_device_access_response;
			}
		}
	}
}

