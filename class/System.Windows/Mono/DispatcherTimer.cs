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
using System.Windows;
using System.Windows.Media.Animation;

namespace Mono
{
	internal partial class DispatcherTimer : Timeline
	{
		public System.Windows.Threading.DispatcherTimer managedTimer;

		static UnmanagedEventHandler tick_proxy = Events.CreateSafeHandler (UnmanagedTick);

		private static void UnmanagedTick (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			Mono.DispatcherTimer timer = (Mono.DispatcherTimer) Helper.GCHandleFromIntPtr (closure).Target;
			timer.OnTick ();
		}

		private void OnTick ()
		{
			EventHandler h = (EventHandler)events[TickEvent];
			if (h != null) {
				try {
					h (managedTimer, EventArgs.Empty);
				} catch (Exception ex) {
					Application.OnUnhandledException (this, ex);
					Mono.NativeMethods.dispatcher_timer_stop (native);
				}
			}
		}

		static object TickEvent = new object ();
		public event EventHandler Tick {
			add {
				if (events[TickEvent] == null)
					Events.AddHandler (this, "Tick", tick_proxy);
				events.AddHandler (TickEvent, value);
			}
			remove {
				events.RemoveHandler (TickEvent, value);
				if (events[TickEvent] == null)
					Events.RemoveHandler (this, "Tick", tick_proxy);
			}
		}
	}
}
