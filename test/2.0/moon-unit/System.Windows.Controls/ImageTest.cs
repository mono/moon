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
	public partial class ImageTest : Microsoft.Silverlight.Testing.SilverlightTest
	{
		[TestMethod]
		public void MeasureTest_Empty ()
		{
			Image image = new Image ();
			
			image.Measure (new Size (50,50));

			Assert.AreEqual (new Size (0,0), image.DesiredSize);
			Assert.AreEqual (0, image.ActualWidth);
			Assert.AreEqual (0, image.ActualHeight);
		}

		[TestMethod]
		public void MeasureTest ()
		{
			Image image = new Image ();

			image.Source = new BitmapImage (new Uri ("images/mono.png", UriKind.Relative));

			image.Measure (new Size (500,500));

			Assert.AreEqual (new Size (0,0), image.DesiredSize);
			Assert.AreEqual (0, image.ActualWidth);
			Assert.AreEqual (0, image.ActualHeight);

		}

		[TestMethod]
		public void MeasureTest2 ()
		{
			Image image = new Image ();
			image.Width = 50;
			image.Height = 50;
			image.Source = new BitmapImage (new Uri ("images/mono.png", UriKind.Relative));
			
			image.Measure (new Size (500,500));

			Assert.AreEqual (new Size (0,0), image.DesiredSize);
			Assert.AreEqual (50, image.ActualWidth);
			Assert.AreEqual (50, image.ActualHeight);

		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void LoadTest ()
		{
			// Fails in Silverlight 3
			bool loaded = false;
			bool failed = false;
			double progress = 0.0;

			Image image = new Image ();
			BitmapImage bit = new BitmapImage (new Uri ("http://planet.gnome.org/heads/miguel.png"));
			bit.DownloadProgress += (sender, args) => { progress = args.Progress; };
			image.Source = bit;
			
			image.Loaded += (sender, args) => { loaded = true; };
			image.ImageFailed += (sender, args) => { failed = true; };

			var border = new Border { Child = image, Width = 128, Height = 300 };

			Enqueue (() => TestPanel.Children.Add (border));
			EnqueueConditional (() => (loaded && progress >= 100) || failed );
			Enqueue (() => {
				Assert.IsFalse (failed, "failed");
				Assert.AreEqual (100, progress, "progress");
				
				//				Assert.AreEqual (DependencyProperty.UnsetValue, image.ReadLocalValue (FrameworkElement.ActualWidthProperty), "local actual.width");
				//Assert.AreEqual (DependencyProperty.UnsetValue, image.ReadLocalValue (FrameworkElement.ActualHeightProperty), "local actual.height");

				Assert.AreEqual (new Size (128, 192), new Size (image.ActualWidth, image.ActualHeight), "actual");
				Assert.AreEqual (new Size (128, 192), image.DesiredSize, "desired");
				Assert.AreEqual (true, Double.IsNaN (image.Width), "specified.width");
				Assert.AreEqual (true, Double.IsNaN (image.Height), "specified.height");
				//Assert.AreEqual (new Size (Double.NaN, Double.NaN), new Size (image.Width, image.Height), "specified");
			});
			
			Enqueue (() => TestPanel.Children.Clear ());
			EnqueueTestComplete ();
		} 
		
		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void LoadTestCanvas ()
		{
			// Fails in Silverlight 3
			bool loaded = false;
			bool failed = false;
			double progress = 0.0;
			
			Image image = new Image ();
			BitmapImage bit = new BitmapImage (new Uri ("http://planet.gnome.org/heads/miguel.png"));
			bit.DownloadProgress += (sender, args) => { progress = args.Progress; };
			image.Source = bit;
			
			image.Loaded += (sender, args) => { loaded = true; };
			image.ImageFailed += (sender, args) => { failed = true; };
			
			Canvas c = new Canvas ();
			c.Children.Add (image);
			
			Enqueue (() => TestPanel.Children.Add (c));
			EnqueueConditional (() => (loaded && progress >= 100) || failed );
			Enqueue (() => {
				Assert.IsFalse (failed, "failed");
				Assert.AreEqual (100, progress, "progress");

				//				Assert.AreEqual (DependencyProperty.UnsetValue, image.ReadLocalValue (FrameworkElement.ActualWidthProperty), "local actual.width");
				//Assert.AreEqual (DependencyProperty.UnsetValue, image.ReadLocalValue (FrameworkElement.ActualHeightProperty), "local actual.height");

				Assert.AreEqual (new Size (64, 96), new Size (image.ActualWidth, image.ActualHeight), "actual");

				Assert.AreEqual (new Size (0, 0), image.DesiredSize, "desired");
				Assert.AreEqual (true, Double.IsNaN (image.Width), "specified.width");
				Assert.AreEqual (true, Double.IsNaN (image.Height), "specified.height");
				//Assert.AreEqual (new Size (Double.NaN, Double.NaN), new Size (image.Width, image.Height), "specified");
				
				image.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
				
				Assert.AreEqual (new Size (0, 0), image.DesiredSize, "desired");
				Assert.AreEqual (new Size (64, 96), new Size (image.ActualWidth, image.ActualHeight), "actual");
			});
			
			Enqueue (() => TestPanel.Children.Clear ());
			EnqueueTestComplete ();
		} 
		
		[TestMethod]
		public void ComputeActualWidth ()
		{
			var c = new Image ();

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c actual1");

			c.MaxWidth = 25;
			c.Width = 50;
			c.MinHeight = 33;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired");
			Assert.AreEqual (new Size (25,33), new Size (c.ActualWidth,c.ActualHeight), "c actual1");

			c.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired");
			Assert.AreEqual (new Size (25,33), new Size (c.ActualWidth,c.ActualHeight), "c actual2");
		}

		[TestMethod]
		public void LoadTwiceDifferentInstance ()
		{
			string xaml = @"<Image xmlns=""http://schemas.microsoft.com/client/2007"" Width=""100"" Height=""100"" />";
			Image i1 = (Image) XamlReader.Load (xaml);
			Image i2 = (Image) XamlReader.Load (xaml);
			Assert.AreNotSame (i1, i2, "Load twice");
		}

		[TestMethod]
		public void FoundTwiceSameInstance ()
		{
			string xaml = @"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
<Image x:Name=""a"" Width=""100"" Height=""100"" />
</Canvas>";
			Canvas c = (Canvas) XamlReader.Load (xaml);
			Image i1 = (Image) c.FindName ("a");
			Image i2 = (Image) c.FindName ("a");
			Assert.AreSame (i1, i2, "Found twice");
		}
	}
}
