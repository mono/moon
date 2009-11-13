//
// Unit tests for System.Xml.XmlReader
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
using System.Xml;
using System.Xml.Serialization;
using System.IO;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Net {

	[TestClass]
	public class XmlNamespaceManagerTest {

		string xmlstr = @"<Items xmlns:foo='bar'><foo:Item>test</foo:Item></Items>";
		string xmlstr2 = @"<foo:haha>namespace test</foo:haha>";
		string empty = "<empty/>";

		[TestMethod]
		public void Parse1 ()
		{
			XmlNamespaceManager nsMgr = new MyNS (XmlReader.Create (new StringReader (empty)));
			nsMgr.AddNamespace ("foo", "bar");
			XmlParserContext inputContext = new XmlParserContext (null, nsMgr, null, XmlSpace.None);

			XmlReader xr = XmlReader.Create (new StringReader (xmlstr), new XmlReaderSettings (), inputContext);
			while (xr.Read ()) {}
		}

		[TestMethod]
		[MoonlightBug]
		public void Parse2 ()
		{
			XmlNamespaceManager nsMgr = new MyNS (XmlReader.Create (new StringReader (empty)));
			nsMgr.AddNamespace ("foo", "bar");
			XmlParserContext inputContext = new XmlParserContext (null, nsMgr, null, XmlSpace.None);
			XmlReader xr = XmlReader.Create (new StringReader (xmlstr), new XmlReaderSettings (), inputContext);
			XmlNamespaceManager aMgr = new MyNS (xr);
			XmlParserContext inputContext2 = new XmlParserContext (null, aMgr, null, XmlSpace.None);

			XmlReader xr2 = XmlReader.Create (new StringReader (xmlstr2), new XmlReaderSettings (), inputContext2);
			while (xr2.Read ()) {}
		}

		[TestMethod]
		public void Parse3 ()
		{
			XmlNamespaceManager nsMgr = new MyNS (XmlReader.Create (new StringReader (empty)));
			XmlParserContext inputContext = new XmlParserContext (null, nsMgr, null, XmlSpace.None);
			XmlReader xr = XmlReader.Create (new StringReader (xmlstr), new XmlReaderSettings (), inputContext);
			XmlNamespaceManager aMgr = new MyNS (xr);
			XmlParserContext inputContext2 = new XmlParserContext(null, aMgr, null, XmlSpace.None);

			XmlReader xr2 = XmlReader.Create (new StringReader (xmlstr2), new XmlReaderSettings (), inputContext2);
			Assert.Throws<XmlException> (delegate {
				while (xr2.Read ()) {}
			}, "null");
		}
	}
	class MyNS : XmlNamespaceManager {
		private XmlReader xr;


		public MyNS (XmlReader xr)
			: base (xr.NameTable) {
			this.xr = xr;
		}

		public override string LookupNamespace (string prefix) {
			string str = base.LookupNamespace (prefix);
			if (!string.IsNullOrEmpty (str)) {
				return str;
			}
			if (xr != null)
				return xr.LookupNamespace (prefix);
			return String.Empty;
		}
	}
}

