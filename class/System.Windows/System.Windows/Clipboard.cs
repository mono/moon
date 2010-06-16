//
// Clipboard.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2010 Novell, Inc.
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

using System.Security;
using Mono;

namespace System.Windows {
	public static class Clipboard {
		// this needs to match the pal.h enum
		private enum MoonClipboardType {
			MoonClipboard_Clipboard,
			MoonClipboard_Primary
		}

		private static IntPtr GetClipboard ()
		{
			Surface surface;
			IntPtr window;
			surface = Deployment.Current.Surface;
			window = NativeMethods.surface_get_window (surface.Native);
			if (window == IntPtr.Zero)
				return IntPtr.Zero;
			return NativeMethods.moon_window_get_clipboard (window, (int)MoonClipboardType.MoonClipboard_Clipboard);
		}

		public static bool ContainsText ()
		{
			IntPtr clipboard = GetClipboard();
			return NativeMethods.moon_clipboard_contains_text (clipboard);
		}

		public static string GetText ()
		{
			CheckUserInitiated ();
			IntPtr clipboard = GetClipboard();
			return NativeMethods.moon_clipboard_get_text (clipboard);
		}

		public static void SetText (string text)
		{
			CheckUserInitiated ();
			IntPtr clipboard = GetClipboard();
			NativeMethods.moon_clipboard_set_text (clipboard, text, text.Length);
		}

		private static void CheckUserInitiated ()
		{
			if (!Helper.IsUserInitiated ())
				throw new SecurityException ("Clipboard access is not allowed");
		}
	}
}

