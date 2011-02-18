/*
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

using System;
using System.Runtime.InteropServices;
using Mono;

namespace System.Windows.Media {
	internal class CaptureFormatChangedEventArgs : EventArgs {
		internal CaptureFormatChangedEventArgs (IntPtr raw)
		{
			IntPtr v = NativeMethods.capture_format_changed_event_args_get_new_video_format (raw);
			IntPtr a = NativeMethods.capture_format_changed_event_args_get_new_audio_format (raw);

			unsafe {
				NewVideoFormat = ((UnmanagedVideoFormat *) v)->ToVideoFormat ();
				NewAudioFormat = ((UnmanagedAudioFormat *) a)->ToAudioFormat ();
			}
		}

		public VideoFormat NewVideoFormat {
			get; private set;
			}
	
		public AudioFormat NewAudioFormat {
			get; private set;
		}
	}
}
