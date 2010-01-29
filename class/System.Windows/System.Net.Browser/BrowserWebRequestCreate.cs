//
// System.Net.Browser.BrowserHttpWebRequestCreator
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2009-2010 Novell, Inc (http://www.novell.com)
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

using System.Reflection;

namespace System.Net.Browser {

	internal class BrowserHttpWebRequestCreator : IWebRequestCreate {

		static Type browser_http_request;

		static WebRequest CreateBrowserHttpWebRequest (Uri uri)
		{
			var assembly = Assembly.Load ("System.Windows.Browser, Version=2.0.5.0, Culture=Neutral, PublicKeyToken=7cec85d7bea7798e");
			if (assembly == null)
				throw new InvalidOperationException ("Can not load System.Windows.Browser");
 
			browser_http_request = assembly.GetType ("System.Windows.Browser.Net.BrowserHttpWebRequest");
			if (browser_http_request == null)
				throw new InvalidOperationException ("Can not get BrowserHttpWebRequest");

			return (WebRequest) Activator.CreateInstance (browser_http_request, new object [] { uri });
		}

		public WebRequest Create (Uri uri)
		{
			// note: we can't decorate this method with [SecurityCritical] or [SecuritySafeCritical] since it implement
			// an interface and, even if the interface was decorated, application code could not be decorated
			return CreateBrowserHttpWebRequest (uri);
		}
	}
}

