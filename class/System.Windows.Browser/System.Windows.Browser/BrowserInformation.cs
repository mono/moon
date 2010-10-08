//
// System.Windows.Browser.BrowserInformation class
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2007, 2009-2010 Novell, Inc (http://www.novell.com)
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

namespace System.Windows.Browser {

	public sealed class BrowserInformation {

		ScriptObject navigator;
		Version version;
		string user_agent, product_name, product_version;

		public Version BrowserVersion {
			get {
				if (version == null)
					version = GetVersion ();
				return version;
			}
		}

		public bool CookiesEnabled {
			get { return (bool) navigator.GetProperty ("cookieEnabled"); }
		}

		public string Name {
			get { return (string) navigator.GetProperty ("appName"); }
		}

		public string Platform {
			get { return (string) navigator.GetProperty ("platform"); }
		}

		public string ProductName {
			get {
				if (product_name == null)
					GetProduct ();
				return product_name;
			}
		}

		public string ProductVersion {
			get {
				if (product_version == null)
					GetProduct ();
				return product_version;
			}
		}

		public string UserAgent {
			get {
				if (user_agent == null)
					user_agent = (string) navigator.GetProperty ("userAgent");
				return user_agent;
			}
		}

		internal BrowserInformation (HtmlWindow window)
		{
			navigator = window.GetProperty ("navigator") as ScriptObject;
		}

		Version GetVersion ()
		{
			try {
				string appVersion = (string) navigator.GetProperty ("appVersion");
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

		// according to documentation ProductName and ProductVersion properties are extracted from userAgent
		// cover UA like:
		//	Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9.1.9) Gecko/20100317 SUSE/3.5.9-0.1.1 Firefox/3.5.9
		//	Mozilla/5.0 (X11; U; Linux x86_64; en-US) AppleWebKit/533.3 (KHTML, like Gecko) Chrome/5.0.353.0 Safari/533.3
		//	Mozilla/5.0 (X11; U; Linux i686; en; rv:1.9) Gecko/2008062113 Iceweasel/3.0

		bool CheckFor (string browser)
		{
			int pos = UserAgent.IndexOf (browser + "/");
			if (pos == -1)
				return false;

			pos += browser.Length + 1;
			product_name = browser;
			int p2 = UserAgent.IndexOf (" ", pos);
			product_version = (p2 == -1) ? UserAgent.Substring (pos) : UserAgent.Substring (pos, p2 - pos);
			return true;
		}

		void GetProduct ()
		{
			if (!CheckFor ("Firefox")) {
				if (!CheckFor ("Chrome")) {
					CheckFor ("Iceweasel");
				}
			}
		}
	}
}
