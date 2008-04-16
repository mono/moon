using System;
using System.Windows.Browser.Serialization;

using NUnit.Framework;

namespace MonoTests.System.Windows.Browser.Serialization
{
	[TestFixture]
	public class SimpleTypeResolverTest
	{
		[Test]
		[ExpectedException (typeof (ArgumentNullException))]
		public void ResolveTypeIdNull ()
		{
			new SimpleTypeResolver ().ResolveType (null);
		}

		[Test]
		[Category ("NotWorking")]
		public void ResolveTypeIdFine ()
		{
			SimpleTypeResolver r = new SimpleTypeResolver ();
			Assert.AreEqual (typeof (string).AssemblyQualifiedName, r.ResolveTypeId (typeof (string)), "#1");
			Assert.AreEqual (typeof (double).AssemblyQualifiedName, r.ResolveTypeId (typeof (double)), "#2");
			Assert.AreEqual (typeof (bool).AssemblyQualifiedName, r.ResolveTypeId (typeof (bool)), "#3");
			Assert.AreEqual (typeof (void).AssemblyQualifiedName, r.ResolveTypeId (typeof (void)), "#4");
			Assert.AreEqual (typeof (Array).AssemblyQualifiedName, r.ResolveTypeId (typeof (Array)), "#5");
			Assert.AreEqual (typeof (object).AssemblyQualifiedName, r.ResolveTypeId (typeof (object)), "#6");

			Assert.AreEqual (typeof (float).AssemblyQualifiedName, r.ResolveTypeId (typeof (float)), "#7");
		}

		[Test]
		[ExpectedException (typeof (ArgumentNullException))]
		public void ResolveTypeNull ()
		{
			new SimpleTypeResolver ().ResolveType (null);
		}

		[Test]
//		[Ignore ("This test is still wrong")]
		public void ResolveTypeFine ()
		{
			SimpleTypeResolver r = new SimpleTypeResolver ();
//			Assert.AreEqual (typeof (string), r.ResolveType ("string"), "#1");
			Assert.IsNull (r.ResolveType ("string"), "#1");
			Assert.AreEqual (typeof (string), r.ResolveType ("System.String"), "#1-2");
			/*
			Assert.AreEqual (typeof (double), r.ResolveType ("number"), "#2");
			Assert.AreEqual (typeof (bool), r.ResolveType ("boolean"), "#3");
			Assert.AreEqual (typeof (void), r.ResolveType ("null"), "#4");
			Assert.AreEqual (typeof (Array), r.ResolveType ("array"), "#5");
			Assert.AreEqual (typeof (object), r.ResolveType ("object"), "#6");
			*/
		}
	}
}
