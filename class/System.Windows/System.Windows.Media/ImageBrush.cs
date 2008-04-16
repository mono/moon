//
// System.Windows.Media.ImageBrush class
//
// Authors:
//	Sebastien Pouliot  <sebastien@ximian.com>
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
using Mono;

namespace System.Windows.Media {

	public sealed class ImageBrush : TileBrush {

		public static readonly DependencyProperty DownloadProgressProperty =
			DependencyProperty.Lookup (Kind.IMAGEBRUSH, "DownloadProgress", typeof (double));
		public static readonly DependencyProperty ImageSourceProperty =
			DependencyProperty.Lookup (Kind.IMAGEBRUSH, "ImageSource", typeof (string));


		public ImageBrush () : base(NativeMethods.image_brush_new ())
		{
		}
		
		internal ImageBrush (IntPtr raw) : base (raw)
		{
		}


		public double DownloadProgress {
			get { return (double) GetValue (DownloadProgressProperty); }
			set { SetValue (DownloadProgressProperty, value); }
		}

		public Uri ImageSource {
			get { 
				// Uri is not a DependencyObject, we save it as a string
				string uri = (string) GetValue (ImageSourceProperty);
				if (uri == null || uri == string.Empty)
					return null;
				return new Uri (uri);
			}
			set { 
				string uri = value.OriginalString;
				SetValue (ImageSourceProperty, uri); 
			}
		}

		public void SetSource (DependencyObject Downloader, string PartName)
		{
			NativeMethods.image_brush_set_source (native, Downloader.native, PartName);
		}

		internal override Kind GetKind ()
		{
			return Kind.IMAGEBRUSH;
		}

		public event ErrorEventHandler ImageFailed;
	}
}
