//
// Unit tests for System.Net.Browser.WebRequestCreator
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2010 Novell, Inc (http://www.novell.com)
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
using System.Net;
using System.Net.Browser;

using Mono.Moonlight.UnitTesting;
using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Net.Browser {

	[TestClass]
	public class WebRequestCreatorTest {

		static Uri Uri = new Uri ("http://www.mono-project.com");

		[TestMethod]
		public void TwoDifferentStacks ()
		{
			Assert.IsNotNull (WebRequestCreator.BrowserHttp, "BrowserHttp");
			Assert.IsNotNull (WebRequestCreator.ClientHttp, "ClientHttp");
			Assert.AreNotSame (WebRequestCreator.BrowserHttp, WebRequestCreator.ClientHttp, "!same");
		}

		[TestMethod]
		public void BrowserHttp ()
		{
			IWebRequestCreate wrc = WebRequestCreator.BrowserHttp;
			Assert.AreEqual (wrc.GetType ().ToString (), "System.Net.Browser.BrowserHttpWebRequestCreator", "Type");

			WebRequest wr = wrc.Create (Uri);
			Assert.AreEqual (wr.GetType ().ToString (), "System.Net.Browser.BrowserHttpWebRequest", "Type");

			Assert.AreSame (wrc, wr.CreatorInstance, "CreatorInstance");

			// default so we don't register it - since we can do so only one time!
			// Assert.IsTrue (WebRequest.RegisterPrefix ("http://", WebRequestCreator.BrowserHttp), "RegisterPrefix");
		}

		[TestMethod]
		public void ClientHttp ()
		{
			IWebRequestCreate wrc = WebRequestCreator.ClientHttp;
			Assert.AreEqual (wrc.GetType ().ToString (), "System.Net.Browser.ClientHttpWebRequestCreator", "Type");

			WebRequest wr = wrc.Create (Uri);

			Assert.AreEqual (wr.GetType ().ToString (), "System.Net.Browser.ClientHttpWebRequest", "Type");
			Assert.AreSame (wrc, wr.CreatorInstance, "CreatorInstance");

			// we use 'https' instead of 'http' to avoid messing with other tests

			// registering 'https:' is not valid
			Assert.IsFalse (WebRequest.RegisterPrefix ("https:", WebRequestCreator.ClientHttp), "RegisterPrefix-https:");

			// registering 'https' is not good enough
			Assert.IsTrue (WebRequest.RegisterPrefix ("https", WebRequestCreator.ClientHttp), "RegisterPrefix-https");
			wr = WebRequest.Create ("https://www.mono-project.com");
			Assert.AreSame (WebRequestCreator.BrowserHttp, wr.CreatorInstance, "WebRequest.Create(string)-https");

			// you need to register 'https:/'
			Assert.IsTrue (WebRequest.RegisterPrefix ("https:/", WebRequestCreator.ClientHttp), "RegisterPrefix-https:/");
			wr = WebRequest.Create ("https://www.mono-project.com");
			Assert.AreSame (WebRequestCreator.ClientHttp, wr.CreatorInstance, "WebRequest.Create(string)-https");

			// or register 'https://'
			Assert.IsTrue (WebRequest.RegisterPrefix ("https://", WebRequestCreator.ClientHttp), "RegisterPrefix-https://");
			wr = WebRequest.Create ("https://www.mono-project.com");
			Assert.AreSame (WebRequestCreator.ClientHttp, wr.CreatorInstance, "WebRequest.Create(string)-https://");

			// we cannot register twice (e.g. go back to Browser)
			Assert.IsFalse (WebRequest.RegisterPrefix ("https://", WebRequestCreator.BrowserHttp), "RegisterPrefix-https://-2");
			wr = WebRequest.Create (new Uri ("https://www.mono-project.com"));
			Assert.AreSame (WebRequestCreator.ClientHttp, wr.CreatorInstance, "WebRequest.Create(Uri)-https://");

			// we cannot register twice (even with the same, Client, value)
			Assert.IsFalse (WebRequest.RegisterPrefix ("https://", WebRequestCreator.ClientHttp), "RegisterPrefix-https://-2");
		}
	}
}

