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
		[Asynchronous]
		[MoonlightBug]
		public void TestDetached ()
		{
			int media_opened_counter = 0;
			int media_ended_counter = 0;
			int media_failed_counter = 0;
			int media_buffering_progress_counter = 0;
			int media_download_progress_counter = 0;
			int media_state_changed_counter = 0;

			MediaElement mel = new MediaElement ();
			TestPanel.Children.Add (mel);

			mel.BufferingProgressChanged += new RoutedEventHandler (delegate (object sender, RoutedEventArgs e)
				{
					Debug.WriteLine ("BufferingProgressChanged");
					media_buffering_progress_counter++;
				});
			mel.CurrentStateChanged += new RoutedEventHandler (delegate (object sender, RoutedEventArgs e)
			{
				Debug.WriteLine ("CurrentStateChanged");
				media_state_changed_counter++;
			});
			mel.DownloadProgressChanged += new RoutedEventHandler (delegate (object sender, RoutedEventArgs e)
			{
				Debug.WriteLine ("DownloadProgressChanged");
				media_download_progress_counter++;
			});
			mel.MediaEnded += new RoutedEventHandler (delegate (object sender, RoutedEventArgs e)
			{
				Debug.WriteLine ("MediaEnded");
				media_ended_counter++;
			});
			mel.MediaFailed += new EventHandler<ExceptionRoutedEventArgs> (delegate (object sender, ExceptionRoutedEventArgs e)
			{
				Debug.WriteLine ("MediaFailed");
				media_failed_counter++;
			});
			mel.MediaOpened += new RoutedEventHandler (delegate (object sender, RoutedEventArgs e)
			{
				Debug.WriteLine ("MediaOpened");
				media_opened_counter++;
			});
			mel.Source = new Uri ("/moon-unit;component/timecode-long-with-audio.wmv", UriKind.Relative);

			// set all properties to non-defaults, to check which ones are reset
			mel.AutoPlay = false;
			mel.Balance = 0.7;
			mel.BufferingTime = TimeSpan.FromSeconds (3.0);
			mel.IsMuted = true;
			mel.Volume = 0.33;

			// wait until media has been opened
			EnqueueConditional (() => media_opened_counter >= 1);

			// check initial values
			Enqueue (delegate ()
			{
				Assert.AreEqual (0.0, mel.Position.TotalMilliseconds, "Initial - Position");
			});

			// play
			Enqueue (delegate ()
			{
				mel.Play ();
			});

			// wait until the media element is playing and has played 1 second
			EnqueueConditional (() => mel.Position.TotalMilliseconds > 1.0);

			// assert attached values
			Enqueue (delegate ()
			{
				Assert.AreEqual (1, media_opened_counter, "Attached - MediaOpenedEvent");
				Assert.AreEqual (0, media_ended_counter, "Attached - MediaEndedEvent");
				Assert.AreEqual (0, media_failed_counter, "Attached - MediaFailedEvent");
				Assert.IsGreater (0, media_buffering_progress_counter, "Attached - BufferingProgressChangedEvent");
				Assert.IsGreater (0, media_download_progress_counter, "Attached - DownloadProgressChangedEvent");
				Assert.IsGreater (0, media_state_changed_counter, "Attached - CurrentStateChangedEvent");
				Assert.AreEqual (false, mel.AutoPlay, "Attached - AutoPlay");
				Assert.AreEqualWithDelta (0.7, mel.Balance, 0.0001, "Attached - Balance");
				Assert.AreEqual (1.0, mel.BufferingProgress, "Attached - BufferingProgress");
				Assert.AreEqual (3.0, mel.BufferingTime.TotalSeconds, "Attached - BufferingTime");
				Assert.AreEqual (true, mel.CanPause, "Attached - CanPause");
				Assert.AreEqual (true, mel.CanSeek, "Attached - CanSeek");
				Assert.AreEqual (MediaElementState.Playing, mel.CurrentState, "Attached - CurrentState");
				Assert.AreEqual (true, mel.IsMuted, "Attached - IsMuted");
				Assert.AreEqual (30033.0, mel.NaturalDuration.TimeSpan.TotalMilliseconds, "Attached - NaturalDuration");
				Assert.IsGreater (1.0, mel.Position.TotalMilliseconds, "Attached - Position");
				Assert.AreEqualWithDelta (0.33, mel.Volume, 0.0001, "Attached - Volume");
			});

			// detach
			Enqueue (delegate ()
			{
				media_opened_counter = 0;
				media_ended_counter = 0;
				media_failed_counter = 0;
				media_buffering_progress_counter = 0;
				media_download_progress_counter = 0;
				media_state_changed_counter = 0;

				TestPanel.Children.Clear ();

				// check what changed immediately
				Assert.AreEqual (0, media_opened_counter, "Detached (immediately) - MediaOpenedEvent");
				Assert.AreEqual (0, media_ended_counter, "Detached (immediately) - MediaEndedEvent");
				Assert.AreEqual (0, media_failed_counter, "Detached (immediately) - MediaFailedEvent");
				Assert.AreEqual (0, media_buffering_progress_counter, "Detached (immediately) - BufferingProgressChangedEvent");
				Assert.AreEqual (0, media_download_progress_counter, "Detached (immediately) - DownloadProgressChangedEvent");
				Assert.AreEqual (0, media_state_changed_counter, "Detached (immediately) - CurrentStateChangedEvent");
				Assert.AreEqual (false, mel.AutoPlay, "Detached (immediately) - AutoPlay");
				Assert.AreEqualWithDelta (0.7, mel.Balance, 0.0001, "Detached (immediately) - Balance");
				Assert.AreEqual (0.0, mel.BufferingProgress, "Detached (immediately) - BufferingProgress");
				Assert.AreEqual (3.0, mel.BufferingTime.TotalSeconds, "Detached (immediately) - BufferingTime");
				Assert.AreEqual (false, mel.CanPause, "Detached (immediately) - CanPause");
				Assert.AreEqual (false, mel.CanSeek, "Detached (immediately) - CanSeek");
				Assert.AreEqual (MediaElementState.Playing, mel.CurrentState, "Detached (immediately) - CurrentState");
				Assert.AreEqual (true, mel.IsMuted, "Detached (immediately) - IsMuted");
				Assert.AreEqual (0.0, mel.NaturalDuration.TimeSpan.TotalMilliseconds, "Detached (immediately) - NaturalDuration");
				Assert.AreEqual (0.0, mel.Position.TotalMilliseconds, "Detached (immediately) - Position");
				Assert.AreEqualWithDelta (0.33, mel.Volume, 0.0001, "Detached (immediately) - Volume");
			});

			// wait a bit
			EnqueueSleep (100);

			// check detached values
			Enqueue (delegate ()
			{
				Assert.AreEqual (0, media_opened_counter, "Detached - MediaOpenedEvent");
				Assert.AreEqual (0, media_ended_counter, "Detached - MediaEndedEvent");
				Assert.AreEqual (0, media_failed_counter, "Detached - MediaFailedEvent");
				Assert.AreEqual (0, media_buffering_progress_counter, "Detached - BufferingProgressChangedEvent");
				Assert.AreEqual (0, media_download_progress_counter, "Detached - DownloadProgressChangedEvent");
				Assert.AreEqual (0, media_state_changed_counter, "Detached - CurrentStateChangedEvent");
				Assert.AreEqual (false, mel.AutoPlay, "Detached - AutoPlay");
				Assert.AreEqualWithDelta (0.7, mel.Balance, 0.0001, "Detached - Balance");
				Assert.AreEqual (0.0, mel.BufferingProgress, "Detached - BufferingProgress");
				Assert.AreEqual (3.0, mel.BufferingTime.TotalSeconds, "Detached - BufferingTime");
				Assert.AreEqual (false, mel.CanPause, "Detached - CanPause");
				Assert.AreEqual (false, mel.CanSeek, "Detached - CanSeek");
				Assert.AreEqual (MediaElementState.Playing, mel.CurrentState, "Detached - CurrentState");
				Assert.AreEqual (true, mel.IsMuted, "Detached - IsMuted");
				Assert.AreEqual (0.0, mel.NaturalDuration.TimeSpan.TotalMilliseconds, "Detached - NaturalDuration");
				Assert.AreEqual (0.0, mel.Position.TotalMilliseconds, "Detached - Position");
				Assert.AreEqualWithDelta (0.33, mel.Volume, 0.0001, "Detached - Volume");
			});

			// reattach

			Enqueue (delegate ()
			{
				TestPanel.Children.Add (mel);
				// check which properties changed immediately
				Assert.AreEqual (0, media_opened_counter, "Reattached (immediately) - MediaOpenedEvent");
				Assert.AreEqual (0, media_ended_counter, "Reattached (immediately) - MediaEndedEvent");
				Assert.AreEqual (0, media_failed_counter, "Reattached (immediately) - MediaFailedEvent");
				Assert.AreEqual (0, media_buffering_progress_counter, "Reattached (immediately) - BufferingProgressChangedEvent");
				Assert.AreEqual (0, media_download_progress_counter, "Reattached (immediately) - DownloadProgressChangedEvent");
				Assert.AreEqual (0, media_state_changed_counter, "Reattached (immediately) - CurrentStateChangedEvent");
				Assert.AreEqual (false, mel.AutoPlay, "Reattached (immediately) - AutoPlay");
				Assert.AreEqualWithDelta (0.7, mel.Balance, 0.001, "Reattached (immediately) - Balance");
				Assert.AreEqual (1.0, mel.BufferingProgress, "Reattached (immediately) - BufferingProgress");
				Assert.AreEqual (3.0, mel.BufferingTime.TotalSeconds, "Reattached (immediately) - BufferingTime");
				Assert.AreEqual (true, mel.CanPause, "Reattached (immediately) - CanPause");
				Assert.AreEqual (true, mel.CanSeek, "Reattached (immediately) - CanSeek");
				Assert.IsTrue (mel.CurrentState == MediaElementState.Opening || mel.CurrentState == MediaElementState.Playing, "Reattached (immediately) - CurrentState");
				Assert.AreEqual (true, mel.IsMuted, "Reattached (immediately) - IsMuted");
				Assert.AreEqual (0.0, mel.NaturalDuration.TimeSpan.TotalMilliseconds, "Reattached (immediately) - NaturalDuration");
				Assert.AreEqual (0.0, mel.Position.TotalMilliseconds, "Reattached (immediately) - Position");
				Assert.AreEqualWithDelta (0.33, mel.Volume, 0.0001, "Reattached (immediately) - Volume");
			});

			// wait a bit
			EnqueueSleep (200);
			EnqueueConditional (() => media_opened_counter >= 1);

			Enqueue (delegate () {
				Assert.AreEqual (1, media_opened_counter, "Reattached A - MediaOpenedEvent");
				Assert.AreEqual (0, media_ended_counter, "Reattached A - MediaEndedEvent");
				Assert.AreEqual (0, media_failed_counter, "Reattached A - MediaFailedEvent");
				Assert.IsGreater (0, media_buffering_progress_counter, "Reattached A - BufferingProgressChangedEvent");
				Assert.IsGreater (0, media_download_progress_counter, "Reattached A - DownloadProgressChangedEvent");
				Assert.IsGreater (0, media_state_changed_counter, "Reattached A - CurrentStateChangedEvent");
				Assert.AreEqual (false, mel.AutoPlay, "Reattached A - AutoPlay");
				Assert.AreEqualWithDelta (0.7, mel.Balance, 0.001, "Reattached A - Balance");
				Assert.AreEqual (1.0, mel.BufferingProgress, "Reattached A - BufferingProgress");
				Assert.AreEqual (3.0, mel.BufferingTime.TotalSeconds, "Reattached A - BufferingTime");
				Assert.AreEqual (true, mel.CanPause, "Reattached A - CanPause");
				Assert.AreEqual (true, mel.CanSeek, "Reattached A - CanSeek");
				Assert.AreEqual (MediaElementState.Paused, mel.CurrentState, "Reattached A - CurrentState");
				Assert.AreEqual (true, mel.IsMuted, "Reattached A - IsMuted");
				Assert.AreEqual (30033.0, mel.NaturalDuration.TimeSpan.TotalMilliseconds, "Reattached A - NaturalDuration");
				Assert.AreEqual (0.0, mel.Position.TotalMilliseconds, "Reattached A - Position");
				Assert.AreEqualWithDelta (0.33, mel.Volume, 0.001, "Reattached A - Volume");
			});


			Enqueue (delegate ()
			{
				mel.Play ();
			});

			// wait until 0.1s have been played. This is to check that when we start playing again
			// we start at the beginning, not the position when the ME was detached.
			EnqueueConditional (() => mel.Position.TotalMilliseconds > 100);

			Enqueue (delegate ()
			{
				Assert.AreEqual (1, media_opened_counter, "Reattached B - MediaOpenedEvent");
				Assert.AreEqual (0, media_ended_counter, "Reattached B - MediaEndedEvent");
				Assert.AreEqual (0, media_failed_counter, "Reattached B - MediaFailedEvent");
				Assert.IsGreater (0, media_buffering_progress_counter, "Reattached B - BufferingProgressChangedEvent");
				Assert.IsGreater (0, media_download_progress_counter, "Reattached B - DownloadProgressChangedEvent");
				Assert.IsGreater (0, media_state_changed_counter, "Reattached B - CurrentStateChangedEvent");
				Assert.AreEqual (false, mel.AutoPlay, "Reattached B - AutoPlay");
				Assert.AreEqualWithDelta (0.7, mel.Balance, 0.001, "Reattached B - Balance");
				Assert.AreEqual (1.0, mel.BufferingProgress, "Reattached B - BufferingProgress");
				Assert.AreEqual (3.0, mel.BufferingTime.TotalSeconds, "Reattached B - BufferingTime");
				Assert.AreEqual (true, mel.CanPause, "Reattached B - CanPause");
				Assert.AreEqual (true, mel.CanSeek, "Reattached B - CanSeek");
				Assert.AreEqual (MediaElementState.Playing, mel.CurrentState, "Reattached B - CurrentState");
				Assert.AreEqual (true, mel.IsMuted, "Reattached B - IsMuted");
				Assert.AreEqual (30033.0, mel.NaturalDuration.TimeSpan.TotalMilliseconds, "Reattached B - NaturalDuration");
				Assert.IsBetween (0.100, 0.9, mel.Position.TotalSeconds, "Reattached B - Position");
				Assert.AreEqualWithDelta (0.33, mel.Volume, 0.001, "Reattached B - Volume");
			});

			// detach with AutoPlay = true
			Enqueue (delegate ()
			{
				mel.AutoPlay = true;
				TestPanel.Children.Clear ();
			});

			// reattach
			Enqueue (delegate ()
			{
				TestPanel.Children.Add (mel);
			});

			// wait until we've started playing again
			EnqueueConditional (() => mel.Position.TotalMilliseconds > 0);

			EnqueueTestComplete ();
		}

		[TestMethod]
		[MoonlightBug]
		public void TestInvalidValues()
		{
			MediaElement m = new MediaElement();
			Assert.Throws<InvalidOperationException>(delegate {
				m.SetValue (MediaElement.AttributesProperty, null);
			}, "#1");
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
