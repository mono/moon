//
// System.Windows.Controls.MultiScaleImage.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2008 Novell, Inc (http://www.novell.com)
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

using Mono;
using System.Security;

namespace System.Windows.Controls {
	public partial class MultiScaleImage : FrameworkElement {

#if NET_2_1
		[SecuritySafeCritical]
#endif
		public void ZoomAboutLogicalPoint (double zoomIncrementFactor, double zoomCenterLogicalX, double zoomCenterLogicalY)
		{
			NativeMethods.multi_scale_image_zoom_about_logical_point (this.native, zoomIncrementFactor, zoomCenterLogicalX, zoomCenterLogicalY);
		}

		static object ImageFailedEvent = new object ();
		static object ImageOpenFailedEvent = new object ();
		static object ImageOpenSucceededEvent = new object ();
		static object MotionFinishedEvent = new object ();
		static object ViewportChangedEvent = new object ();

		static UnmanagedEventHandler image_failed = new UnmanagedEventHandler (image_failed_cb);
		static UnmanagedEventHandler image_open_failed = new UnmanagedEventHandler (image_open_failed_cb);
		static UnmanagedEventHandler image_open_succeeded = new UnmanagedEventHandler (image_open_succeeded_cb);
		static UnmanagedEventHandler motion_finished = new UnmanagedEventHandler (motion_finished_cb);
		static UnmanagedEventHandler viewport_changed = new UnmanagedEventHandler (viewport_changed_cb);

		public event RoutedEventHandler ImageFailed {
			add {
				if (events[ImageFailedEvent] == null)
					Events.AddHandler (this, "ImageFailed", image_failed);
				events.AddHandler (ImageFailedEvent, value);
			}
			remove {
				events.RemoveHandler (ImageFailedEvent, value);
				if (events[ImageFailedEvent] == null)
					Events.RemoveHandler (this, "ImageFailed", image_failed);
			}
		}

		public event ExceptionRoutedEventHandler ImageOpenFailed {
			add {
				if (events[ImageOpenFailedEvent] == null)
					Events.AddHandler (this, "ImageOpenFailed", image_open_failed);
				events.AddHandler (ImageOpenFailedEvent, value);
			}
			remove {
				if (events[ImageOpenFailedEvent] == null)
					Events.AddHandler (this, "ImageOpenFailed", image_open_failed);
				events.AddHandler (ImageOpenFailedEvent, value);
			}
		}

		public event RoutedEventHandler ImageOpenSucceeded {
			add {
				if (events[ImageOpenSucceededEvent] == null)
					Events.AddHandler (this, "ImageOpenSucceeded", image_open_succeeded);
				events.AddHandler (ImageOpenSucceededEvent, value);
			}
			remove {
				events.RemoveHandler (ImageOpenSucceededEvent, value);
				if (events[ImageOpenSucceededEvent] == null)
					Events.RemoveHandler (this, "ImageOpenSucceeded", image_open_succeeded);
			}
		}

		public event RoutedEventHandler MotionFinished {
			add {
				if (events[MotionFinishedEvent] == null)
					Events.AddHandler (this, "MotionFinished", motion_finished);
				events.AddHandler (MotionFinishedEvent, value);
			}
			remove {
				events.RemoveHandler (MotionFinishedEvent, value);
				if (events[MotionFinishedEvent] == null)
					Events.RemoveHandler (this, "MotionFinished", motion_finished);
			}
		}

		public event RoutedEventHandler ViewportChanged {
			add {
				if (events[ViewportChangedEvent] == null)
					Events.AddHandler (this, "ViewportChanged", viewport_changed);
				events.AddHandler (ViewportChangedEvent, value);
			}
			remove {
				events.RemoveHandler (ViewportChangedEvent, value);
				if (events[ViewportChangedEvent] == null)
					Events.RemoveHandler (this, "ViewportChanged", viewport_changed);
			}
		}

		static void image_failed_cb (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			((MultiScaleImage) Helper.GCHandleFromIntPtr (closure).Target).InvokeImageFailed ();
		}


		static void image_open_failed_cb (IntPtr target, IntPtr calldata, IntPtr closure) {
			((MultiScaleImage) Helper.GCHandleFromIntPtr (closure).Target).InvokeImageOpenFailed ();
		}

		static void image_open_succeeded_cb (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			((MultiScaleImage) Helper.GCHandleFromIntPtr (closure).Target).InvokeImageOpenSucceeded ();
		}

		static void motion_finished_cb (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			((MultiScaleImage) Helper.GCHandleFromIntPtr (closure).Target).InvokeMotionFinished ();
		}

		static void viewport_changed_cb (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			((MultiScaleImage) Helper.GCHandleFromIntPtr (closure).Target).InvokeViewportChanged ();
		}

		void InvokeImageFailed ()
		{
			EventHandler h = (EventHandler) events[ImageFailedEvent];
			if (h != null)
				h (this, EventArgs.Empty);
		}

		private void InvokeImageOpenFailed ()
		{
			EventHandler<ExceptionRoutedEventArgs> h = (EventHandler<ExceptionRoutedEventArgs>) events[ImageOpenFailedEvent];
			if (h != null)
				h (this, null);
		}

		void InvokeImageOpenSucceeded ()
		{
			EventHandler h = (EventHandler) events[ImageOpenSucceededEvent];
			if (h != null)
				h (this, EventArgs.Empty);
		}

		void InvokeMotionFinished ()
		{
			EventHandler h = (EventHandler) events[MotionFinishedEvent];
			if (h != null)
				h (this, EventArgs.Empty);
		}

		void InvokeViewportChanged ()
		{
			EventHandler h = (EventHandler) events[ViewportChangedEvent];
			if (h != null)
				h (this, EventArgs.Empty);
		}
	}
}
