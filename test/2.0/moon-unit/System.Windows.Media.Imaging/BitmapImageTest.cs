using System;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;

using Mono.Moonlight.UnitTesting;

// FIXME: the exceptions shouldn't be NRE's, but I'm not sure what
// they will be so amde them something they won't be so that the
// Assert fails so I can fix them.

namespace MoonTest.System.Windows.Media.Imaging {
	[TestClass]
	public class BitmapImageTest {
		static Uri corruptImage = new Uri ("images/invalid-image-data.png");
		static Uri badUri = new Uri ("non-existent-uri.png");
		
		// Invalid/bad Uri
		
		[TestMethod]
		public void BadUriInCtor ()
		{
			Assert.Throws (delegate {
				BitmapImage bitmap = new BitmapImage (badUri);
			}, typeof (NullReferenceException));
		}
		
		[TestMethod]
		public void BadUriSetUriSource ()
		{
			Assert.Throws (delegate {
				BitmapImage bitmap = new BitmapImage ();
				bitmap.UriSource = badUri;
			}, typeof (NullReferenceException));
		}
		
		[TestMethod]
		public void BadUriSetUriSourceProperty ()
		{
			Assert.Throws (delegate {
				BitmapImage bitmap = new BitmapImage ();
				bitmap.SetValue (BitmapImage.UriSourceProperty, badUri);
			}, typeof (NullReferenceException));
		}
		
		
		// Corrupt image
		
		[TestMethod]
		public void CorruptImageInCtor ()
		{
			Assert.Throws (delegate {
				BitmapImage bitmap = new BitmapImage (corruptImage);
			}, typeof (NullReferenceException));
		}
		
		[TestMethod]
		public void CorruptImageSetUriSource ()
		{
			Assert.Throws (delegate {
				BitmapImage bitmap = new BitmapImage ();
				bitmap.UriSource = corruptImage;
			}, typeof (NullReferenceException));
		}
		
		[TestMethod]
		public void CorruptImageSetUriSourceProperty ()
		{
			Assert.Throws (delegate {
				BitmapImage bitmap = new BitmapImage ();
				bitmap.SetValue (BitmapImage.UriSourceProperty, corruptImage);
			}, typeof (NullReferenceException));
		}
		
		
	}
}
