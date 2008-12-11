//
// Settings.cs
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
using System.Security;
using Mono;

namespace System.Windows.Interop {
	public sealed class Settings
	{
		public Settings ()
		{
		}

		//FIXME: this should enable the fps counter in the browser status bar
		public bool EnableFrameRateCounter
		{
			[SecuritySafeCritical]
			get { return false; }
			[SecuritySafeCritical]
			set {}
		}

		//FIXME
		public bool EnableHTMLAccess {
			[SecuritySafeCritical]
			get { return false ;}
		}

		//FIXME
		public bool EnableRedrawRegions {
			[SecuritySafeCritical]
			get { return false; }
			[SecuritySafeCritical]
			set {}
		}
		
		public int MaxFrameRate {
			[SecuritySafeCritical]
			get {
				return NativeMethods.time_manager_get_maximum_refresh_rate (NativeMethods.surface_get_time_manager (Application.s_surface));
			}
			[SecuritySafeCritical]
			set {
				NativeMethods.time_manager_set_maximum_refresh_rate (NativeMethods.surface_get_time_manager (Application.s_surface), value);
			}
		}

		//FIXME
		public bool Windowless {
			[SecuritySafeCritical]
			get { return false; }
		}
	}
}
