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
		public void ConvertFromString_Absolute ()
		{
			ImageSourceConverter c = new ImageSourceConverter ();
			var converted = c.ConvertFrom ("http://example.com/image.jpg");
			Assert.IsInstanceOfType<BitmapImage> (converted, "#1");
			((BitmapImage) converted).ImageFailed += delegate { /* do nothing */ };

			Uri uri = ((BitmapImage) converted).UriSource;
			Assert.AreEqual (new Uri ("http://example.com/image.jpg", UriKind.Absolute), uri, "#2");
			Assert.IsTrue (uri.IsAbsoluteUri, "#3");
		}

		[TestMethod]
		public void ConvertFromString_Relative ()
		{
			ImageSourceConverter c = new ImageSourceConverter ();
			var converted = c.ConvertFrom ("string");
			Assert.IsInstanceOfType<BitmapImage> (converted, "#1");
			((BitmapImage) converted).ImageFailed += delegate { /* do nothing */ };

			Uri uri = ((BitmapImage) converted).UriSource;
			Assert.AreEqual (new Uri ("string", UriKind.Relative), uri, "#2");
			Assert.IsFalse (uri.IsAbsoluteUri, "#3");
		}

		[TestMethod]
		public void ConvertFromUri ()
		{
			Uri uri = new Uri ("tester.com", UriKind.Relative);
			ImageSourceConverter c = new ImageSourceConverter ();
			var converted = c.ConvertFrom (uri);
			((BitmapImage) converted).ImageFailed += delegate { /* do nothing */ };
		
			Assert.IsInstanceOfType<BitmapImage> (converted, "#1");
			Assert.AreEqual (uri, ((BitmapImage) converted).UriSource, "#2");
		}
	}
}