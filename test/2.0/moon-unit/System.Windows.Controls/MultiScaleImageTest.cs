using System;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Markup;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Collections.Generic;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.Silverlight.Testing.UnitTesting.UI;
using Microsoft.Silverlight.Testing.UnitTesting.Metadata.VisualStudio;
using Microsoft.Silverlight.Testing;

namespace MoonTest.System.Windows.Controls
{
	[TestClass]
	public partial class MultiScaleImageTest : Microsoft.Silverlight.Testing.SilverlightTest
	{
		[TestMethod]
		[MoonlightBug]
		public void ViewportWidthInMarkup ()
		{
			MultiScaleImage msi = (MultiScaleImage)XamlReader.Load (@"<MultiScaleImage xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" ViewportWidth=""0.5"" />");
			Assert.AreEqual (0.5, msi.ViewportWidth, "1");
		}

		[TestMethod]
		public void UseSpringsNoAnimations ()
		{
			MultiScaleImage msi = new MultiScaleImage ();

			msi.UseSprings = false;
			msi.ViewportWidth = 2.0;
			Assert.AreEqual (2.0, msi.ViewportWidth, "OriginalWidth");
			msi.ViewportWidth = .2;
			Assert.AreEqual (.2, msi.ViewportWidth, "FinalWidth");
		}

		[TestMethod]
		public void ChangeUseSprings ()
		{
			MultiScaleImage msi = new MultiScaleImage ();

			msi.UseSprings = true;
			msi.ViewportWidth = 3.0;
			Assert.AreEqual (1.0, msi.ViewportWidth, "OriginalWidth");
			msi.ViewportWidth = .3;
			msi.UseSprings = false;
			Assert.AreEqual (.3, msi.ViewportWidth, "FinalWidth");
		}
	}
}
