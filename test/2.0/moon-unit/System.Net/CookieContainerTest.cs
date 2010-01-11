//
// Unit tests for System.Net.CookieContainer
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

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Net {

	[TestClass]
	public class CookieContainerTest {

		[TestMethod]
		public void Constants ()
		{
			Assert.AreEqual (4096, CookieContainer.DefaultCookieLengthLimit, "DefaultCookieLengthLimit");
			Assert.AreEqual (300, CookieContainer.DefaultCookieLimit, "DefaultCookieLimit");
			Assert.AreEqual (20, CookieContainer.DefaultPerDomainCookieLimit, "DefaultPerDomainCookieLimit");
		}

		[TestMethod]
		public void Defaults ()
		{
			CookieContainer cc = new CookieContainer ();
			Assert.AreEqual (300, cc.Capacity, "Capacity");
			Assert.AreEqual (0, cc.Count, "Count");
			Assert.AreEqual (4096, cc.MaxCookieSize, "MaxCookieSize");
			Assert.AreEqual (20, cc.PerDomainCapacity, "PerDomainCapacity");
		}

		[TestMethod]
		public void Add ()
		{
			Cookie c = new Cookie ();
			Uri uri = new Uri ("http://www.mono-project.com");
			CookieContainer cc = new CookieContainer ();
			Assert.Throws<ArgumentNullException> (delegate {
				cc.Add (null, c);
			}, "null,Cookie");
			Assert.Throws<ArgumentNullException> (delegate {
				cc.Add (uri, (Cookie)null);
			}, "Uri,null(Cookie)");
			Assert.Throws<CookieException> (delegate {
				cc.Add (uri, c);
			}, "unnamed cookie");

			c.Name = "chocolate";
			cc.Add (uri, c);
			Assert.AreEqual (1, cc.Count, "Count-a");

			cc.Add (uri, c); // twice
			Assert.AreEqual (1, cc.Count, "Count-b");

			Cookie c2 = new Cookie ("maple", "full");
			cc.Add (uri, c2);
			Assert.AreEqual (2, cc.Count, "Count-c");

			Cookie c3 = new Cookie ("name", "value", "/");
			cc.Add (uri, c3);
			Assert.AreEqual (3, cc.Count, "Count-d");

			Cookie c4 = new Cookie ("size", "large", "/", "www.mono-project.com");
			cc.Add (uri, c4);
			Assert.AreEqual (4, cc.Count, "Count-e");

			CookieCollection col = new CookieCollection ();
			Assert.Throws<ArgumentNullException> (delegate {
				cc.Add (null, col);
			}, "null,CookieCollection");
			Assert.Throws<ArgumentNullException> (delegate {
				cc.Add (uri, (CookieCollection) null);
			}, "Uri,null(CookieCollection)");
			
			cc.Add (uri, col);
			Assert.AreEqual (4, cc.Count, "Count-f");
			col.Add (c);
			Assert.AreEqual (1, col.Count, "CookieCllection.Count");
			Assert.AreEqual (4, cc.Count, "Count-g");
		}

		[TestMethod]
		public void GetCookieHeader ()
		{
			CookieContainer cc = new CookieContainer ();
			Assert.Throws<ArgumentNullException> (delegate {
				cc.GetCookieHeader (null);
			}, "null");

			Uri uri = new Uri ("http://www.mono-project.com");
			Assert.AreEqual (String.Empty, cc.GetCookieHeader (uri), "none");

			Cookie c1 = new Cookie ();
			c1.Name = "1";
			cc.Add (uri, c1);
			Assert.AreEqual ("1=", cc.GetCookieHeader (uri), "one");

			Cookie c2 = new Cookie ("2", "z");
			cc.Add (uri, c2);
			Assert.AreEqual ("1=; 2=z", cc.GetCookieHeader (uri), "two");

			Cookie c3 = new Cookie ("3", "xy", "/");
			cc.Add (uri, c3);
			Assert.AreEqual ("1=; 2=z; 3=xy", cc.GetCookieHeader (uri), "three");

			c3.HttpOnly = true;
			Assert.AreEqual ("1=; 2=z; 3=xy", cc.GetCookieHeader (uri), "HttpOnly");
		}

		[TestMethod]
		public void GetCookies ()
		{
			CookieContainer cc = new CookieContainer ();
			Assert.Throws<ArgumentNullException> (delegate {
				cc.GetCookies (null);
			}, "null");

			Uri uri = new Uri ("http://www.mono-project.com");
			Assert.AreEqual (0, cc.GetCookies (uri).Count, "0");

			Cookie c1 = new Cookie ();
			c1.Name = "chocolate";
			cc.Add (uri, c1);
			Assert.AreEqual (1, cc.GetCookies (uri).Count, "1");

			Cookie c2 = new Cookie ("maple", "full");
			cc.Add (uri, c2);
			Assert.AreEqual (2, cc.GetCookies (uri).Count, "2");

			Cookie c3 = new Cookie ("name", "value", "/");
			cc.Add (uri, c3);
			Assert.AreEqual (3, cc.GetCookies (uri).Count, "3");
		}
	}
}

