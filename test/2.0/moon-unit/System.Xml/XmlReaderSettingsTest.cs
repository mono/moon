//
// Unit tests for System.Xml.XmlReaderSettings
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

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Net {

	[TestClass]
	public class XmlReaderSettingsTest {

		[TestMethod]
		public void Defaults ()
		{
			XmlReaderSettings xrs = new XmlReaderSettings ();
			Assert.IsTrue (xrs.CheckCharacters, "CheckCharacters");
			Assert.IsFalse (xrs.CloseInput, "CloseInput");
			Assert.AreEqual (ConformanceLevel.Document, xrs.ConformanceLevel, "ConformanceLevel");
			Assert.AreEqual (DtdProcessing.Prohibit, xrs.DtdProcessing, "DtdProcessing");
			Assert.IsFalse (xrs.IgnoreComments, "IgnoreComments");
			Assert.IsFalse (xrs.IgnoreProcessingInstructions, "IgnoreProcessingInstructions");
			Assert.IsFalse (xrs.IgnoreWhitespace, "IgnoreWhitespace");
			Assert.AreEqual (0, xrs.LineNumberOffset, "LineNumberOffset");
			Assert.AreEqual (0, xrs.LinePositionOffset, "LinePositionOffset");
			Assert.AreEqual (0, xrs.MaxCharactersFromEntities, "MaxCharactersFromEntities");
			Assert.AreEqual (0, xrs.MaxCharactersInDocument, "MaxCharactersInDocument");
			Assert.IsNull (xrs.NameTable, "NameTable");
		}

		class ConcreteXmlResolver : XmlResolver {

			public override object GetEntity (Uri absoluteUri, string role, Type ofObjectToReturn)
			{
				throw new NotImplementedException ();
			}
		}

		[TestMethod]
		public void XmlResolver ()
		{
			XmlReaderSettings xrs = new XmlReaderSettings ();
			// setter-only property
			xrs.XmlResolver = new ConcreteXmlResolver ();
			xrs.XmlResolver = null;
		}
	}
}

