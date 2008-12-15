using System;
using System.Diagnostics;
using System.IO;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.Silverlight.Testing;

namespace MoonTest.System.Windows.Controls
{
	[TestClass]
	public partial class MediaElementTest : Microsoft.Silverlight.Testing.SilverlightTest
	{
		MediaElement media = new MediaElement ();
		WebClient client = new WebClient ();
		bool completed = false;

		[TestMethod]
		[MoonlightBug]
		public void NonWorkingDefaults()
		{
			MediaElement m = new MediaElement ();
			// Should never get an abstract class from unmanaged code MEDIAATTRIBUTE_COLLECTION
			Assert.IsNotNull (m.Attributes, "Attributes");
			// InvalidCastException
			Assert.AreEqual(MediaElementState.Closed, m.CurrentState, "CurrentState");
			// NotImplementedException
			Assert.IsNotNull (m.LicenseAcquirer, "LicenseAcquirer");
		}

		[TestMethod]
		public void TestDefaults()
		{
			MediaElement m = new MediaElement ();
//			Assert.IsNotNull (m.Attributes, "Attributes");
			Assert.AreEqual (0, m.AudioStreamCount, "AudioStreamCount");
			Assert.IsNull (m.AudioStreamIndex, "AudioStreamIndex");
			Assert.IsTrue (m.AutoPlay, "AutoPlay");
			Assert.AreEqual (0, m.Balance, "Balance");
			Assert.AreEqual (0, m.BufferingProgress, "BufferingProgress");
			Assert.IsFalse (m.CanPause, "CanPause");
			Assert.IsFalse (m.CanSeek, "CanSeek");
//			Assert.AreEqual(MediaElementState.Closed, m.CurrentState, "CurrentState");
			Assert.AreEqual (0, m.DownloadProgress, "DownloadProgress");
			Assert.AreEqual (0, m.DownloadProgressOffset, "DownloadProgressOffset");
			Assert.AreEqual (0, m.DroppedFramesPerSecond, "DroppedFramesPerSecond");
			Assert.IsFalse (m.IsMuted, "IsMuted");
//			Assert.IsNotNull (m.LicenseAcquirer, "LicenseAcquirer");
			Assert.IsNotNull (m.Markers, "Markers");
			Assert.AreEqual (0, m.Markers.Count, "Markers.Count");
			Assert.AreEqual (new Duration(TimeSpan.Zero), m.NaturalDuration, "NaturalDuration");
			Assert.AreEqual (0, m.NaturalVideoHeight, "NaturalVideoHeight");
			Assert.AreEqual (0, m.NaturalVideoWidth, "NaturalVideoWidth");
			Assert.AreEqual (TimeSpan.Zero, m.Position, "Position");
			Assert.AreEqual (0, m.RenderedFramesPerSecond, "RenderedFramesPerSecond");
			Assert.IsNull (m.Source, "Source");
			Assert.AreEqual (Stretch.Uniform, m.Stretch, "Stretch");
			Assert.AreEqual (0.5, m.Volume, "Volume");

			FrameworkElementTest.CheckDefaultProperties (m);
		}

		[TestMethod]
		[MoonlightBug]
		public void TestInvalidValues()
		{
			MediaElement m = new MediaElement();

			m.AudioStreamIndex = -1;
			Assert.IsNull(m.AudioStreamIndex);
			m.AudioStreamIndex = int.MaxValue;
			Assert.AreEqual(int.MaxValue, m.AudioStreamIndex);
			m.Balance = -10000;
			m.Balance = 10000;
			m.BufferingTime = TimeSpan.FromSeconds(-1000);
			Assert.Throws<Exception>(delegate { m.BufferingTime = TimeSpan.MaxValue; });
			m.BufferingTime = TimeSpan.FromSeconds(1000);
			Assert.Throws<ArgumentNullException>(delegate { m.LicenseAcquirer = null; });
			m.Position = TimeSpan.FromSeconds(-100);
			Assert.AreEqual(TimeSpan.Zero, m.Position);
			m.Position = TimeSpan.FromSeconds(1000000);
			Assert.AreEqual(TimeSpan.Zero, m.Position);
			m.Source = null;
			m.Volume = -1000;
			m.Volume = 10000;
		}

		[TestMethod]
		[Asynchronous ()]
		public void SetSourceTest () 
		{
			client.DownloadProgressChanged += new DownloadProgressChangedEventHandler (client_DownloadProgressChanged);
			client.OpenReadCompleted += new OpenReadCompletedEventHandler (client_OpenReadCompleted);
			client.OpenReadAsync (new Uri ("http://localhost:8080/elephants-dream-320x180-first-minute.wmv", UriKind.Absolute));
			EnqueueConditional (delegate () { return completed; });
			EnqueueTestComplete ();
		}
		
		[TestMethod]
		[Ignore ("we can't run this test on windows, FileStream.ctor is unavailable.")]
		public void SetSourceTest2 ()
		{
			MediaElement media = new MediaElement ();
			Stream s = new FileStream ("/mono/main/src/ml2/test/media/video/elephants-dream-320x180-first-minute.wmv", FileMode.Open);
			media.SetSource (s);
		}

		void client_DownloadProgressChanged (object sender, DownloadProgressChangedEventArgs e) {
			Debug.WriteLine ("client_DownloadProgressChanged: " + e.ProgressPercentage.ToString ());
		}

		void client_OpenReadCompleted (object sender, OpenReadCompletedEventArgs e) {
			completed = true;
			try {
				// Accessing e.Result causes a security exception in SL which makes the harness stop working
				// if we don't handle the exception 
				Console.WriteLine ("client_OpenRead: e.Result: {0}, e.ex: {1}", e.Result, e.Error);
				if (e.Result != null)
					media.SetSource (new SlowStream (e.Result));
			} catch (Exception ex) {
				Console.WriteLine (ex.Message);
			}
		}
	}
}
