using System;
using System.Collections.Generic;
using System.Net;
using System.Windows.Browser.Serialization;

using NUnit.Framework;

using JSONObject = System.Collections.Generic.Dictionary<string,object>;

namespace MonoTests.System.Windows.Browser.Serialization
{
	[TestFixture]
	public class JavaScriptSerializerTest
	{
		[Test]
		public void DefaultValues ()
		{
			JavaScriptSerializer ser = new JavaScriptSerializer ();
			Assert.AreEqual (100, ser.RecursionLimit, "#1");
		}

		#region Serialization

		[Test]
		public void SerializeStringLiteral ()
		{
			JavaScriptSerializer ser = new JavaScriptSerializer ();
			Assert.AreEqual ("\"test string\"", ser.Serialize ("test string"), "#1");
			Assert.AreEqual ("\"test\\r\\n/string\\f\"", ser.Serialize ("test\r\n/string\f"), "#2");
		}

		[Test]
		public void SerializeObject ()
		{
			JavaScriptSerializer ser = new JavaScriptSerializer ();
			string s1 = @"{""P1"":""x1"",""P2"":123}";
			Assert.AreEqual (s1, ser.Serialize (new Foo ()), "#1");

			ser = new JavaScriptSerializer (new SimpleTypeResolver ());
			string s2 = @"{""__type"":""" + typeof (Foo).AssemblyQualifiedName + @""",""P1"":""x1"",""P2"":123}";
			Assert.AreEqual (s2, ser.Serialize (new Foo ()), "#2");
		}

		#endregion

		#region Deserialization

		[Test]
		[ExpectedException (typeof (ArgumentNullException))]
		public void DeserializeNullArg ()
		{
			new JavaScriptSerializer ().DeserializeObject (null);
		}

		[Test]
		public void DeserializeNull ()
		{
			JavaScriptSerializer ser = new JavaScriptSerializer ();
			Assert.IsNull (ser.DeserializeObject (""), "#1");
			Assert.IsNull (ser.DeserializeObject ("null"), "#2");
		}

		[Test]
		[Category ("NotDotNet")]
		public void DeserializeNumbers ()
		{
			JavaScriptSerializer ser = new JavaScriptSerializer ();
			Assert.AreEqual (-12345, ser.DeserializeObject ("-12345"), "#2");

			// they are .NET bugs.
			try {
				ser.DeserializeObject ("01");
				Assert.Fail ("#3");
			} catch (ArgumentException) {
			}
			try {
				ser.DeserializeObject ("123.");
				Assert.Fail ("#4");
			} catch (ArgumentException) {
			}
			// FIXME: do not skip
			Assert.AreEqual (-12345.000m, ser.DeserializeObject ("-12345.000"), "#5");
			Assert.AreEqual (-12345.04689, ser.DeserializeObject ("-12345.04689"), "#6");
			try {
				ser.DeserializeObject ("123e");
				Assert.Fail ("#7");
			} catch (ArgumentException) {
			}
			// it is a .NET bug.
			try {
				ser.DeserializeObject ("123.e4");
				Assert.Fail ("#8");
			} catch (ArgumentException) {
			}
			try {
				ser.DeserializeObject ("123.5-e4");
				Assert.Fail ("#9");
			} catch (ArgumentException) {
			}
			Assert.AreEqual (-123.5e-4, ser.DeserializeObject ("-123.5e-4"), "#10");
			Assert.AreEqual (-123.5e4, ser.DeserializeObject ("-123.5e+4"), "#11");
			try {
				ser.DeserializeObject ("123.5e4.7");
				Assert.Fail ("#12");
			} catch (ArgumentException) {
			}
		}

		[Test]
		public void DeserializeString ()
		{
			JavaScriptSerializer ser = new JavaScriptSerializer ();
			Assert.AreEqual ("test", ser.DeserializeObject ("\"test\""), "#1");
			try {
				ser.DeserializeObject ("'test'");
				Assert.Fail ("#2");
			} catch (ArgumentException) {
			}
			Assert.AreEqual ("test\\\r\n\t\f\u0065test", ser.DeserializeObject ("\"test\\\\\\r\\n\\t\\f\\u0065test\""), "#3");
//			try {
//				ser.DeserializeObject ("test\\u10000");
//				Assert.Fail ("#4");
//			} catch (ArgumentException) {
//			}
			// FIXME: not working (BTW .net is buggy too to fail here)
			// Assert.AreEqual ("test\u1000" + "0", ser.DeserializeObject ("test\\u10000"), "#5");
		}

		[Test]
		public void DeserializeArray ()
		{
			JavaScriptSerializer ser = new JavaScriptSerializer ();
			object [] arrobj = ser.DeserializeObject ("[0, \t 1, -2,3 ]") as object [];
			Assert.IsNotNull (arrobj, "#1-1");
			Assert.AreEqual (4, arrobj.Length, "#1-2");
			Assert.AreEqual (-2, arrobj [2], "#1-3");

			arrobj = ser.DeserializeObject ("[0,  \"[]\", [1,2],[3,4], {}]") as object [];
			Assert.IsNotNull (arrobj, "#2-1");
			Assert.AreEqual (5, arrobj.Length, "#2-2");
			Assert.AreEqual ("[]", arrobj [1], "#2-3");
			Assert.IsTrue (arrobj [2] is object [], "#2-4");
			Assert.IsTrue (arrobj [4] is Dictionary<string,object>, "#2-5");

			try {
				ser.DeserializeObject ("[\"abc\",[123,\"[]]\"]");
				Assert.Fail ("#3");
			} catch (ArgumentException) {
			}

			try {
				ser.DeserializeObject ("[\"abc\" 123]");
				Assert.Fail ("#4");
			} catch (ArgumentException) {
			}
		}

		[Test]
		public void DeserializeObject ()
		{
			JavaScriptSerializer ser = new JavaScriptSerializer ();

			JSONObject dic = ser.DeserializeObject ("{}") as JSONObject;
			Assert.IsNotNull (dic, "#1-1");
			Assert.AreEqual (0, dic.Count, "#1-2");

			try {
				ser.DeserializeObject ("{1}");
				Assert.Fail ("#2");
			} catch (ArgumentException) {
			}

			// .NET fails to reject it.
			try {
				ser.DeserializeObject ("{1:2}");
				Assert.Fail ("#3");
			} catch (ArgumentException) {
			}

			try {
				ser.DeserializeObject ("{\"ABC\"}");
				Assert.Fail ("#4");
			} catch (ArgumentException) {
			}

			try {
				ser.DeserializeObject ("{\"ABC\":}");
				Assert.Fail ("#5");
			} catch (ArgumentException) {
			}

			dic = ser.DeserializeObject ("{ \"A\":123 , \"1\" : 1}") as JSONObject;
			Assert.IsNotNull (dic, "#6-1");
			Assert.AreEqual (2, dic.Count, "#6-2");

			try {
				ser.DeserializeObject ("{\"ABC\": 1,}");
				Assert.Fail ("#7");
			} catch (ArgumentException) {
			}

			try {
				ser.DeserializeObject ("{\"A\":123 , \"1\": {\"nest\":\"{}}\"}");
				Assert.Fail ("#8");
			} catch (ArgumentException) {
			}

			dic = ser.DeserializeObject ("{\"A\": {\"x\":{}},\"B\" :[1,2,3,{\"nest\":543}]}") as JSONObject;
			Assert.IsNotNull (dic, "#9-1");
			Assert.AreEqual (2, dic.Count, "#9-2");
			JSONObject a = dic ["A"] as JSONObject;
			Assert.IsNotNull (a, "#9-3");
			Assert.IsTrue (a ["x"] is JSONObject, "#9-4");
			object [] b = dic ["B"] as object [];
			Assert.IsNotNull (b, "#9-5");
			Assert.AreEqual (4, b.Length, "#9-6");
			Assert.IsTrue (b [3] is JSONObject, "#9-7");
		}

		[Test]
		public void DeserializeObjectWithResolver ()
		{
			Assert.IsNotNull (new SimpleTypeResolver ().ResolveType (typeof (Foo).AssemblyQualifiedName), "premise#1");

			JavaScriptSerializer ser = new JavaScriptSerializer (new SimpleTypeResolver ());
			string s1 = @"{""__type"":""" + typeof (Foo).AssemblyQualifiedName + @""",""P1"":""x1"",""P2"":123}";
			Foo f = ser.DeserializeObject (s1) as Foo;
			Assert.IsNotNull (f, "#1:" + ser.DeserializeObject (s1));

			// put __type at last
			string s2 = @"{""P1"":""x1"",""P2"":123,""__type"":""" + typeof (Foo).AssemblyQualifiedName + @"""}";
			f = ser.DeserializeObject (s2) as Foo;
			Assert.IsNotNull (f, "#2");

			try {
				ser.DeserializeObject ("{\"__type\":\"UnresolvableType, nonexistent\"}");
				Assert.Fail ("#3");
			} catch (InvalidOperationException) {
			}
		}

		#endregion

		#region Conversion

		[Test]
		public void ConvertToType ()
		{
			JavaScriptSerializer ser = new JavaScriptSerializer ();
			Assert.AreEqual (1, ser.ConvertToType<int> ("1"), "#1");
			try {
				ser.ConvertToType<int> ("\"1\"");
				Assert.Fail ("#2");
			} catch (AssertionException) {
				throw;
			} catch (Exception) { // .NET throws System.Exception (!)
			}

			string [] arr1 = new string [] {"A", "B", "C"};
			Assert.AreEqual (arr1, ser.ConvertToType<object> (arr1), "#3");
			try {
				ser.ConvertToType<string> (arr1);
				Assert.Fail ("#4"); // at least no concat.
			} catch (InvalidOperationException) {
			}

			JSONObject dic = new JSONObject ();
			dic.Add ("P1", "x");
			dic.Add ("P2", 1);
			dic.Add ("__type", "bogus");
			dic.Add ("extra", 1); // any extra members are ignored
			Foo f = ser.ConvertToType<Foo> (dic);
			Assert.AreEqual ("x", f.P1, "#5");
			Assert.AreEqual (1, f.P2, "#6");

			ser.RegisterConverters (new JavaScriptConverter [] {new MyConverter1 ()});
			try {
				ser.ConvertToType<Foo> (dic);
				Assert.Fail ("#7");
			} catch (WebException) {
			}
		}

		#endregion

		#region classes

		class MyConverter1 : JavaScriptConverter
		{
			public override IEnumerable<Type> SupportedTypes {
				get { yield return typeof (Foo); }
			}

			public override object Deserialize (
				IDictionary<string, object> dictionary,
				Type type,
				JavaScriptSerializer serializer)
			{
				throw new WebException ();// something unusual
			}
		
			public override IDictionary<string, object> Serialize (
				object obj, JavaScriptSerializer serializer)
			{
				throw new WebException ();// something unusual
			}
		}

		class Foo
		{
			public Foo ()
			{
				P1 = "x1";
				P2 = 123;
				P3 = "y1";
			}

			string p1, p3;
			int p2;

			public string P1 {
				get { return p1; }
				set { p1 = value; }
			}

			public int P2 {
				get { return p2; }
				set { p2 = value; }
			}

			[ScriptIgnore]
			public string P3 {
				get { return p3; }
				set { p3 = value; }
			}
		}

		#endregion
	}
}
