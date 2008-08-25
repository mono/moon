//
// System.Windows.Controls.Image
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2007 Novell, Inc (http://www.novell.com)
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
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.IO;
using Mono;

namespace System.Windows.Controls {
	public sealed partial class Image : FrameworkElement {
		// XXX this should be an ImageSource
		public static readonly DependencyProperty SourceProperty =
			DependencyProperty.Lookup (Kind.MEDIABASE, "Source", typeof (string));
		
		public static readonly DependencyProperty StretchProperty =
			DependencyProperty.Lookup (Kind.MEDIABASE, "Stretch", typeof (Stretch));
				
		public ImageSource Source {
			get { throw new NotImplementedException (); }
			set {
				if (value == null) {
					NativeMethods.media_base_set_source (this.native, null);
				} else {
					value.SetElement (this);
				}
			}
		}
		
		public Stretch Stretch {
			get { return (Stretch) GetValue (StretchProperty); }
			set { SetValue (StretchProperty, value); }
		}
		
		static object ImageFailedEvent = new object ();
		
		public event EventHandler<ExceptionRoutedEventArgs> ImageFailed {
			add {
				if (events[ImageFailedEvent] == null)
					Events.AddHandler (this, "ImageFailed", image_failed);
				events.AddHandler (ImageFailedEvent, value);
			}
			remove {
				if (events[ImageFailedEvent] == null)
					Events.AddHandler (this, "ImageFailed", image_failed);
				events.AddHandler (ImageFailedEvent, value);
			}
		}
		
		static UnmanagedEventHandler image_failed = new UnmanagedEventHandler (image_failed_cb);
		
		private static void image_failed_cb (IntPtr target, IntPtr calldata, IntPtr closure) {
			// XXX we need to marshal calldata to an ErrorEventArgs struct
			((Image) Helper.GCHandleFromIntPtr (closure).Target).InvokeImageFailed (/* XXX and pass it here*/);
		}
		
		private void InvokeImageFailed (/* XXX ErrorEventArgs args */)
		{
			EventHandler<ExceptionRoutedEventArgs> h = (EventHandler<ExceptionRoutedEventArgs>) events[ImageFailedEvent];
			if (h != null)
				h (this, null); // XXX pass args here
		}
	}
}
