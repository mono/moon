
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
	public class PrimitiveTest : SilverlightTest
	{

		[TestMethod]
		public void ParseStringKey ()
		{
			Canvas c;
			string s;

			c = (Canvas) XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
						   xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
 						   xmlns:sys=""clr-namespace:System;assembly=mscorlib""><Canvas.Resources><sys:String x:Key=""hola"">hola</sys:String></Canvas.Resources></Canvas>");

			s = c.Resources ["hola"] as string;
			Assert.AreEqual (1, c.Resources.Count, "1");
			Assert.IsNotNull (s, "2");
			Assert.AreEqual (s, "hola", "3");
		}

		[TestMethod]
		public void ParseStringName ()
		{
			Canvas c;
			string s;

			c = (Canvas) XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
						   xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
 						   xmlns:sys=""clr-namespace:System;assembly=mscorlib""><Canvas.Resources><sys:String x:Name=""hola"">hola</sys:String></Canvas.Resources></Canvas>");

			s = c.Resources ["hola"] as string;
			Assert.AreEqual (1, c.Resources.Count, "1");
			Assert.IsNotNull (s, "2");
			Assert.AreEqual (s, "hola", "3");
		}

		[TestMethod]
		[MoonlightBug ("we don't throw an exception")]
		public void ParseStringKeyAndName ()
		{
			Assert.Throws<XamlParseException> (delegate { XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
						   xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
 						   xmlns:sys=""clr-namespace:System;assembly=mscorlib""><Canvas.Resources><sys:String x:Name=""hola"" x:Key=""hola"">hola</sys:String></Canvas.Resources></Canvas>"); }, "1");
		}

		[TestMethod]
		public void ParseEmptyString ()
		{
			string s;

			Canvas c = (Canvas) XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
						   xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
 						   xmlns:sys=""clr-namespace:System;assembly=mscorlib""><Canvas.Resources><sys:String x:Name=""hola""></sys:String></Canvas.Resources></Canvas>");

			s = c.Resources ["hola"] as string;
			Assert.AreEqual (1, c.Resources.Count, "1");
			Assert.IsNotNull (s, "2");
			Assert.AreEqual (s, String.Empty, "3");

		}

		[TestMethod]
		public void ParseStringNonSys ()
		{
			Canvas c;
			string s;

			// See if you can use a xmlns other than sys:
			c = (Canvas) XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
						   xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
 						   xmlns:moon=""clr-namespace:System;assembly=mscorlib""><Canvas.Resources><moon:String x:Key=""hola"">hola</moon:String></Canvas.Resources></Canvas>");

			s = c.Resources ["hola"] as string;
			Assert.AreEqual (1, c.Resources.Count, "1");
			Assert.IsNotNull (s, "2");
			Assert.AreEqual (s, "hola", "3");
		}

		[TestMethod]
		public void ParseStringNonNamespace ()
		{
			// Can't get away with not setting the namespace
			Assert.Throws<XamlParseException> (delegate { XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
						   xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
 						   xmlns:moon=""assembly=mscorlib""><Canvas.Resources><moon:String x:Key=""hola"">hola</moon:String></Canvas.Resources></Canvas>"); }, "1");
		}

		[TestMethod]
		public void ParseStringNamespaceAssemblyReversed ()
		{
			// Can't get away with not setting the namespace
			Assert.Throws<XamlParseException> (delegate { XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
						   xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
 						   xmlns:moon=""assembly=mscorlib;clr-namespace:System""><Canvas.Resources><moon:String x:Key=""hola"">hola</moon:String></Canvas.Resources></Canvas>"); }, "1");
		}

		[TestMethod]
		public void ParseInt ()
		{
			Canvas c;
			int i;

			c = (Canvas) XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
						   xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
 						   xmlns:sys=""clr-namespace:System;assembly=mscorlib""><Canvas.Resources><sys:Int32 x:Key=""int"">39</sys:Int32></Canvas.Resources></Canvas>");

			
			Assert.AreEqual (1, c.Resources.Count, "1");
			Assert.IsNotNull (c.Resources ["int"], "2");

			i = (int) c.Resources ["int"];
			Assert.AreEqual (i, 39, "3");
		}

		[TestMethod]
		public void ParseNegativeInt ()
		{
			Canvas c;
			int i;

			c = (Canvas) XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
						   xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
 						   xmlns:sys=""clr-namespace:System;assembly=mscorlib""><Canvas.Resources><sys:Int32 x:Key=""int"">-39</sys:Int32></Canvas.Resources></Canvas>");

			
			Assert.AreEqual (1, c.Resources.Count, "1");
			Assert.IsNotNull (c.Resources ["int"], "2");

			i = (int) c.Resources ["int"];
			Assert.AreEqual (i, -39, "3");
		}

		[TestMethod]
		public void ParseEmptyInt ()
		{
			Canvas c;
			int i;

			c = (Canvas) XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
						   xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
 						   xmlns:sys=""clr-namespace:System;assembly=mscorlib""><Canvas.Resources><sys:Int32 x:Key=""int""></sys:Int32></Canvas.Resources></Canvas>");

			
			Assert.AreEqual (1, c.Resources.Count, "1");
			Assert.IsNotNull (c.Resources ["int"], "2");

			i = (int) c.Resources ["int"];
			Assert.AreEqual (i, 0, "3");
		}

		[TestMethod]
		public void ParseIntOnlyWhitespace ()
		{
			Canvas c;
			int i;

			c = (Canvas) XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
						   xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
 						   xmlns:sys=""clr-namespace:System;assembly=mscorlib""><Canvas.Resources><sys:Int32 x:Key=""int""> </sys:Int32></Canvas.Resources></Canvas>");

			
			Assert.AreEqual (1, c.Resources.Count, "1");
			Assert.IsNotNull (c.Resources ["int"], "2");

			i = (int) c.Resources ["int"];
			Assert.AreEqual (i, 0, "3");
		}

		[TestMethod]
		public void ParseDoubleAsInt ()
		{
			Canvas c;
			int i;

			c = (Canvas) XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
						   xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
 						   xmlns:sys=""clr-namespace:System;assembly=mscorlib""><Canvas.Resources><sys:Int32 x:Key=""int"">39.5</sys:Int32></Canvas.Resources></Canvas>");

			
			Assert.AreEqual (1, c.Resources.Count, "1");
			Assert.IsNotNull (c.Resources ["int"], "2");

			i = (int) c.Resources ["int"];
			Assert.AreEqual (i, 39, "3");

			c = (Canvas) XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
						   xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
 						   xmlns:sys=""clr-namespace:System;assembly=mscorlib""><Canvas.Resources><sys:Int32 x:Key=""int"">39.9</sys:Int32></Canvas.Resources></Canvas>");

			
			Assert.AreEqual (1, c.Resources.Count, "4");
			Assert.IsNotNull (c.Resources ["int"], "5");

			// No rounding
			i = (int) c.Resources ["int"];
			Assert.AreEqual (i, 39, "6");
		}

		[TestMethod]
		public void ParseIntAsHex ()
		{
			Canvas c;
			int i;

			c = (Canvas) XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
						   xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
 						   xmlns:sys=""clr-namespace:System;assembly=mscorlib""><Canvas.Resources><sys:Int32 x:Key=""int"">0x39</sys:Int32></Canvas.Resources></Canvas>");

			
			Assert.AreEqual (1, c.Resources.Count, "1");
			Assert.IsNotNull (c.Resources ["int"], "2");

			// NOTE: the hex is not parsed correctly and this just gets a default value
			i = (int) c.Resources ["int"];
			Assert.AreEqual (i, 0, "3");
		}

		[TestMethod]
		public void ParseIntWithComma ()
		{
			Canvas c;
			int i;

			c = (Canvas) XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
						   xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
 						   xmlns:sys=""clr-namespace:System;assembly=mscorlib""><Canvas.Resources><sys:Int32 x:Key=""int"">39,000</sys:Int32></Canvas.Resources></Canvas>");

			
			Assert.AreEqual (1, c.Resources.Count, "1");
			Assert.IsNotNull (c.Resources ["int"], "2");

			// NOTE: Comma is not handled correctly
			i = (int) c.Resources ["int"];
			Assert.AreEqual (i, 39, "3");
		}

		[TestMethod]
		public void ParseIntWithSpace ()
		{
			Canvas c;
			int i;

			c = (Canvas) XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
						   xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
 						   xmlns:sys=""clr-namespace:System;assembly=mscorlib""><Canvas.Resources><sys:Int32 x:Key=""int"">39 000</sys:Int32></Canvas.Resources></Canvas>");

			
			Assert.AreEqual (1, c.Resources.Count, "1");
			Assert.IsNotNull (c.Resources ["int"], "2");

			// NOTE: Space is not handled correctly
			i = (int) c.Resources ["int"];
			Assert.AreEqual (i, 39, "3");
		}

		[TestMethod]
		public void ParseIntBrokenHex ()
		{
			Canvas c;
			int i;

			c = (Canvas) XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
						   xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
 						   xmlns:sys=""clr-namespace:System;assembly=mscorlib""><Canvas.Resources><sys:Int32 x:Key=""int"">1x10</sys:Int32></Canvas.Resources></Canvas>");

			
			Assert.AreEqual (1, c.Resources.Count, "1");
			Assert.IsNotNull (c.Resources ["int"], "2");

			// NOTE: This isn't handled properly and sets the value to the first int found
			i = (int) c.Resources ["int"];
			Assert.AreEqual (i, 1, "3");
		}

		[TestMethod]
		public void ParseDouble ()
		{
			Canvas c;
			double d;

			c = (Canvas) XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
						   xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
 						   xmlns:sys=""clr-namespace:System;assembly=mscorlib""><Canvas.Resources><sys:Double x:Key=""double"">39.0</sys:Double></Canvas.Resources></Canvas>");

			
			Assert.AreEqual (1, c.Resources.Count, "1");
			Assert.IsNotNull (c.Resources ["double"], "2");

			d = (double) c.Resources ["double"];
			Assert.AreEqual (d, 39.0, "3");
		}

		[TestMethod]
		public void ParseNegativeDouble ()
		{
			Canvas c;
			double d;

			c = (Canvas) XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
						   xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
 						   xmlns:sys=""clr-namespace:System;assembly=mscorlib""><Canvas.Resources><sys:Double x:Key=""double"">-39.0</sys:Double></Canvas.Resources></Canvas>");

			
			Assert.AreEqual (1, c.Resources.Count, "1");
			Assert.IsNotNull (c.Resources ["double"], "2");

			d = (double) c.Resources ["double"];
			Assert.AreEqual (d, -39.0, "3");
		}

		[TestMethod]
		public void ParseIntAsDouble ()
		{
			Canvas c;
			double d;

			c = (Canvas) XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
						   xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
 						   xmlns:sys=""clr-namespace:System;assembly=mscorlib""><Canvas.Resources><sys:Double x:Key=""double"">39</sys:Double></Canvas.Resources></Canvas>");

			
			Assert.AreEqual (1, c.Resources.Count, "1");
			Assert.IsNotNull (c.Resources ["double"], "2");

			d = (double) c.Resources ["double"];
			Assert.AreEqual (d, 39.0, "3");
		}

		[TestMethod]
		[MoonlightBug ("Moonlight doesn't permit empty doubles (perhaps because of the plugin?)")]
		public void ParseEmptyDouble ()
		{
			Canvas c;
			double d;

			c = (Canvas) XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
						   xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
 						   xmlns:sys=""clr-namespace:System;assembly=mscorlib""><Canvas.Resources><sys:Double x:Key=""double""></sys:Double></Canvas.Resources></Canvas>");

			
			Assert.AreEqual (1, c.Resources.Count, "1");
			Assert.IsNotNull (c.Resources ["double"], "2");

			d = (double) c.Resources ["double"];
			Assert.AreEqual (d, 0.0, "3");
		}

		[TestMethod]
		public void ParseDoubleWithComma ()
		{
			Assert.Throws<XamlParseException> (delegate { XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
						   xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
 						   xmlns:sys=""clr-namespace:System;assembly=mscorlib""><Canvas.Resources><sys:Double x:Key=""double"">39,000</sys:Double></Canvas.Resources></Canvas>"); }, "1");
		}

		[TestMethod]
		public void ParseDoubleWithSpace ()
		{
			Assert.Throws<XamlParseException> (delegate { XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
						   xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
 						   xmlns:sys=""clr-namespace:System;assembly=mscorlib""><Canvas.Resources><sys:Double x:Key=""double"">39 000</sys:Double></Canvas.Resources></Canvas>"); }, "1");
		}

		[TestMethod]
		[MoonlightBug ("Moonlight permits doubles in hex format (thanks to strtod)")]
		public void ParseDoubleHex ()
		{
			Assert.Throws<XamlParseException> (delegate { XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
						   xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
 						   xmlns:sys=""clr-namespace:System;assembly=mscorlib""><Canvas.Resources><sys:Double x:Key=""double"">0x10</sys:Double></Canvas.Resources></Canvas>"); }, "1");
		}

		[TestMethod]
		public void ParseChar ()
		{
			Assert.Throws<XamlParseException> (delegate { XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
						   xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
 						   xmlns:sys=""clr-namespace:System;assembly=mscorlib""><Canvas.Resources><sys:Char x:Key=""char"">c</sys:Char></Canvas.Resources></Canvas>"); }, "1");

		}

		[TestMethod]
		public void ParseFloat ()
		{
			Assert.Throws<XamlParseException> (delegate { XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
						   xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
 						   xmlns:sys=""clr-namespace:System;assembly=mscorlib""><Canvas.Resources><sys:Float x:Key=""float"">c</sys:Float></Canvas.Resources></Canvas>"); }, "1");
		}
	}

}

