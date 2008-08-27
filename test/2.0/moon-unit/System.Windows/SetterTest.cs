using System;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Markup;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Collections.Generic;
using Mono.Moonlight.UnitTesting;


namespace MoonTest.System.Windows
{
	[TestClass]
	public class SetterTest
	{
		[TestMethod]
		[KnownFailure]
		public void CreateTest ()
		{
			Setter s = new Setter (UIElement.OpacityProperty, 2.0);

			Assert.AreEqual (UIElement.OpacityProperty, s.Property);
			Assert.AreEqual (2.0, s.Value);
		}

		[TestMethod]
		public void NullProperty ()
		{
			Assert.Throws (delegate { Setter s = new Setter (null, 2.0); }, typeof (NullReferenceException));
		}

		[TestMethod]
		[KnownFailure]
		public void TypeMismatch ()
		{
			Setter s = new Setter (UIElement.OpacityProperty, "does this work?");
		}

		[TestMethod]
		[KnownFailure]
		public void StyleOfDifferentType ()
		{
			Setter s = new Setter (Line.X1Property, 10.0);
			Style style = new Style (typeof (Rectangle));
			style.Setters.Add (s);
		}

		[TestMethod]
		[KnownFailure]
		public void Parse ()
		{
			Setter s = (Setter)XamlReader.Load ("<Setter xmlns=\"http://schemas.microsoft.com/client/2007\" Property=\"IsEnabled\" Value=\"hi\" />");
			Assert.IsNull (s.Property);
			Assert.AreEqual ("hi", s.Value);
		}

		[TestMethod]
		[KnownFailure]
		public void ParseAndAddToStyle ()
		{
			Setter s = (Setter)XamlReader.Load ("<Setter xmlns=\"http://schemas.microsoft.com/client/2007\" Property=\"Width\" Value=\"5.0\" />");

			Assert.IsNull (s.Property);
			Assert.AreEqual ("5.0", s.Value);

			Style style = new Style(typeof (Rectangle));
			style.Setters.Add (s);

			Assert.IsNull (s.Property);
			Assert.AreEqual ("5.0", s.Value);

			Rectangle r = new Rectangle ();
			r.Style = style;

			Assert.IsNull (s.Property);
			Assert.AreEqual ("5.0", s.Value);
			Assert.IsTrue (Double.IsNaN(r.Width));
		}
	}
}
