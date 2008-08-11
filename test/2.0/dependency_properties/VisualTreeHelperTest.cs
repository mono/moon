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

namespace MoonTest.System.Windows.Media
{
	[TestClass]
	public class VisualTreeHelperTest
	{
		[TestMethod]
		public void NullTests ()
		{
			Assert.Throws (delegate { VisualTreeHelper.GetChild (null, 1); }, typeof (InvalidOperationException));
			Assert.Throws (delegate { VisualTreeHelper.GetChildrenCount (null); }, typeof (InvalidOperationException));
			Assert.Throws (delegate { VisualTreeHelper.GetParent (null); }, typeof (InvalidOperationException));
		}

		[TestMethod]
		public void GetChild ()
		{
			Canvas p = new Canvas();
			Canvas p2 = new Canvas ();
			p.Children.Add (p2);

			Assert.AreEqual (p2, VisualTreeHelper.GetChild (p, 0));
			Assert.Throws (delegate { VisualTreeHelper.GetChild (p, 1); }, typeof (ArgumentOutOfRangeException));
		}

		[TestMethod]
		public void GetChildrenCount ()
		{
			Canvas p = new Canvas ();

			Assert.AreEqual (0, VisualTreeHelper.GetChildrenCount (p));

			p.Children.Add (new Canvas());

			Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (p));

			//			p.Resources.Add ("foo", new Canvas());

			Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (p));
		}

		[TestMethod]
		public void GetParent ()
		{
			Canvas p = new Canvas ();
			Canvas p2 = new Canvas ();
			Canvas p3 = new Canvas ();
			SolidColorBrush scb = new SolidColorBrush ();

			p.Children.Add (p2);
			//			p.Resources.Add ("foo", p3);

			p.Background = scb;

			Assert.AreEqual (p, VisualTreeHelper.GetParent (p2));
			//Assert.AreEqual (p, VisualTreeHelper.GetParent (p3));
			Assert.AreEqual (null, VisualTreeHelper.GetParent (p));

			Assert.Throws (delegate { VisualTreeHelper.GetParent (scb); }, typeof (InvalidOperationException));
		}
	}
}