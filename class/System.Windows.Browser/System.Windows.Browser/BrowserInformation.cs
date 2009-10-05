//
// System.Windows.Browser.BrowserInformation class
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2007, 2009 Novell, Inc (http://www.novell.com)
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

namespace System.Windows.Browser {

	public sealed class BrowserInformation {

		HtmlElement navigator;
		Version version;

		public Version BrowserVersion {
			get {
				if (version == null)
					version = GetVersion ();
				return version;
			}
		}

		public bool CookiesEnabled {
			get { return GetNavigatorProperty<bool> ("cookieEnabled"); }
		}

		public string Name {
			get { return GetNavigatorProperty<string> ("appName"); }
		}

		public string Platform {
			get { return GetNavigatorProperty<string> ("platform"); }
		}

		public string UserAgent {
			get { return GetNavigatorProperty<string> ("userAgent"); }
		}

		internal BrowserInformation (HtmlWindow window)
		{
			navigator = window.GetPropertyInternal<HtmlElement> ("navigator");
		}

		Version GetVersion ()
		{
			try {
				var appVersion = GetNavigatorProperty<string> ("appVersion");
				int position = appVersion.IndexOf (' ');
				if (position == -1)
					return new Version (appVersion);

				return new Version (appVersion.Substring (0, position));
			}
			catch (ArgumentException) {
				// don't throw an exception for weird/bad ua strings
				return new Version ();
			}
		}

		T GetNavigatorProperty<T> (string name)
		{
			return navigator.GetPropertyInternal<T> (name);
		}
	}
}
