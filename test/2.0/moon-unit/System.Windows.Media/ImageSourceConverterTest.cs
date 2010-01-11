using System;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Windows.Media.Imaging;

namespace MoonTest.System.Windows.Media {
	
	[TestClass]
	public class ImageSourceConverterTest {

		[TestMethod]
		public void ConvertFromInt ()
		{
			ImageSourceConverter c = new ImageSourceConverter ();
			Assert.Throws<NotImplementedException>(() => c.ConvertFrom (5), "#1");
		}

		[TestMethod]
		public void ConvertFromNull ()
		{
			ImageSourceConverter c = new ImageSourceConverter ();
			Assert.Throws<NotImplementedException> (() => c.ConvertFrom (null), "#1");
		}

		[TestMethod]
		public void ConvertFromString ()
		{
			ImageSourceConverter c = new ImageSourceConverter ();
			var converted = c.ConvertFrom ("string");
			Assert.IsInstanceOfType<BitmapImage> (converted, "#1");
			Assert.AreEqual (new Uri ("string", UriKind.Relative), ((BitmapImage) converted).UriSource, "#2");
		}

		[TestMethod]
		public void ConvertFromUri ()
		{
			Uri uri = new Uri ("tester.com", UriKind.Relative);
			ImageSourceConverter c = new ImageSourceConverter ();
			var converted = c.ConvertFrom (uri);
			Assert.IsInstanceOfType<BitmapImage> (converted, "#1");
			Assert.AreEqual (uri, ((BitmapImage) converted).UriSource, "#2");
		}
	}
}