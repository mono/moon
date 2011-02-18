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
	internal class SampleReadyEventArgs : EventArgs {
		internal SampleReadyEventArgs (IntPtr raw)
		{
			SampleTime = NativeMethods.sample_ready_event_args_get_sample_time (raw);
			FrameDuration = NativeMethods.sample_ready_event_args_get_frame_duration (raw);

			int sample_data_length = NativeMethods.sample_ready_event_args_get_sample_data_length (raw);
			IntPtr sample_data = NativeMethods.sample_ready_event_args_get_sample_data (raw);

			SampleData = new byte [sample_data_length];

			Marshal.Copy (sample_data, SampleData, 0, sample_data_length);
		}

		public long SampleTime {
			get; private set;
		}

		public long FrameDuration {
			get; private set;
		}

		public byte[] SampleData {
			get; private set;
		}
	}
}
