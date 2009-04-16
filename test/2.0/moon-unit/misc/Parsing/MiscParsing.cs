
using System;
using System.Net;
using System.Resources;
using System.Windows;
using System.Windows.Markup;
using System.Windows.Controls;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.Silverlight.Testing;


namespace MoonTest.Misc.Parsing
{
	internal class MiscParsingTestPage : Canvas {

		public MiscParsingTestPage ()
		{
			Application.LoadComponent (this, new Uri ("/moon-unit;component/misc/Parsing/MiscParsingTestPage.xaml", UriKind.Relative));
		}
	}

	public class MiscParsingCanvas : Canvas {

		public MiscParsingCanvas ()
		{
			Application.LoadComponent (this, new Uri ("/moon-unit;component/misc/Parsing/MiscParsingCanvas.xaml", UriKind.Relative));
		}
	}

	[TestClass]
	public class MiscParsingTest : SilverlightTest
	{
		[TestMethod]
		[MoonlightBug]
		public void FindNameOfManagedControl ()
		{
			MiscParsingTestPage page = new MiscParsingTestPage ();
			

			Assert.IsNotNull (page, "1");

			MiscParsingCanvas canvas = (MiscParsingCanvas) page.FindName ("the_canvas");
			Assert.IsNotNull (canvas, "2");
			Assert.IsNotNull (canvas.FindName ("the_block"), "3");
			Assert.IsNull (page.FindName ("the_block"), "4");

			//
			// This shows that it actually isn't a parsing issue, setting the
			// name in code has the same problems.
			//
			
			TextBlock the_block = (TextBlock) canvas.FindName ("the_block");
			the_block.Name = "some_name";
			Assert.IsNotNull (canvas.FindName ("some_name"), "5");
			Assert.IsNull (canvas.FindName ("the_block"), "6");
			Assert.IsNull (page.FindName ("some_name"), "7");

			//
			// Make sure we are picking up the sub element and not ourself
			//
			Canvas sub_canvas = canvas.FindName ("the_canvas") as Canvas;
			Assert.IsNotNull (sub_canvas, "8");
			Assert.AreEqual (0, sub_canvas.Children.Count, "9");
		}
	}
}

