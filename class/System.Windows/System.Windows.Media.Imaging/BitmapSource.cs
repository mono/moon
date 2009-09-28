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
	public partial class BitmapSource : ImageSource
	{
		public void SetSource (Stream streamSource)
		{
			if (streamSource == null)
				NativeMethods.bitmap_source_set_bitmap_data (native, IntPtr.Zero, true);
			else {
				NativeMethods.bitmap_image_pixbuf_write (native, Helper.StreamToIntPtr (streamSource), 0, (int) streamSource.Length);
				NativeMethods.bitmap_image_pixmap_complete (native);
			}
		}
                
	}

}
