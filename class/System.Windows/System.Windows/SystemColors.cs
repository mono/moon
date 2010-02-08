//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2009 Novell, Inc.
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
using System.Windows;
using System.Windows.Media;

namespace System.Windows {
	public static class SystemColors {
		enum SystemColor {
			ActiveBorderColor,
			ActiveCaptionColor,
			ActiveCaptionTextColor,
			AppWorkspaceColor,
			ControlColor,
			ControlDarkColor,
			ControlDarkDarkColor,
			ControlLightColor,
			ControlLightLightColor,
			ControlTextColor,
			DesktopColor,
			GrayTextColor,
			HighlightColor,
			HighlightTextColor,
			InactiveBorderColor,
			InactiveCaptionColor,
			InactiveCaptionTextColor,
			InfoColor,
			InfoTextColor,
			MenuColor,
			MenuTextColor,
			ScrollBarColor,
			WindowColor,
			WindowFrameColor,
			WindowTextColor
		}
		
		static Color GetSystemColor (SystemColor which)
		{
			IntPtr clr = NativeMethods.surface_get_system_color (Deployment.Current.Surface.Native, (int) which);
			
			if (clr == IntPtr.Zero)
				return new Color ();
			
			unsafe {
				return ((UnmanagedColor *) clr)->ToColor ();
			}
		}
		
		// FIXME: should these cache?
		public static Color ActiveBorderColor {
			get { return GetSystemColor (SystemColor.ActiveBorderColor); }
		}
		
		public static Color ActiveCaptionColor {
			get { return GetSystemColor (SystemColor.ActiveCaptionColor); }
		}
		
		public static Color ActiveCaptionTextColor {
			get { return GetSystemColor (SystemColor.ActiveCaptionTextColor); }
		}
		
		public static Color AppWorkspaceColor {
			get { return GetSystemColor (SystemColor.AppWorkspaceColor); }
		}
		
		public static Color ControlColor {
			get { return GetSystemColor (SystemColor.ControlColor); }
		}
		
		public static Color ControlDarkColor {
			get { return GetSystemColor (SystemColor.ControlDarkColor); }
		}
		
		public static Color ControlDarkDarkColor {
			get { return GetSystemColor (SystemColor.ControlDarkDarkColor); }
		}
		
		public static Color ControlLightColor {
			get { return GetSystemColor (SystemColor.ControlLightColor); }
		}
		
		public static Color ControlLightLightColor {
			get { return GetSystemColor (SystemColor.ControlLightLightColor); }
		}
		
		public static Color ControlTextColor {
			get { return GetSystemColor (SystemColor.ControlTextColor); }
		}
		
		public static Color DesktopColor {
			get { return GetSystemColor (SystemColor.DesktopColor); }
		}
		
		public static Color GrayTextColor {
			get { return GetSystemColor (SystemColor.GrayTextColor); }
		}
		
		public static Color HighlightColor {
			get { return GetSystemColor (SystemColor.HighlightColor); }
		}
		
		public static Color HighlightTextColor {
			get { return GetSystemColor (SystemColor.HighlightTextColor); }
		}
		
		public static Color InactiveBorderColor {
			get { return GetSystemColor (SystemColor.InactiveBorderColor); }
		}
		
		public static Color InactiveCaptionColor {
			get { return GetSystemColor (SystemColor.InactiveCaptionColor); }
		}
		
		public static Color InactiveCaptionTextColor {
			get { return GetSystemColor (SystemColor.InactiveCaptionTextColor); }
		}
		
		public static Color InfoColor {
			get { return GetSystemColor (SystemColor.InfoColor); }
		}
		
		public static Color InfoTextColor {
			get { return GetSystemColor (SystemColor.InfoTextColor); }
		}
		
		public static Color MenuColor {
			get { return GetSystemColor (SystemColor.MenuColor); }
		}
		
		public static Color MenuTextColor {
			get { return GetSystemColor (SystemColor.MenuTextColor); }
		}
		
		public static Color ScrollBarColor {
			get { return GetSystemColor (SystemColor.ScrollBarColor); }
		}
		
		public static Color WindowColor {
			get { return GetSystemColor (SystemColor.WindowColor); }
		}
		
		public static Color WindowFrameColor {
			get { return GetSystemColor (SystemColor.WindowFrameColor); }
		}
		
		public static Color WindowTextColor {
			get { return GetSystemColor (SystemColor.WindowTextColor); }
		}
	}
}
