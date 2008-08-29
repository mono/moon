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

namespace MoonTest.System.Windows.Media
{
	[TestClass]
	public class ColorTest
	{

		[TestMethod]
		[Ignore ("Crashes moonlight at the moment")]
		public void ParseValidColor ()
		{
			Assert.Throws (delegate { XamlReader.Load (@"<Color xmlns=""http://schemas.microsoft.com/client/2007"">#ffffff</Color>");
				},
				typeof (XamlParseException));
		}
	}
}