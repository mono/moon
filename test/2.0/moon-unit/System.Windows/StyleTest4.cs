using System;
using System.ComponentModel;
using System.Globalization;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Markup;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Collections.Generic;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows
{
	[TestClass]
	public class StyleTest4_TypeConverterOnProperty_MisnamedProperty {

		public class HappyStructConverter : TypeConverter {
			public override bool CanConvertFrom (ITypeDescriptorContext context, Type sourceType)
			{
				return sourceType == typeof(string);
			}

			public override object ConvertFrom (ITypeDescriptorContext context, CultureInfo culture, object value)
			{
				// don't actually parse the string, just return a new HappyStruct
				return new HappyStruct (5, 5);
			}
		}

		public struct HappyStruct {
			public HappyStruct (int _x, int _y)
			{
				x = _x;
				y = _y;
			}

			public int X { 
				get { return x; }
				set { x = value; }
			}

			public int Y {
				get { return y; }
				set { y = value; }
			}

			public override string ToString ()
			{
				return string.Format ("Happy ({0},{1})", x, y);
			}

			private int x;
			private int y;
		}

		public class HappyButton : Button {
			public static readonly DependencyProperty HappyProperty = DependencyProperty.Register ("Happpy", typeof (HappyStruct),
													       typeof (HappyButton),
													       null);

			[TypeConverter (typeof (HappyStructConverter))]
			public HappyStruct Happy {
				get { return (HappyStruct)GetValue (HappyButton.HappyProperty); }
				set { SetValue (HappyButton.HappyProperty, value); }
			}
		}

		[TestMethod]
		public void TestWithValue ()
		{
			HappyButton b = new HappyButton ();
			Style s = new Style (typeof (HappyButton));

			s.Setters.Add (new Setter (HappyButton.HappyProperty, new HappyStruct (10, 10)));

			b.Style = s;

			Assert.AreEqual (new HappyStruct (10, 10), b.Happy);
		}

		[TestMethod]
		[MoonlightBug]
		public void TestWithString ()
		{
			HappyButton b = new HappyButton ();
			Style s = new Style (typeof (HappyButton));

			s.Setters.Add (new Setter (HappyButton.HappyProperty, "just to invoke the type converter"));

			Assert.Throws<XamlParseException> (delegate {
				b.Style = s;
			});
		}
	}
}