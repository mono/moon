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
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows
{
	[TestClass]
	public class FrameworkElementTest
	{
		[TestMethod]
		[KnownFailure]
		public void TestName ()
		{
			Assert.AreEqual ("", (new Canvas()).Name);
		}

		[TestMethod]
		public void PropertyPriority()
		{
			Style style = new Style (typeof (Rectangle));
			style.Setters.Add (new Setter (Rectangle.OpacityProperty, 0.5));
			Rectangle r = new Rectangle ();
			Assert.AreEqual (1.0, r.Opacity, "#1");
			Assert.AreEqual (DependencyProperty.UnsetValue, r.ReadLocalValue (Rectangle.OpacityProperty), "#2");
			r.Opacity = 1.0;
			Assert.AreEqual (1.0, r.ReadLocalValue (Rectangle.OpacityProperty), "#3");
			r.ClearValue (Rectangle.OpacityProperty);
			Assert.AreEqual (DependencyProperty.UnsetValue, r.ReadLocalValue (Rectangle.OpacityProperty), "#4");
			r.Style = style;
			Assert.AreEqual (0.5, r.Opacity, "#5");
			Assert.AreEqual(DependencyProperty.UnsetValue, r.ReadLocalValue (Rectangle.OpacityProperty), "#6");
			r.Opacity = 1.0;
			Assert.AreEqual (1.0, r.Opacity, "#7");
			Assert.AreEqual (1.0, r.ReadLocalValue (Rectangle.OpacityProperty), "#8");
			r.ClearValue (Rectangle.OpacityProperty);
			Assert.AreEqual (0.5, r.Opacity, "#9");
			Assert.AreEqual (DependencyProperty.UnsetValue, r.ReadLocalValue (Rectangle.OpacityProperty), "#10");
		}
	}
}