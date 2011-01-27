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

		static bool? consented;

		private static IntPtr GetClipboard ()
		{
			Surface surface;
			IntPtr window;

			CheckAccess ();
			surface = Deployment.Current.Surface;
			window = NativeMethods.surface_get_normal_window (surface.Native);
			if (window == IntPtr.Zero)
				return IntPtr.Zero;
			return NativeMethods.moon_window_get_clipboard (window, (int)MoonClipboardType.MoonClipboard_Clipboard);
		}

		public static bool ContainsText ()
		{
			CheckAccess ();
			IntPtr clipboard = GetClipboard();
			return NativeMethods.moon_clipboard_contains_text (clipboard);
		}

		public static string GetText ()
		{
			CheckAccess ();
			CheckUserInitiated ();
			IntPtr clipboard = GetClipboard();
			return NativeMethods.moon_clipboard_get_text (clipboard) ?? String.Empty;
		}

		public static void SetText (string text)
		{
			if (text == null)
				throw new ArgumentNullException ("text");

			CheckAccess ();
			CheckUserInitiated ();
			IntPtr clipboard = GetClipboard();
			NativeMethods.moon_clipboard_set_text (clipboard, text);
		}

		private static void CheckUserInitiated ()
		{
			bool asked_user;

			if (!Helper.IsUserInitiated ())
				throw new SecurityException ("Clipboard access is not allowed");

			if (consented.HasValue && consented.Value)
				return;

			if (!NativeMethods.consent_prompt_user_for (0, out asked_user))
				throw new SecurityException ("Clipboard access is not allowed");

			if (asked_user)
				consented = true;
		}

		private static void CheckAccess ()
		{
			if (!Helper.CheckAccess ())
				throw new UnauthorizedAccessException ("Must be called from the main thread");
		}
	}
}

