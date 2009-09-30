//Permission is hereby granted, free of charge, to any person obtaining
//a copy of this software and associated documentation files (the
//"Software"), to deal in the Software without restriction, including
//without limitation the rights to use, copy, modify, merge, publish,
//distribute, sublicense, and/or sell copies of the Software, and to
//permit persons to whom the Software is furnished to do so, subject to
//the following conditions:
//
//The above copyright notice and this permission notice shall be
//included in all copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
//LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
//OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
//WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// Copyright (c) 2009 Novell, Inc.
//
// Authors:
//	Moonlight List (moonlight-list@lists.ximian.com)
//

using System;
using System.Windows;
using System.Windows.Browser;
using System.Windows.Controls;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Browser {

	[TestClass]
	public class HtmlPageTest {

		public class Ok {

			public Ok ()
			{
			}
		}

		public class InternalDefaultCtor {

			internal InternalDefaultCtor ()
			{
			}
		}

		public enum Enumeration {
		}

		public delegate string ManagedDelegate (int x);

		[TestMethod]
		public void RegisterCreateableType ()
		{
			Type t = typeof (Ok);

			Assert.Throws<ArgumentNullException> (delegate {
				HtmlPage.RegisterCreateableType (null, t);
			}, "null,type");
			Assert.Throws<ArgumentException> (delegate {
				HtmlPage.RegisterCreateableType (String.Empty, t);
			}, "Empty,type");
			Assert.Throws<ArgumentException> (delegate {
				HtmlPage.RegisterCreateableType ("a\0b", t);
			}, "string-with-null,type");

			Assert.Throws<ArgumentNullException> (delegate {
				HtmlPage.RegisterCreateableType ("a", null);
			}, "string,null");

			HtmlPage.RegisterCreateableType ("test-ok", t);
			Assert.Throws<ArgumentException> (delegate {
				HtmlPage.RegisterCreateableType ("test-ok", t);
			}, "twice");
			// same type can be re-registred under another name
			HtmlPage.RegisterCreateableType ("test-ok-again", t);

			Assert.Throws<ArgumentException> (delegate {
				HtmlPage.RegisterCreateableType ("test", typeof (string));
			}, "string");
			Assert.Throws<ArgumentException> (delegate {
				HtmlPage.RegisterCreateableType ("test", typeof (int));
			}, "primitive");
			Assert.Throws<ArgumentException> (delegate {
				HtmlPage.RegisterCreateableType ("test", typeof (ManagedDelegate));
			}, "managed-delegate");
			Assert.Throws<ArgumentException> (delegate {
				HtmlPage.RegisterCreateableType ("test", typeof (InternalDefaultCtor));
			}, "ctor");

			// LAMESPEC: value types are documented as invalid
			HtmlPage.RegisterCreateableType ("test-struct", typeof (CornerRadius)); // struct
			HtmlPage.RegisterCreateableType ("test-enum", typeof (CrossDomainAccess)); // enum
		}

		[TestMethod]
		public void UnregisterCreateableType ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				HtmlPage.UnregisterCreateableType (null);
			}, "null");
			Assert.Throws<ArgumentException> (delegate {
				HtmlPage.UnregisterCreateableType (String.Empty);
			}, "Empty");
			Assert.Throws<ArgumentException> (delegate {
				HtmlPage.UnregisterCreateableType ("a\0b");
			}, "string-with-null");

			Assert.Throws<ArgumentException> (delegate {
				HtmlPage.UnregisterCreateableType ("does-not-exists");
			}, "does-not-exists");

			HtmlPage.RegisterCreateableType ("test-temp-enum", typeof (CrossDomainAccess));
			HtmlPage.UnregisterCreateableType ("test-temp-enum");
			// LAMESPEC "No action is taken if scriptAlias no longer exists."
			Assert.Throws<ArgumentException> (delegate {
				HtmlPage.UnregisterCreateableType ("test-temp-enum");
			}, "twice");
		}

		class Private {
		}

		public class PublicScriptable {

			internal PublicScriptable ()
			{
			}

			[ScriptableMember]
			public int Foo ()
			{
				return 42;
			}
		}

		[TestMethod]
		public void RegisterScriptableObject ()
		{
			PublicScriptable instance = new PublicScriptable ();

			Assert.Throws<ArgumentNullException> (delegate {
				HtmlPage.RegisterScriptableObject (null, instance);
			}, "null,instance");
			Assert.Throws<ArgumentException> (delegate {
				HtmlPage.RegisterScriptableObject (String.Empty, instance);
			}, "Empty,instance");
			Assert.Throws<ArgumentException> (delegate {
				HtmlPage.RegisterScriptableObject ("a\0b", instance);
			}, "string-with-null,instance");

			Assert.Throws<ArgumentNullException> (delegate {
				HtmlPage.RegisterScriptableObject ("a", null);
			}, "string,null");

			Assert.Throws<InvalidOperationException> (delegate {
				HtmlPage.RegisterScriptableObject ("non-public", new Private ());
			}, "string,non-public");
			Assert.Throws<ArgumentException> (delegate {
				HtmlPage.RegisterScriptableObject ("no-scriptable-member", new Ok ());
			}, "string,no-scriptable-member");
		}

		public class PrivateScriptable {

			[ScriptableMember]
			private int Foo ()
			{
				return 42;
			}
		}

		public class ProtectedScriptable {

			[ScriptableMember]
			protected int Foo ()
			{
				return 42;
			}
		}

		public class InternalScriptable {

			[ScriptableMember]
			internal int Foo ()
			{
				return 42;
			}
		}

		public class StaticScriptable {

			[ScriptableMember]
			static public int Foo ()
			{
				return 42;
			}
		}

		[TestMethod]
		public void RegisterScriptableObject_Visibility ()
		{
			HtmlPage.RegisterScriptableObject ("public-scriptable", new PublicScriptable ());
			HtmlPage.RegisterScriptableObject ("static-scriptable", new StaticScriptable ());

			Assert.Throws<ArgumentException> (delegate {
				HtmlPage.RegisterScriptableObject ("private-scriptable", new PrivateScriptable ());
			}, "string,private-scriptable");
			Assert.Throws<ArgumentException> (delegate {
				HtmlPage.RegisterScriptableObject ("protected-scriptable", new ProtectedScriptable ());
			}, "string,protected-scriptable");
			Assert.Throws<ArgumentException> (delegate {
				HtmlPage.RegisterScriptableObject ("internal-scriptable", new InternalScriptable ());
			}, "string,internal-scriptable");
		}

		public class Scriptable_addEventListener {

			[ScriptableMember]
			public int addEventListener ()
			{
				return 42;
			}
		}

		public class Scriptable_removeEventListener {

			[ScriptableMember]
			public int removeEventListener ()
			{
				return 42;
			}
		}

		public class Scriptable_CONSTRUCTOR {

			[ScriptableMember]
			public int CONSTRUCTOR ()
			{
				return 42;
			}
		}

		public class Scriptable_constructor {

			[ScriptableMember]
			public int constructor ()
			{
				return 42;
			}
		}

		public class Scriptable_createManagedObject {

			[ScriptableMember]
			public int createManagedObject ()
			{
				return 42;
			}
		}

		public class NonScriptableReservedNames {

			[ScriptableMember]
			static public int Foo ()
			{
				return 42;
			}

			public int createManagedObject ()
			{
				return 42;
			}

			public int constructor ()
			{
				return 42;
			}

			public int addEventListener ()
			{
				return 42;
			}

			public int removeEventListener ()
			{
				return 42;
			}
		}

		[TestMethod]
		public void RegisterScriptableObject_Reserved ()
		{
			PublicScriptable instance = new PublicScriptable ();

			// reserved *method* (not registration) names
			HtmlPage.RegisterScriptableObject ("addEventListener", instance);
			HtmlPage.RegisterScriptableObject ("removeEventListener", instance);
			HtmlPage.RegisterScriptableObject ("constructor", instance);
			HtmlPage.RegisterScriptableObject ("createManagedObject", instance);

			// case sensitive test case
			HtmlPage.RegisterScriptableObject ("CONSTRUCTOR-casing", new Scriptable_CONSTRUCTOR ());

			// reserved members
			Assert.Throws<InvalidOperationException> (delegate {
				HtmlPage.RegisterScriptableObject ("bad-addEventListener", new Scriptable_addEventListener ());
			}, "addEventListener,instance");
			Assert.Throws<InvalidOperationException> (delegate {
				HtmlPage.RegisterScriptableObject ("bad-removeEventListener", new Scriptable_removeEventListener ());
			}, "removeEventListener,instance");
			Assert.Throws<InvalidOperationException> (delegate {
				HtmlPage.RegisterScriptableObject ("bad-CONSTRUCTOR", new Scriptable_constructor ());
			}, "constructor,instance");
			Assert.Throws<InvalidOperationException> (delegate {
				HtmlPage.RegisterScriptableObject ("bad-createManagedObject", new Scriptable_createManagedObject ());
			}, "createManagedObject,instance");

			HtmlPage.RegisterScriptableObject ("non-scriptable-reserved-names", new NonScriptableReservedNames ());
		}
	}
}

