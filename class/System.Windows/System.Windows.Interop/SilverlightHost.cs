//
// SilverlightHost.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2008-2011 Novell, Inc.
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
using System.Collections.Generic;
using System.IO;
using Mono;
#if !BOOTSTRAP
using System.Windows.Browser;
#endif

namespace System.Windows.Interop {
	public class SilverlightHost {
		Content content;
		Settings settings;
		Uri source_uri;
		private Dictionary<string,string> init_params;

		public SilverlightHost ()
		{
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
					if (PluginHost.Handle == IntPtr.Zero) {
						source_uri = UriHelper.FromNativeUri (NativeMethods.surface_get_source_location (Deployment.Current.Surface.Native));
					}
					else {
						// note: this must return the original URI (i.e. before any redirection)
						string source = NativeMethods.plugin_instance_get_source_original (PluginHost.Handle);
						source_uri = new Uri (source, UriKind.RelativeOrAbsolute);
						// the source is often relative (but can be absolute for cross-domain applications)
						if (!source_uri.IsAbsoluteUri) {
							string location = NativeMethods.plugin_instance_get_source_location_original (PluginHost.Handle);
							source_uri = new Uri (new Uri (location), source_uri);
						}
					}
				}
				return source_uri;
			}
		}

		public IDictionary<string,string> InitParams {
			get {
				if (init_params != null)
					return init_params;

				init_params = new Dictionary<string,string> ();

#if !NET_2_1_LAUNCHER
				if (PluginHost.Handle != IntPtr.Zero) {
#endif
					char [] param_separator = new char [] { ',' };

#if NET_2_1_LAUNCHER
					// FIXME android specific
					var paramFile = "/data/local/tmp/AdaptiveStreaming.xap/initParams";
					string param_string = "";
					if (File.Exists (paramFile)) {
						StreamReader sr = File.OpenText ("/data/local/tmp/AdaptiveStreaming.xap/initParams");
//						StreamReader sr = new StreamReader (Application.GetResourceStream(new Uri("initParams")).Stream);
						param_string = sr.ReadToEnd();
					}
					Console.WriteLine ("param_string = {0}", param_string);
#else
					string param_string = NativeMethods.plugin_instance_get_init_params (PluginHost.Handle);
#endif
					// Console.WriteLine ("params = {0}", param_string);
					if (!String.IsNullOrEmpty (param_string)) {
						foreach (string val in param_string.Split (param_separator)) {
							int split = val.IndexOf ('=');
							if (split >= 0) {
								string k = val.Substring (0, split).Trim ();
								string v = val.Substring (split + 1).Trim ();
								if (k.Length > 0)
									init_params [k] = v;
							} else {
								string s = val.Trim ();
								if (!String.IsNullOrEmpty (s))
									init_params [s] = String.Empty;
							}
						}
					}
#if !NET_2_1_LAUNCHER
				}
#endif

				return init_params;
			}
		}

		private string oob_state = String.Empty;
		private event EventHandler<NavigationStateChangedEventArgs> navigation_state_changed;
		private object locker = new object ();

		public string NavigationState {
			get {
				if (!Helper.CheckAccess ())
					throw new UnauthorizedAccessException ();

				if (Application.Current.IsRunningOutOfBrowser) {
					return oob_state;
#if !BOOTSTRAP
				} else if (Settings.EnableNavigation) {
					// the default value for 'NavigationState' comes from the HTML page hosting the plugin
					return HtmlPage.NavigationState;
#endif
				} else {
					throw new InvalidOperationException ("enabledNavigation is set to false");
				}
			}
			set {
				if (value == null)
					throw new ArgumentNullException ();

				if (!Helper.CheckAccess ())
					throw new UnauthorizedAccessException ();

				// note: there's no event for re-assigning the same state
				string current = NavigationState;
				if (current == value)
					return;

				if (Application.Current.IsRunningOutOfBrowser)
					oob_state = value;
#if !BOOTSTRAP
				else
					HtmlPage.NavigationState = value;

				// note: there's no event if HtmlPage.Window.CurrentBookmark is used to change NavigationState
				var changed = navigation_state_changed;
				if (changed != null)
					changed (this, new NavigationStateChangedEventArgs (current, value));
			}
		}

		public event EventHandler<NavigationStateChangedEventArgs> NavigationStateChanged {
			add {
				if (!Helper.CheckAccess ())
					throw new UnauthorizedAccessException ();
				if (!Application.Current.IsRunningOutOfBrowser)
					HtmlPage.EnsureHistoryIframePresence ();

				lock (locker) {
					navigation_state_changed += value;
				}
			}
			remove {
				lock (locker) {
					navigation_state_changed -= value;
				}
#endif
			}
		}
	}
}
