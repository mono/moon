//
// SilverlightHost.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2008, 2009 Novell, Inc.
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
using System.Runtime.InteropServices;
using Mono;

namespace System.Windows.Interop {
	public class SilverlightHost {
		Content content;
		Settings settings;
		Uri source_uri;

		public SilverlightHost ()
		{
			NavigationState = "";
		}

		public bool IsVersionSupported (string versionStr)
		{
			// no null check so we throw an NRE, just like Silverlight, for a null versionStr
			if (versionStr.Length == 0)
				return false;
			return NativeMethods.surface_is_version_supported (versionStr);
		}

		public Color Background {
			get {
				IntPtr clr = NativeMethods.surface_get_background_color (Deployment.Current.Surface.Native);
				
				if (clr == IntPtr.Zero)
					return new Color ();
				
				unsafe {
					return ((UnmanagedColor *) clr)->ToColor ();
				}
			}
		}

		public Content Content {
			get { return content ?? (content = new Content ()); }
		}

		public bool IsLoaded {
			get { 
				return NativeMethods.surface_is_loaded (Deployment.Current.Surface.Native);
			}
		}

		public Settings Settings {
			get { return settings ?? (settings = new Settings ()); }
		}

		public Uri Source {
			get {
				if (source_uri == null) {
					// note: this must return the original URI (i.e. before any redirection)
					string source = NativeMethods.plugin_instance_get_source_original (PluginHost.Handle);
					source_uri = new Uri (source, UriKind.RelativeOrAbsolute);
					// the source is often relative (but can be absolute for cross-domain applications)
					if (!source_uri.IsAbsoluteUri) {
						string location = NativeMethods.plugin_instance_get_source_location_original (PluginHost.Handle);
						source_uri = new Uri (new Uri (location), source_uri);
					}
				}
				return source_uri;
			}
		}


		public string NavigationState { get; set; }

		public event EventHandler<NavigationStateChangedEventArgs> NavigationStateChanged;
	}
}
