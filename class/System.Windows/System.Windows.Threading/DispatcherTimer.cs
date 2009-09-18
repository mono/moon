//
// DispatcherTimer.cs
//
// Copyright 2008 Novell, Inc.
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
using Mono;
using System.Windows.Interop;

namespace System.Windows.Threading {

	public class DispatcherTimer {
		Mono.DispatcherTimer internalTimer;
		bool started;

		public DispatcherTimer ()
		{
			internalTimer = new Mono.DispatcherTimer ();
			internalTimer.managedTimer = this;
		}

		~DispatcherTimer ()
		{
			internalTimer = null;
		}

		public void Start ()
		{
			if (!started) {
				started = true;
				Mono.NativeMethods.dispatcher_timer_start (internalTimer.native);
			}

		}

		public void Stop ()
		{
			if (started) {
				started = false;
				Mono.NativeMethods.dispatcher_timer_stop (internalTimer.native);
			}
		}

		public TimeSpan Interval {
			get {
				if (!internalTimer.Duration.HasTimeSpan)
					internalTimer.Duration = new Duration (TimeSpan.FromMilliseconds (0));
				return internalTimer.Duration.TimeSpan;
			}
			set {  internalTimer.Duration = new Duration (value); }
		}

		public bool IsEnabled {
			get { return started; }
		}

		public event EventHandler Tick {
			add {
				internalTimer.Tick += value;
			}
			remove {
				internalTimer.Tick -= value;
			}
		}

	}

}
