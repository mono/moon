using System;
using System.ComponentModel;
using System.Globalization;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
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
using Microsoft.Silverlight.Testing;

namespace MoonTest.System.Windows
{
	[TestClass]
	public class StyleTest_Implicit : SilverlightTest {

		[TestMethod]
		[Asynchronous]
		public void TestImplicitStyleRectangle_styleInRectangleDictionary ()
		{
			Style rectStyle = new Style { TargetType = typeof (Rectangle) };
			Setter setter = new Setter (FrameworkElement.WidthProperty, 100.0);
			rectStyle.Setters.Add (setter);

			Rectangle r = new Rectangle ();

			Assert.IsTrue (Double.IsNaN (r.Width), "1");

			r.Resources.Add (typeof (Rectangle), rectStyle);

			Assert.AreEqual (100.0, r.Width, "2");

			CreateAsyncTest (r,  () => {
					Assert.AreEqual (100.0, r.Width, "3");

					//setter.Value = 200.0;
					//Assert.AreEqual (200.0, r.Width, "4");

					rectStyle.Setters.Remove (setter);

					Assert.AreEqual (100.0, r.Width, "5");
				});
		}

		[TestMethod]
		[Asynchronous]
		public void TestImplicitStyleRectangle_multipleImplicitStylesInVisualTree ()
		{
			Style rectStyle1 = new Style { TargetType = typeof (Rectangle) };
			Setter setter = new Setter (FrameworkElement.WidthProperty, 100.0);
			rectStyle1.Setters.Add (setter);

			Style rectStyle2 = new Style { TargetType = typeof (Rectangle) };
			setter = new Setter (FrameworkElement.HeightProperty, 100.0);
			rectStyle2.Setters.Add (setter);

			Rectangle r = new Rectangle ();
			r.Resources.Add (typeof (Rectangle), rectStyle1);

			Canvas c = new Canvas ();
			c.Resources.Add (typeof (Rectangle), rectStyle2);

			c.Children.Add (r);

			Assert.IsTrue (Double.IsNaN (r.Height), "1");

			CreateAsyncTest (c,  () => {
					Assert.AreEqual (100.0, r.Width, "2");
					Assert.IsTrue (Double.IsNaN (r.Height), "3");

					r.Resources.Remove (typeof (Rectangle));

					Assert.AreEqual (100.0, r.Height, "4");
					Assert.IsTrue (Double.IsNaN (r.Width), "5");
				});
		}

		[TestMethod]
		[Asynchronous]
		public void TestImplicitStyleRectangle_oneStyleInVisualTree_onStyleInAppResources ()
		{
			Style rectStyle1 = new Style { TargetType = typeof (Rectangle) };
			Setter setter = new Setter (FrameworkElement.WidthProperty, 100.0);
			rectStyle1.Setters.Add (setter);

			Style rectStyle2 = new Style { TargetType = typeof (Rectangle) };
			setter = new Setter (FrameworkElement.HeightProperty, 100.0);
			rectStyle2.Setters.Add (setter);

			Rectangle r = new Rectangle ();
			r.Resources.Add (typeof (Rectangle), rectStyle1);

			Application.Current.Resources.Remove (typeof (Rectangle));
			Application.Current.Resources.Add (typeof (Rectangle), rectStyle2);

			Assert.IsTrue (Double.IsNaN (r.Height), "1");

			CreateAsyncTest (r,  () => {
					Assert.AreEqual (100.0, r.Width, "2");
					Assert.IsTrue (Double.IsNaN (r.Height), "3");

					Application.Current.Resources.Remove (typeof (Rectangle));
				});
		}

		[TestMethod]
		[Asynchronous]
		public void TestImplicitStyleButton_oneStyleInVisualTree_onStyleInAppResources ()
		{
			Style style1 = new Style { TargetType = typeof (Button) };
			Setter setter = new Setter (FrameworkElement.WidthProperty, 100.0);
			style1.Setters.Add (setter);

			Style style2 = new Style { TargetType = typeof (Button) };
			setter = new Setter (FrameworkElement.HeightProperty, 100.0);
			style2.Setters.Add (setter);

			Button b = new Button ();
			b.Resources.Add (typeof (Button), style1);

			Application.Current.Resources.Remove (typeof (Button));
			Application.Current.Resources.Add (typeof (Button), style2);

			Assert.IsTrue (Double.IsNaN (b.Height), "1");

			CreateAsyncTest (b,  () => {
					b.UpdateLayout ();

					Assert.AreEqual (100.0, b.Width, "2");
					Assert.IsTrue (Double.IsNaN (b.Height), "3");

					Application.Current.Resources.Remove (typeof (Button));
				});
		}
	}

}
