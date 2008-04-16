//
// Storyboard.cs
//
// Author:
//   Miguel de Icaza (miguel@novell.com)
//
// Copyright 2007 Novell, Inc.
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
using System.Windows.Media;
using System.Runtime.InteropServices;
using Mono;

namespace System.Windows.Media.Animation {

	public sealed class Storyboard : ParallelTimeline {
		public static readonly DependencyProperty TargetPropertyProperty;
		public static readonly DependencyProperty TargetNameProperty;

		static Storyboard ()
		{
			TargetPropertyProperty = DependencyProperty.Lookup (Kind.STORYBOARD, "TargetProperty", typeof (string));
			TargetNameProperty = DependencyProperty.Lookup (Kind.STORYBOARD, "TargetName", typeof (string));
		}

		internal Storyboard (IntPtr raw) : base (raw)
		{
		}

		public Storyboard ()
			: base (Mono.NativeMethods.storyboard_new ())
		{
		}
		
		public void Begin ()
		{
			NativeMethods.storyboard_begin (native);
		}

		public void Pause ()
		{
			NativeMethods.storyboard_pause (native);
		}

		public void Resume ()
		{
			NativeMethods.storyboard_resume (native);
		}

		public void Seek (TimeSpan time_span)
		{
			NativeMethods.storyboard_seek (native, time_span.Ticks);
		}

		public void Stop ()
		{
			NativeMethods.storyboard_stop (native);
		}


		static object CompletedEvent = new object ();
		public event EventHandler Completed {
			add {
				if (events[CompletedEvent] == null)
					Events.AddHandler (this, "Completed", completed_proxy);
				events.AddHandler (CompletedEvent, value);
			}
			remove {
				events.RemoveHandler (CompletedEvent, value);
				if (events[CompletedEvent] == null)
					Events.RemoveHandler (this, "Completed", completed_proxy);
			}
		}

		static UnmanagedEventHandler completed_proxy = new UnmanagedEventHandler (UnmanagedCompleted);

		private static void UnmanagedCompleted (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			Storyboard sb = (Storyboard) Helper.GCHandleFromIntPtr (closure).Target;
			sb.InvokeCompleted ();
		}

		private void InvokeCompleted ()
		{
			EventHandler h = (EventHandler)events[CompletedEvent];
			if (h != null)
				h (this, EventArgs.Empty);
		}

		internal override Kind GetKind ()
		{
			return Kind.STORYBOARD;
		}
	}
}
