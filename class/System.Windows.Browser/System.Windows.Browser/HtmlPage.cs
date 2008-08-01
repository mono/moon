//
// System.Windows.Browser.HtmlPage class
//
// Authors:
//	Sebastien Pouliot  <sebastien@ximian.com>
//
// Copyright (C) 2007 Novell, Inc (http://www.novell.com)
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

using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Security;

namespace System.Windows.Browser{

	public class HtmlPage : HtmlObject {

		private static BrowserInformation browser_info;

		public HtmlPage ()
		{
		}

		public static BrowserInformation BrowserInformation {
#if NET_2_1
		[SecuritySafeCritical ()]
#endif
			get {
				if (browser_info == null)
					browser_info = new BrowserInformation ();
				return browser_info;
			}
		}

		public static bool IsEnabled {		
#if NET_2_1
		[SecuritySafeCritical ()]
#endif
			get { throw new System.NotImplementedException (); }
		}
		
		public static HtmlDocument Document {
#if NET_2_1
		[SecuritySafeCritical ()]
#endif
			get {
				return new HtmlDocument (GetPropertyInternal<IntPtr> (IntPtr.Zero, "document"));
			}
		}

		public static IDictionary<string,string> QueryString {
			get {
				// document.location.search
				IntPtr loc = GetPropertyInternal<IntPtr> (Document.Handle, "location");
				string search = GetPropertyInternal<string> (loc, "search");

				Dictionary<string, string> res = new Dictionary<string, string> ();

				int li = 1;
				string name = null;
				bool in_name = true;
				for (int i = 1; i < search.Length; i++) {
					if (in_name) {
						if (search [i] == '=') {
							name = search.Substring (li, i - li);
							in_name = false;
							li = i + 1;
						}
					} else {
						if (search [i] == '&') {
							res.Add (name, search.Substring (li, i - li));
							in_name = true;
							li = i + 1;
						}
					}
				}

				if (name != null && li < search.Length)
					res.Add (name, search.Substring (li, search.Length - li));

				return res;
			}
		}
		
		public static void UnregisterCreateableType (string scriptAlias)
		{
			throw new System.NotImplementedException ();
		}
		
		public static void RegisterScriptableObject (string scriptKey, object instance)
		{
			throw new System.NotImplementedException ();
		}
		
		public static void RegisterCreateableType (string scriptAlias, Type type)
		{
			throw new System.NotImplementedException ();
		}

		public static HtmlWindow Window {
#if NET_2_1
		[SecuritySafeCritical ()]
#endif
			get {
				return new HtmlWindow (GetPropertyInternal<IntPtr> (IntPtr.Zero, "window"));
			}
		}

		public static HtmlElement Plugin {
#if NET_2_1
		[SecuritySafeCritical ()]
#endif
			get { throw new System.NotImplementedException (); }
		}
	}
}
