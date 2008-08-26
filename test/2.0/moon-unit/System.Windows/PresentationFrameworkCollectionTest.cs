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
	public class PresentationFrameworkCollectionTest
	{
		[TestMethod]
		public void AddWithoutParent ()
		{
			UIElementCollection col = new UIElementCollection();
			Assert.Throws (delegate { col.Add (new Rectangle()); }, typeof (NullReferenceException));
		}

		[TestMethod]
		public void RemoveAt ()
		{
			Canvas c = new Canvas ();
			UIElementCollection col = c.Children;
			col.Add (new Rectangle());
			col.RemoveAt (0);
			Assert.Throws (delegate { col.RemoveAt (1); }, typeof (ArgumentException));
		}
	}
}
