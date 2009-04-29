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

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Net {

	[TestClass]
	public class XmlReaderTest {

		void CheckAppManifest (XmlReader xr)
		{
			Assert.AreEqual ("AppManifest.xaml", xr.BaseURI, "BaseURI");
			Assert.IsTrue (xr.CanReadBinaryContent, "CanReadBinaryContent");
			Assert.IsTrue (xr.CanReadValueChunk, "CanReadValueChunk");
			Assert.IsTrue (xr.CanResolveEntity, "CanResolveEntity");
			Assert.AreEqual (0, xr.Depth, "Depth");
			Assert.IsFalse (xr.EOF, "EOF");
			Assert.IsFalse (xr.HasAttributes, "HasAttributes");
			Assert.IsFalse (xr.HasValue, "HasValue");
			Assert.IsFalse (xr.IsDefault, "IsDefault");
			Assert.IsFalse (xr.IsEmptyElement, "IsEmptyElement");
			Assert.AreEqual (String.Empty, xr.LocalName, "LocalName");
			Assert.AreEqual (String.Empty, xr.NamespaceURI, "NamespaceURI");
			Assert.IsNotNull (xr.NameTable, "NameTable");
			Assert.AreEqual (XmlNodeType.None, xr.NodeType, "NodeType");
			Assert.AreEqual (String.Empty, xr.Prefix, "Prefix");
			Assert.AreEqual (ReadState.Initial, xr.ReadState, "ReadState");
			Assert.IsNotNull (xr.Settings, "Settings");
			Assert.AreEqual (String.Empty, xr.Value, "Value");
			Assert.AreEqual (typeof (string), xr.ValueType, "ValueType");
			Assert.AreEqual (String.Empty, xr.XmlLang, "XmlLang");
			Assert.AreEqual (XmlSpace.None, xr.XmlSpace, "XmlSpace");
		}

		[TestMethod]
		public void Create_String ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				XmlReader.Create ((string) null);
			}, "null");
			Assert.Throws<ArgumentException> (delegate {
				XmlReader.Create (String.Empty);
			}, "empty");
			Assert.Throws<XmlException> (delegate {
				XmlReader.Create ("does-not-exists");
			}, "does-not-exists");

			XmlReader xr = XmlReader.Create ("AppManifest.xaml");
			CheckAppManifest (xr);
		}

		[TestMethod]
		public void Create_String_XmlReaderSettings ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				XmlReader.Create ((string) null, null);
			}, "null");
			Assert.Throws<ArgumentException> (delegate {
				XmlReader.Create (String.Empty, null);
			}, "empty");
			Assert.Throws<XmlException> (delegate {
				XmlReader.Create ("does-not-exists", null);
			}, "does-not-exists");

			XmlReader xr = XmlReader.Create ("AppManifest.xaml", null);
			CheckAppManifest (xr);
		}

		[TestMethod]
		public void Create_String_XmlReaderSettings_XmlParserContext ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				XmlReader.Create ((string) null, null, null);
			}, "null");
			Assert.Throws<ArgumentException> (delegate {
				XmlReader.Create (String.Empty, null, null);
			}, "empty");
			Assert.Throws<XmlException> (delegate {
				XmlReader.Create ("does-not-exists", null, null);
			}, "does-not-exists");

			XmlReader xr = XmlReader.Create ("AppManifest.xaml", null, null);
			CheckAppManifest (xr);
		}
	}
}

