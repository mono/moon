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
	public partial class WriteableBitmap : BitmapSource
	{
		IntPtr buffer;
		bool rendered;

		public WriteableBitmap (BitmapSource source) : base (NativeMethods.writeable_bitmap_new (), true)
		{
			rendered = true;
			if (source != null)
				NativeMethods.writeable_bitmap_initialize_from_bitmap_source (native, source.native);
		}

		public WriteableBitmap (int width, int height, PixelFormat format) : base (NativeMethods.writeable_bitmap_new (), true)
		{
			PixelWidth = width;
			PixelHeight = height;
			PixelFormat = format;
			rendered = false;

			checked {
				buffer = Marshal.AllocHGlobal (width * height * 4);
				NativeMethods.bitmap_source_set_bitmap_data (native, buffer);
			}
		}

		public int this[int index] {
			get {
				if (rendered)
					throw new NullReferenceException ();

				return Marshal.ReadInt32 (buffer, index*4);
			}
			set {
				if (index > PixelWidth*PixelHeight || index < 0)
					throw new ArgumentOutOfRangeException ("index must lie withing the boundaries of the bitmap");
				checked {
					Marshal.WriteInt32 (buffer, index*4, value);
				}
			}
		}
		
		public void Render (UIElement element, Transform transform) {
			Lock ();

			if (element == null)
				throw new NullReferenceException ("element cannot be null");
			if (transform == null)
				throw new NullReferenceException ("transform cannot be null");

			rendered = true;
			NativeMethods.writeable_bitmap_render (native, element.native, transform.native);

			Unlock ();
		}

		public void Invalidate () {
			NativeMethods.bitmap_source_invalidate (native);
		}

		public void Lock () {
			NativeMethods.writeable_bitmap_lock (native);
		}

		public void Unlock () {
			NativeMethods.writeable_bitmap_unlock (native);
		}
	}

}
