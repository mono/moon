
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
	[Flags]
	public enum Alpha {
		A, B, C, D,E
	}

	[TestClass]
	public class EnumsTest : SilverlightTest
	{

		[TestMethod]
		public void ParseManagedEnum ()
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

		[TestMethod]
		public void ParseManagedEnumLowerCase ()
		{
			Canvas c;

			c = (Canvas) XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
					xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
					xmlns:vsm=""clr-namespace:System.Windows;assembly=System.Windows"">
					<Canvas.Resources><vsm:Visibility x:Key=""visibility"">collapsed</vsm:Visibility></Canvas.Resources></Canvas>");

			Assert.IsNotNull (c.Resources ["visibility"], "1");
			Assert.AreEqual (typeof (Visibility), c.Resources ["visibility"].GetType (), "2");
			Assert.AreEqual (Visibility.Collapsed, c.Resources ["visibility"], "3");
		}

		[TestMethod]
		public void ParseCustomEnum ()
		{
			var c = (Canvas) XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
							   	    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
								    xmlns:c=""clr-namespace:MoonTest.Misc;assembly=moon-unit"">
								      <Canvas.Resources>
									<c:Alpha x:Key=""alpha"">E</c:Alpha>
					                              </Canvas.Resources>
							    </Canvas>");

			Assert.IsNotNull (c.Resources ["alpha"], "1");
			Assert.AreEqual (Alpha.E, c.Resources ["alpha"]);
		}
		
		[TestMethod]
		public void ParseWpfStyleFlags ()
		{
					var c = (Canvas) XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
							   	    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
								    xmlns:c=""clr-namespace:MoonTest.Misc;assembly=moon-unit"">
								      <Canvas.Resources>
									<c:Alpha x:Key=""alpha"">A,B,C</c:Alpha>
					                              </Canvas.Resources>
							    </Canvas>");

			Assert.IsNotNull (c.Resources ["alpha"], "1");
			Assert.AreEqual (Alpha.A | Alpha.B | Alpha.C, c.Resources ["alpha"]);
		}
	}
}

