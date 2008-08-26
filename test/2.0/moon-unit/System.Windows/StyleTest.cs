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

		[TestMethod]
		public void MismatchTargetType ()
		{
			Style s = (Style)XamlReader.Load (@"<Style xmlns=""http://schemas.microsoft.com/client/2007"" TargetType=""CheckBox""><Setter Property=""Width"" Value=""10""/></Style>");
			Button b = new Button ();

			Assert.Throws (delegate { b.Style = s; }, typeof (XamlParseException));
		}

		[TestMethod]
		public void MissingTargetType ()
		{
			Style s = (Style)XamlReader.Load (@"<Style xmlns=""http://schemas.microsoft.com/client/2007""><Setter Property=""Width"" Value=""10""/></Style>");
			Button b = new Button ();

			Assert.Throws (delegate { b.Style = s; }, typeof (NullReferenceException));

			// we need a new button or else the b.Style assignment below won't occur
			b = new Button ();

			s = (Style)XamlReader.Load (@"<Style xmlns=""http://schemas.microsoft.com/client/2007""><Setter Property=""Width"" Value=""10""/></Style>");
			s.TargetType = typeof (Button);

			Assert.AreEqual (typeof (Button), s.TargetType);

			b.Style = s;

			// in a perfect world (or maybe a less broken one), the following would be true
			// Assert.AreEqual (10, b.Width);
			//
			// but in SL, it seems that if you're missing
			// the target type in the xaml you create the
			// style with, it's forever lost to you, even
			// if you set it in code before assigning the
			// style to an element.  You don't get an
			// exception like above, though.
			Assert.IsTrue (Double.IsNaN(b.Width));
		}
	}
}
