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
	public class StyleTest
	{
		[TestMethod]
		[KnownFailure]
		public void Sealed ()
		{
			Style style = new Style (typeof (UIElement));

			style.Seal ();
			
			// This should throw, no?
			/*Assert.Throws (delegate {*/ style.TargetType = typeof (FrameworkElement);/* }, typeof (Exception));*/

			// This too?
			/*Assert.Throws (delegate {*/ style.TargetType = typeof (SolidColorBrush);/* }, typeof (Exception));*/
		}

		[TestMethod]
		[KnownFailure]
		public void SetTwiceOnElement ()
		{
			Style style = new Style (typeof (Rectangle));
			Rectangle r = new Rectangle ();

			r.Style = style;
			Assert.Throws (delegate { r.Style = style; }, typeof (Exception));
		}

		[TestMethod]
		public void AvailableBeforeLoaded ()
		{
			Style s = (Style)XamlReader.Load (@"<Style xmlns=""http://schemas.microsoft.com/client/2007"" TargetType=""Button""><Setter Property=""Width"" Value=""10""/></Style>");
			Button b = new Button ();

			Assert.IsTrue (Double.IsNaN(b.Width));

			b.Style = s;

			Assert.AreEqual (10, b.Width);
		}
	}
}
