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
using System.Security;
using Mono;
using System.Windows.Interop;

namespace System.Windows.Threading {

	public class DispatcherTimer {
		private TimeSpan interval;
		uint source_id;
		NativeMethods.GSourceFunc callback;

#if NET_2_1
		[SecuritySafeCritical]
#endif
		public void Start ()
		{
			if (source_id != 0)
				return;

			callback = new NativeMethods.GSourceFunc (timer_callback);
			IntPtr handle = PluginHost.Handle;

			if (handle != IntPtr.Zero)
				source_id = NativeMethods.plugin_timer_timeout_add (
					handle, (int) interval.TotalMilliseconds, callback, IntPtr.Zero);
			else
				source_id = NativeMethods.runtime_timer_timeout_add (
					(int) interval.TotalMilliseconds, callback, IntPtr.Zero);
		}

#if NET_2_1
		[SecuritySafeCritical]
#endif
		public void Stop ()
		{
			if (source_id == 0)
				return;

			IntPtr handle = PluginHost.Handle;
			if (handle != IntPtr.Zero)
				NativeMethods.plugin_timer_timeout_stop (PluginHost.Handle, source_id);
			else
				NativeMethods.runtime_timer_timeout_stop (source_id);
			source_id = 0;
			callback = null;
		}

		public TimeSpan Interval {
#if NET_2_1
			[SecuritySafeCritical]
#endif
			get { return interval; }

#if NET_2_1
			[SecuritySafeCritical]
#endif
			set { interval = value; }
			
		}

		public bool IsEnabled {
			get { return source_id != 0; }
		}

		public event EventHandler Tick;

		bool timer_callback (IntPtr data)
		{
			Tick (this, EventArgs.Empty);

			// If we are killed by Enabled or Stop, still return that value
			return source_id != 0;
		}
	}

}
