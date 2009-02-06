//
// Content.cs
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

using Mono;
using System;
using System.Security;

namespace System.Windows.Interop {

	public sealed class Content {

		static IntPtr surface = IntPtr.Zero;

		public Content ()
		{
		}

		public double ActualHeight {
			[SecuritySafeCritical ()]
			get {
				if (PluginHost.Handle != IntPtr.Zero) {
					int n = NativeMethods.plugin_instance_get_actual_height (PluginHost.Handle);
					return n >= 0 ? n : 0;
				} else
					return 768;
			}
		}

		public double ActualWidth {
			[SecuritySafeCritical ()]
			get {
				if (PluginHost.Handle != IntPtr.Zero) {
					int n = NativeMethods.plugin_instance_get_actual_width (PluginHost.Handle);
					return n >= 0 ? n : 0;
				} else
					return 1024;
			}
		}

		public bool IsFullScreen {
			[SecuritySafeCritical ()]
			get {
				return NativeMethods.surface_get_full_screen (Deployment.Current.Surface.Native);
			}
			[MonoTODO ()]
			[SecuritySafeCritical ()]
			set {
				// FIXME: Stack walk here to ensure this is in response to a user generated event
				NativeMethods.surface_set_full_screen (Deployment.Current.Surface.Native, value);
			}
		}
		
		static EventHandlerList events = new EventHandlerList ();
		static object ResizeEvent = new object ();
		static object FullScreenChangeEvent = new object ();

		static internal void InvokeResize ()
		{
			EventHandler h = (EventHandler) events[ResizeEvent];
			
			if (h != null)
				h (null, null);
		}
		
		static internal void InvokeFullScreenChange ()
		{
			EventHandler h = (EventHandler) events[FullScreenChangeEvent];
			
			if (h != null)
				h (null, null);
		}
	
		public event EventHandler FullScreenChanged {
			add {
				events.AddHandler (FullScreenChangeEvent, value);
			}
			remove {
				events.RemoveHandler (FullScreenChangeEvent, value);
			}
		}
		
		public event EventHandler Resized {
			add {
				if (surface == IntPtr.Zero) {
					surface = NativeMethods.plugin_instance_get_surface (PluginHost.Handle);
					Events.InitSurface (surface);
				}
				events.AddHandler (ResizeEvent, value);
			}
			remove {
				events.RemoveHandler (ResizeEvent, value);
			}
		}
	}
}
