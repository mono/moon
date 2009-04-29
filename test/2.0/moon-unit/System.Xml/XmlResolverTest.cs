//
// Unit tests for System.Xml.XmlResolver
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2009 Novell, Inc (http://www.novell.com)
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
using System.IO;
using System.Xml;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Net {

	[TestClass]
	public class XmlResolverTest {

		class ConcreteXmlResolver : XmlResolver {

			public override object GetEntity (Uri absoluteUri, string role, Type ofObjectToReturn)
			{
				throw new NotImplementedException ();
			}
		}

		[TestMethod]
		public void ResolveUri ()
		{
			ConcreteXmlResolver xr = new ConcreteXmlResolver ();
			Assert.Throws<ArgumentNullException> (delegate {
				xr.ResolveUri (null, null);
			}, "null,null");

			Uri uri = xr.ResolveUri (null, "/Moonlight");
			Assert.IsFalse (uri.IsAbsoluteUri, "null,string");
			Assert.AreEqual ("/Moonlight", uri.ToString (), "ToString");

			uri = new Uri ("http://www.mono-project.com");
			Uri u2 = xr.ResolveUri (uri, null);
			Assert.AreEqual (uri, u2, "Equals");
			Assert.IsTrue (Object.ReferenceEquals (uri, u2), "ReferenceEquals");

			u2 = xr.ResolveUri (uri, "/Moonlight");
			Assert.IsTrue (uri.IsAbsoluteUri, "abs,string");
			Assert.AreEqual ("http://www.mono-project.com/Moonlight", u2.ToString (), "ToString3");

			u2 = xr.ResolveUri (null, "http://www.mono-project.com");
			Assert.IsTrue (u2.IsAbsoluteUri, "null,absolute/http");
			Assert.AreEqual (uri, u2, "Equals-2");

			u2 = xr.ResolveUri (null, "https://www.mono-project.com");
			Assert.IsTrue (u2.IsAbsoluteUri, "null,absolute/https");

			u2 = xr.ResolveUri (null, "ftp://mono-project.com/download");
			Assert.IsTrue (u2.IsAbsoluteUri, "null,absolute/ftp");

			u2 = xr.ResolveUri (null, "file:///mystuff");
			Assert.IsTrue (u2.IsAbsoluteUri, "null,absolute/file");
		}

		[TestMethod]
		public void SupportsType ()
		{
			ConcreteXmlResolver xr = new ConcreteXmlResolver ();
			Assert.Throws<ArgumentNullException> (delegate {
				xr.SupportsType (null, typeof (Stream));
			}, "null,Stream");

			Uri uri = new Uri ("http://www.mono-project.com");
			Assert.IsTrue (xr.SupportsType (uri, null), "uri,null");
			Assert.IsTrue (xr.SupportsType (uri, typeof (Stream)), "uri,Stream");
			Assert.IsFalse (xr.SupportsType (uri, typeof (FileStream)), "uri,FileStream");
			Assert.IsFalse (xr.SupportsType (uri, typeof (object)), "uri,object");

			// parameter is named 'absoluteUri' but it does not matter a bit
			Uri rel_uri = new Uri ("/Moonlight", UriKind.Relative);
			Assert.IsTrue (xr.SupportsType (rel_uri, typeof (Stream)), "rel_uri,Stream");
		}
	}
}

