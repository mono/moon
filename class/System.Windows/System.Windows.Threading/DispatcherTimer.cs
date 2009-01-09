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
using System.Runtime.InteropServices;
using System.Windows.Interop;

namespace System.Windows.Threading {

	public class DispatcherTimer {
		private TimeSpan interval;
		private GCHandle handle;
		private uint source_id;
		
		static NativeMethods.GSourceFunc callback = new NativeMethods.GSourceFunc (timer_callback);

		[SecuritySafeCritical]
		public void Start ()
		{
			if (source_id != 0)
				return;

			handle = GCHandle.Alloc (this);
			source_id = NativeMethods.time_manager_add_timeout (NativeMethods.surface_get_time_manager (Application.s_surface), (int) interval.TotalMilliseconds, callback, GCHandle.ToIntPtr (handle));
		}

		[SecuritySafeCritical]
		public void Stop ()
		{
			if (source_id == 0)
				return;

			NativeMethods.time_manager_remove_timeout (NativeMethods.surface_get_time_manager (Application.s_surface), source_id);
			source_id = 0;
			handle.Free ();
		}

		public TimeSpan Interval {
			[SecuritySafeCritical]
			get { return interval; }

			[SecuritySafeCritical]
			set { interval = value; }
			
		}

		public bool IsEnabled {
			get { return source_id != 0; }
		}

		public event EventHandler Tick;

		static bool timer_callback (IntPtr data)
		{
			GCHandle handle = GCHandle.FromIntPtr (data);
			DispatcherTimer target = (DispatcherTimer) handle.Target;
			uint s_id = target.source_id;
			try {
				target.Tick (target, EventArgs.Empty);
				// If we are killed by Enabled or Stop, still return that value
				return target.source_id != 0 && s_id == target.source_id;
			} catch (Exception ex) {
				Console.WriteLine (ex.ToString ());
			} catch {
				Console.WriteLine ("DispatcherTimer.timer_callback exception.");
			}

			return false;
		}
	}

}
