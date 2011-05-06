//
// Unit tests for System.Xml.Serialization.XmlSerializer
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2011 Novell, Inc (http://www.novell.com)
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

using System.Xml;
using System.Xml.Linq;
using System.Xml.Serialization;

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Net.Sockets {

	public class BusinessObject {

		public BusinessObject ()
		{
		}

		[XmlAttribute]
		public int ID { get; set; }

		[XmlAttribute]
		public string Name { get; set; }

		[XmlAttribute]
		public string Description { get; set; }
	}

	[TestClass]
	public class XmlSerializerTest {

		XElement Serialize (string sName, object obj, XmlSerializerNamespaces ns)
		{
			XmlWriterSettings ws = new XmlWriterSettings ();
			ws.OmitXmlDeclaration = true;
			ws.NamespaceHandling = NamespaceHandling.OmitDuplicates;

			XDocument xDoc = new XDocument ();
			using (XmlWriter oWriter = XmlWriter.Create (xDoc.CreateWriter (), ws)) {
				XmlSerializer oSerializer = new XmlSerializer (obj.GetType ());
				oSerializer.Serialize (oWriter, obj, ns);
			}

			XElement xElem = xDoc.Root;
			xElem.Name = sName;
			return xElem;
		}

		[TestMethod] // ensure our S.SM.dll has the required internals for serialization
		public void Bug548913 ()
		{
			BusinessObject obj = new BusinessObject () { ID = 1, Name = "bo1", Description = "Business Object 1" };
			XElement xField = Serialize ("bo1", obj, new XmlSerializerNamespaces ());
			// our xml is not identical, but close enough to be valid, so we're checking only parts
			Assert.AreEqual ("1", xField.Attribute (XName.Get ("ID")).Value, "ID");
			Assert.AreEqual ("bo1", xField.Attribute (XName.Get ("Name")).Value, "Name");
			Assert.AreEqual ("Business Object 1", xField.Attribute (XName.Get ("Description")).Value, "Description");
		}
	}
}
