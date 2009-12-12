//
// Settings.cs
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
using Mono;
using Mono.Xaml;

namespace System.Windows.Interop {
	public sealed class Settings
	{
		public Settings ()
		{
		}

		[MonoTODO ("this should enable the fps counter in the browser status bar")]
		public bool EnableFrameRateCounter {
			get { return false; }
			set {}
		}

		public bool EnableHTMLAccess {
			get {
				return NativeMethods.plugin_instance_get_enable_html_access (XamlLoader.PluginInDomain);
			}
		}

		[MonoTODO]
		public bool EnableRedrawRegions {
			get { return false; }
			set {}
		}

		[MonoTODO]
		public bool EnableGPUAcceleration {
			get { return false; }
		}

		[MonoTODO]
		public bool EnableCacheVisualization {
			get { return false; }
			set { }
		}

		public int MaxFrameRate {
			get {
				return NativeMethods.time_manager_get_maximum_refresh_rate (NativeMethods.surface_get_time_manager (Deployment.Current.Surface.Native));
			}
			set {
				// note: does not throw on invalid (< 1) values - but does not change the value
				NativeMethods.time_manager_set_maximum_refresh_rate (NativeMethods.surface_get_time_manager (Deployment.Current.Surface.Native), value);
			}
		}

		public bool Windowless {
			get {
				return NativeMethods.plugin_instance_get_windowless (XamlLoader.PluginInDomain);
			}
		}
	}
}
