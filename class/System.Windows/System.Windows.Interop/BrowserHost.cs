//
// BrowserHost.cs
//
// Author:
//   Miguel de Icaza (miguel@novell.com)
//
// Copyright 2007 Novell, Inc.
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
using System.Runtime.InteropServices;
using Mono;

namespace System.Windows.Interop {

	public static class BrowserHost {
		static EventHandlerList events = new EventHandlerList ();
		static IntPtr surface = IntPtr.Zero;
		
		// Why are ActualHeight and ActualWidth uints?
		
		public static uint ActualHeight {
			get {
				if (PluginHost.Handle != IntPtr.Zero){
					int n = NativeMethods.plugin_instance_get_actual_height (PluginHost.Handle);
					return n >= 0 ? (uint) n : 0;
				} else
					return 768;
			}
		}

		public static uint ActualWidth {
			get {
				if (PluginHost.Handle != IntPtr.Zero){
					int n = NativeMethods.plugin_instance_get_actual_width (PluginHost.Handle);
					return n >= 0 ? (uint) n : 0;
				} else
					return 1024;
			}
		}

		public static bool IsFullScreen {
			get {
				if (PluginHost.Handle != IntPtr.Zero){
					return false;
				} else
					return false;
			}

			set {
				// not yet implemented
			}
		}
		
		public static event EventHandler FullScreenChange;
		
		static object ResizeEvent = new object ();
		
		internal static void InvokeResize ()
		{
			EventHandler h = (EventHandler) events[ResizeEvent];
			
			if (h != null)
				h (null, null);
		}
		
		public static event EventHandler Resize {
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
