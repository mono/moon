//
// BitmapImage.cs
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

using System.Windows;
using System.Windows.Media;
using System.Windows.Resources;
using System.IO;
using System.Threading;
using System.Net;
using Mono;

namespace System.Windows.Media.Imaging
{
	public sealed partial class BitmapImage : BitmapSource
	{
		public BitmapImage (Uri uriSource) : base (NativeMethods.bitmap_image_new (), true)
		{
			UriSource = uriSource;
		}

		static object DownloadProgressEvent = new object ();
                static object ImageFailedEvent = new object ();
                static object ImageOpenedEvent = new object ();

		public event EventHandler<DownloadProgressEventArgs> DownloadProgress {
			add {
                                RegisterEvent (DownloadProgressEvent, "DownloadProgress", download_progress, value);
			}
			remove {
                                UnregisterEvent (DownloadProgressEvent, "DownloadProgress", download_progress, value);
			}
		}
		
		public event EventHandler<ExceptionRoutedEventArgs> ImageFailed {
			add {
                                RegisterEvent (ImageFailedEvent, "ImageFailed", image_failed, value);
			}
			remove {
                                UnregisterEvent (ImageFailedEvent, "ImageFailed", image_failed, value);
			}
		}

		public event EventHandler<RoutedEventArgs> ImageOpened {
			add {
                                RegisterEvent (ImageOpenedEvent, "ImageOpened", image_opened, value);
			}
			remove {
                                UnregisterEvent (ImageOpenedEvent, "ImageOpened", image_opened, value);
			}
		}

                static UnmanagedEventHandler download_progress = Events.CreateSafeHandler (download_progress_cb);
                static UnmanagedEventHandler image_failed = Events.CreateSafeHandler (image_failed_cb);
                static UnmanagedEventHandler image_opened = Events.CreateSafeHandler (image_opened_cb);

                private static void download_progress_cb (IntPtr target, IntPtr calldata, IntPtr closure) {
			BitmapImage b = (BitmapImage) NativeDependencyObjectHelper.FromIntPtr (closure);
                        b.RaiseDownloadProgress (new DownloadProgressEventArgs (calldata));
                }
                
		private static void image_failed_cb (IntPtr target, IntPtr calldata, IntPtr closure) {
			BitmapImage b = (BitmapImage) NativeDependencyObjectHelper.FromIntPtr (closure);
                        b.RaiseImageFailed (new ExceptionRoutedEventArgs (calldata));
                }
		
		private static void image_opened_cb (IntPtr target, IntPtr calldata, IntPtr closure) {
			BitmapImage b = (BitmapImage) NativeDependencyObjectHelper.FromIntPtr (closure);
                        b.RaiseImageOpened (new RoutedEventArgs (calldata, false));
                }

		private void RaiseDownloadProgress (DownloadProgressEventArgs args) {
                        EventHandler<DownloadProgressEventArgs> h = (EventHandler<DownloadProgressEventArgs>) EventList [DownloadProgressEvent];
			if (h != null)
				h (this, args);
		}
		
		private void RaiseImageFailed (ExceptionRoutedEventArgs args) {
                        EventHandler<ExceptionRoutedEventArgs> h = (EventHandler<ExceptionRoutedEventArgs>) EventList [ImageFailedEvent];
			if (h != null)
				h (this, args);
		}
		
		private void RaiseImageOpened (RoutedEventArgs args) {
                        EventHandler<RoutedEventArgs> h = (EventHandler<RoutedEventArgs>) EventList [ImageOpenedEvent];
			if (h != null)
				h (this, args);
		}
	}

}
