//
// WriteableBitmap.cs
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
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Resources;
using System.IO;
using System.Threading;
using System.Net;
using System.Runtime.InteropServices;
using Mono;

namespace System.Windows.Media.Imaging
{
	public sealed partial class WriteableBitmap : BitmapSource
	{
		int[] pixels;
		GCHandle pixels_handle;

		public WriteableBitmap (BitmapSource source) : base (NativeMethods.writeable_bitmap_new (), true)
		{
			if (source != null)
				NativeMethods.writeable_bitmap_initialize_from_bitmap_source (native, source.native);

			// FIXME: we need to test if modifications
			// done to the WB update the BitmapSource's
			// pixel data.  If they do, we can't use this
			// copying code and need to do more
			// complicated tricks to get the pixels array
			// to reflect the contents of the
			// bitmapsource.
			//
			// FIXME: we aren't taking row stride into
			// account here.  we're assuming the pixel
			// data of the source is 32 bits.
			pixels = new int[source.PixelWidth * source.PixelHeight];
			IntPtr bitmap_data = NativeMethods.bitmap_source_get_bitmap_data (source.native);
			if (bitmap_data != IntPtr.Zero)
				Marshal.Copy (bitmap_data, pixels, 0, pixels.Length);

			PinAndSetBitmapData ();
		}

		public WriteableBitmap (int width, int height) : base (NativeMethods.writeable_bitmap_new (), true)
		{
			PixelWidth = width;
			PixelHeight = height;

			pixels = new int[PixelWidth * PixelHeight];

			PinAndSetBitmapData ();
		}

		void PinAndSetBitmapData ()
		{
			pixels_handle = GCHandle.Alloc(pixels, GCHandleType.Pinned); // pin it so it doesn't move

			NativeMethods.bitmap_source_set_bitmap_data (native, pixels_handle.AddrOfPinnedObject(), false);
		}

		~WriteableBitmap ()
		{
			if (pixels_handle.IsAllocated) {
				pixels_handle.Free ();
			}
		}

		public int[] Pixels {
			get { return pixels; }
		}

		public void Render (UIElement element, Transform transform)
		{
			if (element == null)
				throw new NullReferenceException ("element cannot be null");
			if (transform == null)
				throw new NullReferenceException ("transform cannot be null");

			NativeMethods.writeable_bitmap_render (native, element.native, transform.native);
		}

		public void Invalidate ()
		{
			NativeMethods.bitmap_source_invalidate (native);
		}
	}

}
