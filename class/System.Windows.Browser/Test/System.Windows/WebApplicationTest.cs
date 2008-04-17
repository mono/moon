using System;
using System.Windows;
using System.Windows.Browser;

using NUnit.Framework;

namespace MonoTests.System.Windows
{
	[TestFixture]
	public class WebApplicationTest
	{
		[Scriptable]
		class Scriptable1
		{
			[Scriptable]
			public int Foo {
				get { return 0; }
			}
		}

		[Scriptable]
		class Scriptable2
		{
			[Scriptable]
			public event EventHandler MyEvent;
		}

		[Scriptable]
		class Scriptable3
		{
			[Scriptable]
			public string MyMethod (int i, string s, double d)
			{
				return null;
			}
		}

		class Scriptable4
		{
			[Scriptable]
			public string MyMethod (int i, string s, double d)
			{
				return null;
			}
		}

		[Scriptable]
		class Scriptable5
		{
			public string MyMethod (int i, string s, double d)
			{
				return null;
			}
		}

		[Scriptable]
		class Scriptable6
		{
			[Scriptable]
			public decimal MyMethod (int i)
			{
				return 0;
			}
		}

		[Scriptable]
		class Scriptable7
		{
			[Scriptable]
			public string MyMethod (Uri u)
			{
				return null;
			}
		}

		WebApplication app = WebApplication.Current;

		[Test]
		[ExpectedException (typeof (ArgumentNullException))]
		public void RegisterScriptableObjectNameNull ()
		{
			app.RegisterScriptableObject (null, new Scriptable1 ());
		}

		[Test]
		[ExpectedException (typeof (ArgumentNullException))]
		public void RegisterScriptableObjectInstanceNull ()
		{
			app.RegisterScriptableObject ("my", null);
		}

		[Test]
		[Category ("NotWorking")]
		public void RegisterScriptableObject ()
		{
			app.RegisterScriptableObject ("hoge", new Scriptable1 ());
			app.RegisterScriptableObject ("hoge2", new Scriptable2 ());
			app.RegisterScriptableObject ("hoge3", new Scriptable3 ());
			try {
				app.RegisterScriptableObject ("hoge4", new Scriptable4 ());
				Assert.Fail ("#4");
			} catch (NotSupportedException) {
			}
			try {
				app.RegisterScriptableObject ("hoge5", new Scriptable5 ());
				Assert.Fail ("#5");
			} catch (ArgumentException) {
			}
			try {
				app.RegisterScriptableObject ("hoge6", new Scriptable6 ());
				Assert.Fail ("#6");
			} catch (NotSupportedException) {
			}
			try {
				app.RegisterScriptableObject ("hoge7", new Scriptable7 ());
				Assert.Fail ("#7");
			} catch (NotSupportedException) {
			}
		}
	}
}
