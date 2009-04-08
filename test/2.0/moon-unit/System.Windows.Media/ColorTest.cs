//
// Unit tests for Color
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2009 Novell, Inc (http://www.novell.com)
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

using System;
using System.Windows;
using System.Windows.Markup;
using System.Windows.Media;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Media {

	[TestClass]
	public class ColorTest {

		[TestMethod]
		public void Defaults ()
		{
			Color c = new Color ();
			Assert.AreEqual (0, c.R, "R");
			Assert.AreEqual (0, c.G, "G");
			Assert.AreEqual (0, c.B, "B");
			Assert.AreEqual (0, c.A, "A");
			Assert.AreEqual ("#00000000", c.ToString (), "ToString");
		}

		[TestMethod]
		public void ParseValidColorAsTopLevel ()
		{
			Assert.Throws<XamlParseException> (delegate {
				XamlReader.Load (@"<Color xmlns=""http://schemas.microsoft.com/client/2007"">#ffffff</Color>");
			}, "bad xaml");
		}

		[TestMethod]
		public void ParseValidColorAsResource ()
		{
			XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><Color x:Name=""color"">#ffffff</Color></Canvas.Resources></Canvas>");
		}

		[TestMethod]
		public void ParseValidColorAsContentProperty ()
		{
			XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007""><Canvas.Background><SolidColorBrush><Color>#ffffff</Color></SolidColorBrush></Canvas.Background></Canvas>");
		}

		[TestMethod]
		public void ParseValidColorAsProperty ()
		{
			XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007""><Canvas.Background><SolidColorBrush><SolidColorBrush.Color><Color>#ffffff</Color></SolidColorBrush.Color></SolidColorBrush></Canvas.Background></Canvas>");
		}

		class ColorFormatter : IFormatProvider, ICustomFormatter {

			public object GetFormat (Type formatType)
			{
				return (formatType == typeof (ICustomFormatter)) ? this : null;
			}

			public string Format (string format, object arg, IFormatProvider formatProvider)
			{
				CallCount++;
				Assert.AreEqual (this, formatProvider, "formatProvider");
				if (arg.Equals (','))
					return "#";

				Assert.IsTrue (arg is byte, "arg");
				int n = (byte) arg;
				switch (n) {
				case 1:
				case 2:
				case 3:
				case 4:
				case 11:
				case 12:
				case 13:
				case 14:
					if (String.IsNullOrEmpty (format))
						return "@";
					Assert.AreEqual ("X2", format, n.ToString ());
					return String.Format ("[{0}]", n);
				default:
					Assert.Fail (n.ToString ());
					return null;
				}
			}

			static public int CallCount = 0;
		}

		[TestMethod]
		public void ToStringIFormatProvider ()
		{
			Color c = new Color ();
			c.A = 1;
			c.R = 2;
			c.G = 3;
			c.B = 4;
			ColorFormatter.CallCount = 0;
			Assert.AreEqual ("#01020304", c.ToString (null), "null");
			Assert.AreEqual (0, ColorFormatter.CallCount, "CallCount-a");
			Assert.AreEqual ("#[1][2][3][4]", c.ToString (new ColorFormatter ()), "ColorFormatter");
			// 4 times for each byte
			Assert.AreEqual (4, ColorFormatter.CallCount, "CallCount");
		}

		[TestMethod]
		public void ToStringIFormattable ()
		{
			Color c = new Color ();
			c.A = 11;
			c.R = 12;
			c.G = 13;
			c.B = 14;
			ColorFormatter.CallCount = 0;
			IFormattable f = (c as IFormattable);
			Assert.AreEqual ("#0B0C0D0E", f.ToString (null, null), "null,null");
			Assert.AreEqual (0, ColorFormatter.CallCount, "CallCount-a");
			Assert.AreEqual ("#[11][12][13][14]", f.ToString (null, new ColorFormatter ()), "null,ColorFormatter");
			Assert.AreEqual (4, ColorFormatter.CallCount, "CallCount-b");
			Assert.AreEqual ("sc#b, c, d, e", f.ToString ("x1", null), "x1,null");
			Assert.AreEqual (4, ColorFormatter.CallCount, "CallCount-c");
			Assert.AreEqual ("sc#@# @# @# @", f.ToString (String.Empty, new ColorFormatter ()), "Empty,ColorFormatter");
			// 4 times for each byte, 3 times for the ',' character
			Assert.AreEqual (11, ColorFormatter.CallCount, "CallCount-d");
		}
	}
}
