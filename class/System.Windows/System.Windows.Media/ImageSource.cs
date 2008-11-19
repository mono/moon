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

using System.IO;
using System.Windows;
using System.Security;
using System.Windows.Interop;
using Mono;

namespace System.Windows.Media {

	public abstract class ImageSource : DependencyObject {
		private StreamWrapper wrapper;
		private ManagedStreamCallbacks callbacks;

		internal ImageSource ()
		{
			// Can this ctor be deleted?
		}

		internal ImageSource (IntPtr native) : base (native)
		{
		}

		internal abstract Stream Stream { get; }
		internal abstract Uri Uri { get; }

		internal void SetElement (FrameworkElement element) {
			if (Stream != null) {
				wrapper = new StreamWrapper (Stream);
				callbacks = wrapper.GetCallbacks ();
				NativeMethods.image_set_stream_source (element.native, ref callbacks);
			}
			if (Uri != null) {
				NativeMethods.media_base_set_source (element.native, new Uri (PluginHost.RootUri, Uri).ToString ());
			}
		}
	}
}
