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

		public bool EnableFrameRateCounter {
			get {
				return NativeMethods.plugin_instance_get_enable_frame_rate_counter (XamlLoader.PluginInDomain);
			}
			set {
				NativeMethods.plugin_instance_set_enable_frame_rate_counter (XamlLoader.PluginInDomain, value);
			}
		}

		public bool EnableHTMLAccess {
			get {
#if NET_2_1_LAUNCHER
				return true;
#else
			    	return NativeMethods.plugin_instance_get_enable_html_access (XamlLoader.PluginInDomain);
#endif
			}
		}

		public bool EnableRedrawRegions {
			get {
				return NativeMethods.plugin_instance_get_enable_redraw_regions (XamlLoader.PluginInDomain);
			}
			set {
				NativeMethods.plugin_instance_set_enable_redraw_regions (XamlLoader.PluginInDomain, value);
			}
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
#if NET_2_1_LAUNCHER
				return 60;
#else
				return NativeMethods.plugin_instance_get_max_frame_rate (XamlLoader.PluginInDomain);
#endif
			}
			set {
				// note: does not throw on invalid (< 1) values - but does not change the value
#if !NET_2_1_LAUNCHER
				NativeMethods.plugin_instance_set_max_frame_rate (XamlLoader.PluginInDomain, value);
#endif
			}
		}

		public bool Windowless {
			get {
				return NativeMethods.plugin_instance_get_windowless (XamlLoader.PluginInDomain);
			}
		}

		[MonoTODO]
		public bool EnableAutoZoom {
			get {
				Console.WriteLine ("System.Windows.Interop.Settings.get_EnableAutoZoom: NIEX");
				return false;
			}
			set {
				Console.WriteLine ("System.Windows.Interop.Settings.set_EnableAutoZoom: NIEX");
			}
		}

		internal static bool EnableNavigation {
			get {
				return NativeMethods.plugin_instance_get_enable_navigation (XamlLoader.PluginInDomain);
			}
		}
	}
}
