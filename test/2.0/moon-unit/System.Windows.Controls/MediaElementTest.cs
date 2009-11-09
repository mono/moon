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
using System.Windows.Browser;

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
		public void TestInvalidValues()
		{
			MediaElement m = new MediaElement();
			Assert.Throws<Exception>(delegate {
				m.SetValue (MediaElement.AttributesProperty, null);
			}, "#1"); // Fails in Silverlight 3 (got InvalidOperationException)
			m.AudioStreamIndex = -1000;
			Assert.AreEqual (-1000, m.AudioStreamIndex, "#2");
			m.AudioStreamIndex = -1;
			Assert.IsNull(m.AudioStreamIndex, "#3");
			m.AudioStreamIndex = int.MaxValue;
			Assert.AreEqual(int.MaxValue, m.AudioStreamIndex, "#4");
			m.Balance = -10000;
			m.Balance = 10000;
			m.BufferingTime = TimeSpan.FromSeconds(-1000);
			Assert.Throws<Exception>(delegate { m.BufferingTime = TimeSpan.MaxValue; }, "#5");
			m.BufferingTime = TimeSpan.FromSeconds(1000);
			Assert.Throws<ArgumentNullException>(delegate { m.LicenseAcquirer = null; }, "#6");
			m.Position = TimeSpan.FromSeconds(-100);
			Assert.AreEqual(TimeSpan.Zero, m.Position, "#7");
			m.Position = TimeSpan.FromSeconds(1000000);
			Assert.AreEqual(TimeSpan.Zero, m.Position, "#8");
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
		public void SetSource_Stream_Null ()
		{
			MediaElement media = new MediaElement ();
			Assert.IsNull (media.Source, "Source-1");

			Assert.Throws<ArgumentNullException> (delegate {
				media.SetSource ((Stream) null);
			}, "null");
			Assert.IsNull (media.Source, "Source-2");

			media.Source = new Uri ("thisfinedoesnotexist.wmv", UriKind.Relative);
			Assert.IsNotNull (media.Source, "Source-3");

			Assert.Throws<ArgumentNullException> (delegate {
				media.SetSource ((Stream) null);
			}, "null");
			Assert.IsNotNull (media.Source, "Source-4");
		}

		[TestMethod]
		public void SetSource_StreamNull ()
		{
			MediaElement media = new MediaElement ();
			Assert.IsNull (media.Source, "Source-1");

			media.SetSource (Stream.Null);
			Assert.IsNull (media.Source, "Source-2");

			media.Source = new Uri ("thisfinedoesnotexist.wmv", UriKind.Relative);
			Assert.IsNotNull (media.Source, "Source-3");

			media.SetSource (Stream.Null);
			Assert.IsNull (media.Source, "Source-4");
		}

		[TestMethod]
		public void SetSource_MediaStreamSouce_Null ()
		{
			MediaElement media = new MediaElement ();
			Assert.IsNull (media.Source, "Source-1");

			Assert.Throws<ArgumentNullException> (delegate {
				media.SetSource ((MediaStreamSource) null);
			}, "null");
			Assert.IsNull (media.Source, "Source-2");

			media.Source = new Uri ("thisfinedoesnotexist.wmv", UriKind.Relative);
			Assert.IsNotNull (media.Source, "Source-3");

			Assert.Throws<ArgumentNullException> (delegate {
				media.SetSource ((MediaStreamSource) null);
			}, "null");
			Assert.IsNotNull (media.Source, "Source-4");
		}

		[TestMethod]
		[Asynchronous ()]
		public void MediaFailedState ()
		{
			bool failed = false;
			MediaElement mel = new MediaElement ();
			mel.Name = "mel";			
			TestPanel.Children.Add (mel);
			mel.MediaFailed += new EventHandler<ExceptionRoutedEventArgs>(delegate (object sender, ExceptionRoutedEventArgs e) {
				Assert.AreEqual (MediaElementState.Closed, mel.CurrentState, "CurrentState in MediaFailed.");
				failed = true;
			});

			mel.Source = new Uri ("thisfiledoesnotexist.wmv", UriKind.Relative);

			EnqueueConditional (() => failed);
			Enqueue (delegate ()
			{
				Assert.AreEqual (MediaElementState.Closed, mel.CurrentState, "CurrentState after MediaFailed.");
			});
			Enqueue (delegate ()
			{
				Assert.AreEqual ("Closed", HtmlPage.Window.Eval ("document.getElementById ('silverlight').content.root.findName ('mel').CurrentState"), "CurrentState in js");
			});
			EnqueueTestComplete ();
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

		[TestMethod]
		[MoonlightBug ("other readonly properties are throwing InvalidOperationException instead of ArgumentException")]
		public void CheckReadOnlyProperties ()
		{
			MediaElement m = new MediaElement ();

			Assert.AreEqual (0, (int) m.GetValue (MediaElement.AudioStreamCountProperty), "Get/AudioStreamCountProperty");
			Assert.Throws<ArgumentException> (delegate {
				m.SetValue (MediaElement.AudioStreamCountProperty, 1);
			});
			Assert.AreEqual (0, m.AudioStreamCount, "AudioStreamCount");

			Assert.AreEqual (0.0, (double) m.GetValue (MediaElement.BufferingProgressProperty), "Get/BufferingProgressProperty");
			Assert.Throws<ArgumentException> (delegate {
				m.SetValue (MediaElement.BufferingProgressProperty, 1.0);
			});
			Assert.AreEqual (0.0, m.BufferingProgress, "BufferingProgress");

			Assert.IsFalse ((bool) m.GetValue (MediaElement.CanPauseProperty), "Get/CanPauseProperty");
			Assert.Throws<ArgumentException> (delegate {
				m.SetValue (MediaElement.CanPauseProperty, true);
			});
			Assert.IsFalse (m.CanPause, "CanPause");

			Assert.IsFalse ((bool) m.GetValue (MediaElement.CanSeekProperty), "Get/CanSeekProperty");
			Assert.Throws<ArgumentException> (delegate {
				m.SetValue (MediaElement.CanSeekProperty, true);
			});
			Assert.IsFalse (m.CanSeek, "CanSeek");

			Assert.AreEqual (MediaElementState.Closed, (MediaElementState) m.GetValue (MediaElement.CurrentStateProperty), "Get/CurrentState");
			Assert.Throws<ArgumentException> (delegate {
				m.SetValue (MediaElement.CurrentStateProperty, MediaElementState.Stopped);
			});
			Assert.AreEqual (MediaElementState.Closed, m.CurrentState, "CurrentState");

			Assert.AreEqual (0.0, (double) m.GetValue (MediaElement.DownloadProgressOffsetProperty), "Get/DownloadProgressOffsetProperty");
			Assert.Throws<ArgumentException> (delegate {
				m.SetValue (MediaElement.DownloadProgressOffsetProperty, 1.0);
			});
			Assert.AreEqual (0.0, m.DownloadProgressOffset, "DownloadProgressOffset");

			Assert.AreEqual (0.0, (double) m.GetValue (MediaElement.DroppedFramesPerSecondProperty), "Get/DroppedFramesPerSecondProperty");
			Assert.Throws<ArgumentException> (delegate {
				m.SetValue (MediaElement.DroppedFramesPerSecondProperty, 1.0);
			});
			Assert.AreEqual (0.0, m.DroppedFramesPerSecond, "DroppedFramesPerSecond");

			Assert.AreEqual (0, ((Duration) m.GetValue (MediaElement.NaturalDurationProperty)).TimeSpan.Ticks, "Get/NaturalDurationProperty");
			Assert.Throws<ArgumentException> (delegate {
				m.SetValue (MediaElement.NaturalDurationProperty, Duration.Forever);
			});
			Assert.AreEqual (0.0, m.NaturalDuration.TimeSpan.Ticks, "NaturalDuration");

			Assert.AreEqual (0, (int) m.GetValue (MediaElement.NaturalVideoHeightProperty), "Get/NaturalVideoHeightProperty");
			Assert.Throws<ArgumentException> (delegate {
				m.SetValue (MediaElement.NaturalVideoHeightProperty, 1);
			});
			Assert.AreEqual (0, m.NaturalVideoHeight, "NaturalVideoHeight");

			Assert.AreEqual (0, (int) m.GetValue (MediaElement.NaturalVideoWidthProperty), "Get/NaturalVideoWidthProperty");
			Assert.Throws<ArgumentException> (delegate {
				m.SetValue (MediaElement.NaturalVideoWidthProperty, 1);
			});
			Assert.AreEqual (0, m.NaturalVideoWidth, "NaturalVideoWidth");

			Assert.AreEqual (0.0, (double) m.GetValue (MediaElement.RenderedFramesPerSecondProperty), "Get/RenderedFramesPerSecondProperty");
			Assert.Throws<ArgumentException> (delegate {
				m.SetValue (MediaElement.RenderedFramesPerSecondProperty, 1.0);
			});
			Assert.AreEqual (0.0, m.RenderedFramesPerSecond, "RenderedFramesPerSecond");
		}
	}
}
