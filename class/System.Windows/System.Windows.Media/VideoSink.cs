//
// VideoSink.cs
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

namespace System.Windows.Media {
	public abstract class VideoSink {
		public VideoSink ()
		{
		}

		protected abstract void OnCaptureStarted ();
		protected abstract void OnCaptureStopped ();
		protected abstract void OnFormatChange (VideoFormat videoFormat);
		protected abstract void OnSample (long sampleTimeInHundredNanoseconds, long frameDurationInHundredNanoseconds, byte[] sampleData);
	
		CaptureSource source;
		public CaptureSource CaptureSource {
			get {
				return source;
			}
			set {
				if (source != null) {
					source.SampleReady -= sample_ready;
					source.FormatChanged -= format_changed;
					source.CaptureStarted -= capture_started;
					source.CaptureStopped -= capture_stopped;
				}
				source = value;
				if (source != null) {
					source.SampleReady += sample_ready;
					source.FormatChanged += format_changed;
					source.CaptureStarted += capture_started;
					source.CaptureStopped += capture_stopped;
				}
			}
		}

		void sample_ready (object sender, SampleReadyEventArgs args)
		{
			OnSample (args.SampleTime, args.FrameDuration, args.SampleData);
		}

		void capture_started (object sender, EventArgs args)
		{
			OnCaptureStarted ();
		}

		void capture_stopped (object sender, EventArgs args)
		{
			OnCaptureStopped ();
		}

		void format_changed (object sender, CaptureFormatChangedEventArgs args)
		{
			OnFormatChange (args.NewVideoFormat);
		}

		~VideoSink ()
		{
			// required for API compatibility.
			// Maybe we'll need to do something here...
		}
	}
}
