//
// SilverlightHost.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
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
using System.Windows.Media;
using System.Security;
using System.Runtime.InteropServices;
using Mono;

namespace System.Windows.Interop {
	public class SilverlightHost {
		private Content content;

		public SilverlightHost ()
		{}

		[SecuritySafeCritical ()]
		public bool IsVersionSupported (string versionStr)
		{
			throw new NotImplementedException ();
		}

		public Color Background {
			[SecuritySafeCritical ()]
			get { throw new NotImplementedException (); }
		}

		public Content Content {
			get { return content ?? (content = new Content ()); }
		}

		public bool IsLoaded {
			[SecuritySafeCritical ()]
			get { throw new NotImplementedException (); }
		}

		public Settings Settings {
			get { throw new NotImplementedException (); }
		}

		public Uri Source {
			[SecuritySafeCritical ()]
			get {
				if (PluginHost.Handle == IntPtr.Zero)
					return null;

				IntPtr raw = NativeMethods.plugin_instance_get_source_location (PluginHost.Handle);
				if (raw == IntPtr.Zero)
					return null;

				return new Uri (Marshal.PtrToStringAnsi (raw), UriKind.RelativeOrAbsolute);
			}
		}
	}
}
