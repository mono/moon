//
// Unit tests for System.Runtime.Serialization.Json.DataContractJsonSerializer
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
using System.Collections.Generic;
using System.IO;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Json;
using System.Security;
using System.Text;
using System.Xml;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Runtime.Serialization.Json {

	[TestClass]
	public class DataContractJsonSerializerTest {

		[TestMethod]
		public void Ctor_Type ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				new DataContractJsonSerializer (null);
			}, ".ctor(null)");

			DataContractJsonSerializer dcjs = new DataContractJsonSerializer (typeof (string));
			Assert.AreEqual (0, dcjs.KnownTypes.Count, "KnownTypes");
		}

		[TestMethod]
		public void Ctor_Type_NullKnownType ()
		{
			DataContractJsonSerializer dcjs;

			Assert.Throws<ArgumentNullException> (delegate {
				dcjs = new DataContractJsonSerializer (null, null);
			}, "null");

			dcjs = new DataContractJsonSerializer (typeof (int), null);
			Assert.AreEqual (0, dcjs.KnownTypes.Count, "KnownTypes");
		}

		[TestMethod]
		public void Ctor_Type_EmptyKnownType ()
		{
			DataContractJsonSerializer dcjs = new DataContractJsonSerializer (typeof (DateTime), Type.EmptyTypes);
			Assert.AreEqual (0, dcjs.KnownTypes.Count, "KnownTypes");
		}

		[TestMethod]
		public void Ctor_Type_SomeKnownType ()
		{
			Type [] known = new Type [] { typeof (int), typeof (string) };
			DataContractJsonSerializer dcjs = new DataContractJsonSerializer (typeof (char), known);
			Assert.AreEqual (2, dcjs.KnownTypes.Count, "KnownTypes");
		}

		[TestMethod]
		public void ReadNullStream ()
		{
			DataContractJsonSerializer dcjs = new DataContractJsonSerializer (typeof (string));
			Assert.Throws<ArgumentNullException> (delegate {
				dcjs.ReadObject (null);
			}, "ReadObject(null)");
		}

		[TestMethod]
		[MoonlightBug ("SL throws a NullReferenceException while ML throws an ArgumentNullException")]
		public void WriteNullStream ()
		{
			DataContractJsonSerializer dcjs = new DataContractJsonSerializer (typeof (string));
			Assert.Throws<NullReferenceException> (delegate {
				dcjs.WriteObject (null, "string");
			}, "WriteObject(null,object)");
		}

		[TestMethod]
		[MoonlightBug ("Mono WCF bug #573690")]
		public void ReadWriteNullString ()
		{
			DataContractJsonSerializer dcjs = new DataContractJsonSerializer (typeof (string));
			string data = null;
			using (MemoryStream ms = new MemoryStream ()) {
				dcjs.WriteObject (ms, null);
				ms.Position = 0;
				using (StreamReader sr = new StreamReader (ms)) {
					data = sr.ReadToEnd ();
					Assert.AreEqual ("null", data, "WriteObject(stream,null)");

					ms.Position = 0;
					Assert.IsNull (dcjs.ReadObject (ms), "ReadObject(stream)");
				}
			};
		}

		public object ReadWriteObject (Type type, object obj, string expected)
		{
			using (MemoryStream ms = new MemoryStream ()) {
				DataContractJsonSerializer dcjs = new DataContractJsonSerializer (type);
				dcjs.WriteObject (ms, obj);
				ms.Position = 0;
				using (StreamReader sr = new StreamReader (ms)) {
					Assert.AreEqual (expected, sr.ReadToEnd (), "WriteObject");

					ms.Position = 0;
					return dcjs.ReadObject (ms);
				}
			}
		}

		[TestMethod]
		public void ReadWriteObject_String ()
		{
			Assert.AreEqual ("My String", ReadWriteObject (typeof (string), "My String", "\"My String\""));
		}

		[TestMethod]
		public void ReadWriteObject_Int ()
		{
			Assert.AreEqual (Int32.MaxValue, ReadWriteObject (typeof (int), Int32.MaxValue, "2147483647"));
		}

		[TestMethod]
		public void ReadWriteObject_Double ()
		{
			Assert.AreEqual (1.5d, ReadWriteObject (typeof (double), 1.5d, "1.5"), "1.5d");
		}

		[TestMethod]
		[MoonlightBug ("Mono WCF bug #573691")]
		public void ReadWriteObject_Single_SpecialCases ()
		{
			Assert.IsTrue (Single.IsNaN ((float) ReadWriteObject (typeof (float), Single.NaN, "NaN")));
			Assert.IsTrue (Single.IsNegativeInfinity ((float) ReadWriteObject (typeof (float), Single.NegativeInfinity, "-INF")));
			Assert.IsTrue (Single.IsPositiveInfinity ((float) ReadWriteObject (typeof (float), Single.PositiveInfinity, "INF")));
		}

		[TestMethod]
		[MoonlightBug ("Mono WCF bug #573691")]
		public void ReadWriteObject_Double_SpecialCases ()
		{
			Assert.IsTrue (Double.IsNaN ((double) ReadWriteObject (typeof (double), Double.NaN, "NaN")));
			Assert.IsTrue (Double.IsNegativeInfinity ((double) ReadWriteObject (typeof (double), Double.NegativeInfinity, "-INF")));
			Assert.IsTrue (Double.IsPositiveInfinity ((double) ReadWriteObject (typeof (double), Double.PositiveInfinity, "INF")));
		}

		[DataContract]
		public class Contract {
			[DataMember]
			public string Member;
		}

		[TestMethod]
		public void ReadWriteObject_DataContract ()
		{
			Contract c1 = new Contract ();
			c1.Member = "me";

			Contract c2 = (Contract) ReadWriteObject (typeof (Contract), c1, "{\"Member\":\"me\"}");
			Assert.AreEqual (c1.Member, c2.Member, "Member");
		}

		[DataContract]
		class PrivateContract {
			[DataMember]
			public string Member;
		}

		[TestMethod]
		public void ReadObject_PrivateDataContract ()
		{
			string json = "{\"Member\":null}";
			byte [] data = Encoding.UTF8.GetBytes (json);
			var dcjs = new DataContractJsonSerializer (typeof (PrivateContract));
			using (MemoryStream ms = new MemoryStream (data)) {
				Assert.Throws<SecurityException,FieldAccessException> (delegate {
					dcjs.ReadObject (ms);
				}, "private type");
			}
		}

		[TestMethod]
		public void WriteObject_PrivateDataContract ()
		{
			var dcjs = new DataContractJsonSerializer (typeof (PrivateContract));
			using (MemoryStream ms = new MemoryStream ()) {
				Assert.Throws<SecurityException,FieldAccessException> (delegate {
					dcjs.WriteObject (ms, new PrivateContract ());
				}, "private type");
			}
		}

		[DataContract]
		public class PublicContractAboutPrivateClause {
			[DataMember]
			public string PublicFacingMember;
			[DataMember]
			internal string SecretPolicyMakerMember;
		}

		[TestMethod]
		public void ReadObject_DataContractPrivateMember ()
		{
			string json = "{\"PublicFacingMember\":\"me\",\"SecretPolicyMakerMember\":\"uho\"}";
			byte[] data = Encoding.UTF8.GetBytes (json);
			var dcjs = new DataContractJsonSerializer (typeof (PublicContractAboutPrivateClause));
			using (MemoryStream ms = new MemoryStream (data)) {
				Assert.Throws<SecurityException,FieldAccessException> (delegate {
					dcjs.ReadObject (ms);
				}, "private member");
			}
		}

		[TestMethod]
		public void ReadObject_DataContractIgnoringPrivateMember ()
		{
			string json = "{\"PublicFacingMember\":\"me\"}";
			byte [] data = Encoding.UTF8.GetBytes (json);
			var dcjs = new DataContractJsonSerializer (typeof (PublicContractAboutPrivateClause));
			using (MemoryStream ms = new MemoryStream (data)) {
				var c = (PublicContractAboutPrivateClause) dcjs.ReadObject (ms);
				Assert.AreEqual ("me", c.PublicFacingMember, "Public");
				Assert.IsNull (c.SecretPolicyMakerMember, "Internal");
			}
		}

		[TestMethod]
		public void WriteObject_DataContractPrivateMember ()
		{
			var dcjs = new DataContractJsonSerializer (typeof (PublicContractAboutPrivateClause));
			using (MemoryStream ms = new MemoryStream ()) {
				var c = new PublicContractAboutPrivateClause ();
				c.PublicFacingMember = "me";
				c.SecretPolicyMakerMember = "uho";
				Assert.Throws<SecurityException,FieldAccessException> (delegate {
					dcjs.WriteObject (ms, c);
				}, "private member");
			}
		}

		[TestMethod]
		public void WriteObject_DataContractIgnoringPrivateMember ()
		{
			var dcjs = new DataContractJsonSerializer (typeof (PublicContractAboutPrivateClause));
			using (MemoryStream ms = new MemoryStream ()) {
				var c = new PublicContractAboutPrivateClause ();
				c.PublicFacingMember = "me";
				// SecretPolicyMakerMember is (default) unassigned
				Assert.Throws<SecurityException,FieldAccessException> (delegate {
					dcjs.WriteObject (ms, c);
				}, "unassigned private member");
			}
		}
	}
}

