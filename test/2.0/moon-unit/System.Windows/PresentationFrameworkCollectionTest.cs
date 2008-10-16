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
		[KnownFailure]
		public void RemoveAt ()
		{
			var canvas = new Canvas ();
			var children = canvas.Children;

			children.Add (new Rectangle ());
			children.RemoveAt (0);

			Assert.Throws<ArgumentOutOfRangeException> (() => children.RemoveAt (1));
		}
	}
}
