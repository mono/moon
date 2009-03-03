
using System;
using System.Net;
using System.Resources;
using System.Windows;
using System.Windows.Markup;
using System.Windows.Controls;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.Silverlight.Testing;


namespace MoonTest.Misc
{
	[TestClass]
	public class EnumsTest : SilverlightTest
	{

		[TestMethod]
		[MoonlightBug]
		public void ParseManagedEnum3 ()
		{
			Canvas c;

			c = (Canvas) XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
					xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
					xmlns:vsm=""clr-namespace:System.Windows;assembly=System.Windows"">
					<Canvas.Resources><vsm:Visibility x:Key=""visibility"">Collapsed</vsm:Visibility></Canvas.Resources></Canvas>");

			Assert.IsNotNull (c.Resources ["visibility"], "1");
			Assert.AreEqual (typeof (Visibility), c.Resources ["visibility"].GetType (), "2");
			Assert.AreEqual (Visibility.Collapsed, c.Resources ["visibility"], "3");
		}
	}
}

