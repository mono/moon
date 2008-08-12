using System;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Collections.Generic;

using dependency_properties;

namespace MoonTest.System.Windows
{
	[TestClass]
	public class SetterTest
	{
		[TestMethod]
		public void CreateTest ()
		{
			Setter s = new Setter (UIElement.OpacityProperty, 2.0);

			Assert.AreEqual (UIElement.OpacityProperty, s.Property);
			Assert.AreEqual (2.0, s.Value);
		}

		[TestMethod]
		public void TypeMismatch ()
		{
			Setter s = new Setter (UIElement.OpacityProperty, "does this work?");
		}

		[TestMethod]
		public void StyleOfDifferentType ()
		{
			Setter s = new Setter (Line.X1Property, 10.0);
			Style style = new Style (typeof (Rectangle));
			style.Setters.Add (s);
		}
	}
}
