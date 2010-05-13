using System;
using System.Collections.Generic;
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

namespace MoonTest.System.Windows.Media
{
	[TestClass]
	public class MediaStreamSourceTest : Microsoft.Silverlight.Testing.SilverlightTest
	{
		public class MediaStreamSourceBase : MediaStreamSource
		{
			public class LogEntry
			{
				public string Name;
				public object Value;
				public LogEntry (string Name)
				{
					this.Name = Name;
				}
				public LogEntry (string Name, object Value)
					: this (Name)
				{
					this.Value = Value;
				}
			}
			public List<LogEntry> Log = new List<LogEntry> ();

			public List<MediaStreamDescription> AvailableMediaStreams = new List<MediaStreamDescription> ();
			public Dictionary<MediaSourceAttributesKeys, string> MediaSourceAttributes = new Dictionary<MediaSourceAttributesKeys, string> ();

			protected override void CloseMedia ()
			{
				Log.Add (new LogEntry ("CloseMedia"));
			}

			protected override void GetDiagnosticAsync (MediaStreamSourceDiagnosticKind diagnosticKind)
			{
				Log.Add (new LogEntry ("GetDiagnosticAsync", diagnosticKind));
			}

			protected override void GetSampleAsync (MediaStreamType mediaStreamType)
			{
				Log.Add (new LogEntry ("GetSampleAsync", mediaStreamType));
			}

			protected override void OpenMediaAsync ()
			{
				Log.Add (new LogEntry ("OpenMediaAsync"));

				ReportOpenMediaCompleted (MediaSourceAttributes, AvailableMediaStreams);
			}

			protected override void SeekAsync (long seekToTime)
			{
				Log.Add (new LogEntry ("SeekAsync", seekToTime));
			}

			protected override void SwitchMediaStreamAsync (MediaStreamDescription mediaStreamDescription)
			{
				Log.Add (new LogEntry ("SwitchMediaStreamAsync", mediaStreamDescription));
			}

			public void InitializeSource (bool can_seek, long duration)
			{
				InitializeSource (can_seek ? "true" : "false", duration.ToString ());
			}

			public void InitializeSource (string can_seek, string duration)
			{
				MediaSourceAttributes.Add (MediaSourceAttributesKeys.CanSeek, can_seek);
				MediaSourceAttributes.Add (MediaSourceAttributesKeys.Duration, duration);
			}

			public void AddVideoStream ()
			{
				Dictionary<MediaStreamAttributeKeys, string> mediaStreamAttributes;
				MediaStreamDescription mediaStreamDescription;

				mediaStreamAttributes = new Dictionary<MediaStreamAttributeKeys, string> ();
				mediaStreamAttributes.Add (MediaStreamAttributeKeys.CodecPrivateData, "210000010FD37E27F1678A27F859FF1F1A804908B5B8D44A9C0000010E5A67F840");
				mediaStreamAttributes.Add (MediaStreamAttributeKeys.VideoFourCC, "WVC1");
				mediaStreamAttributes.Add (MediaStreamAttributeKeys.Height, "720");
				mediaStreamAttributes.Add (MediaStreamAttributeKeys.Width, "1280");

				mediaStreamDescription = new MediaStreamDescription (MediaStreamType.Video, mediaStreamAttributes);
				AvailableMediaStreams.Add (mediaStreamDescription);
			}
		}

		[TestMethod]
		[Asynchronous ()]
		public void TestSeekable ()
		{
			bool failed = false;
			bool opened = false;
			
			MediaStreamSourceBase mss = new MediaStreamSourceBase ();
			mss.InitializeSource (true, 5000000);
			mss.AddVideoStream ();

			MediaElement mel = new MediaElement ();
			mel.SetSource (mss);
			mel.MediaFailed += new EventHandler<ExceptionRoutedEventArgs> (delegate (object sender, ExceptionRoutedEventArgs e)
			{
				failed = true;
			});
			mel.MediaOpened += new RoutedEventHandler (delegate (object sender, RoutedEventArgs e)
			{
				opened = true;
			});
			TestPanel.Children.Add (mel);
			EnqueueConditional (() => failed || opened);
			Enqueue (delegate ()
			{
				Assert.IsFalse (failed, "failed");
				Assert.IsTrue (mss.Log.Count >= 2);
				Assert.AreEqual ("OpenMediaAsync", mss.Log [0].Name, "OpenMediaAsync");
				Assert.AreEqual ("SeekAsync", mss.Log [1].Name, "SeekAsync");
				Assert.AreEqual ((long) 0, (long) mss.Log [1].Value, "SeekAsync:Value");
				Assert.IsTrue (mel.CanSeek, "CanSeek");
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous ()]
		public void TestNonSeekable ()
		{
			bool failed = false;
			bool opened = false;

			MediaStreamSourceBase mss = new MediaStreamSourceBase ();
			mss.InitializeSource (false, 5000000);
			mss.AddVideoStream ();

			MediaElement mel = new MediaElement ();
			mel.SetSource (mss);
			mel.MediaFailed += new EventHandler<ExceptionRoutedEventArgs> (delegate (object sender, ExceptionRoutedEventArgs e)
			{
				failed = true;
			});
			mel.MediaOpened += new RoutedEventHandler (delegate (object sender, RoutedEventArgs e)
			{
				opened = true;
			});
			TestPanel.Children.Add (mel);
			EnqueueConditional (() => failed || opened);
			Enqueue (delegate ()
			{
				Assert.IsFalse (failed, "failed");
				Assert.IsTrue (mss.Log.Count >= 2);
				Assert.AreEqual ("OpenMediaAsync", mss.Log [0].Name, "OpenMediaAsync");
				Assert.AreEqual ("SeekAsync", mss.Log [1].Name, "SeekAsync");
				Assert.AreEqual ((long) 0, (long) mss.Log [1].Value, "SeekAsync:Value");
				Assert.IsFalse (mel.CanSeek, "CanSeek");
			});
			EnqueueTestComplete ();
		}


		bool mediafailed;
		bool mediaopened;

		[TestMethod]
		[Asynchronous ()]
		public void TestCanSeekValues ()
		{
			MediaElement melOK;
			
			melOK = new MediaElement ();
			melOK.MediaFailed += new EventHandler<ExceptionRoutedEventArgs> (delegate (object sender, ExceptionRoutedEventArgs e)
			{
				mediafailed = true;
				Assert.Fail ("MediaFailed with value: " + (string) melOK.Tag);
			});
			melOK.MediaOpened += new RoutedEventHandler (delegate (object sender, RoutedEventArgs e)
			{
				mediaopened = true;
			});

			TestPanel.Children.Add (melOK);
			
			Test (melOK,  "true", true);
			Test (melOK, "True", true);
			Test (melOK, "TrUe", true);
			Test (melOK, "1", true);
			Test (melOK, "2", true);
			Test (melOK, "FALSE", false);
			Test (melOK, "fALSE", false);
			Test (melOK, "f", true);
			Test (melOK, "0", true); /* huh? */
			Test (melOK, "invalid", true);
			Test (melOK, "-1", true);
			Test (melOK, "yes", true);
			Test (melOK, "NO", true);
			Test (melOK, "Verdadero", true);

			EnqueueTestComplete ();
		}

		private void Test (MediaElement mel, string value, bool can_seek)
		{
			MediaStreamSourceBase mss;

			Enqueue (delegate ()
			{
				mediafailed = false;
				mediaopened = false;
				mss = new MediaStreamSourceBase ();
				mss.InitializeSource (value, "5000000");
				mss.AddVideoStream ();
				mel.Tag = value;
				mel.SetSource (mss);
			});
			EnqueueConditional (() => mediafailed || mediaopened);
			Enqueue (delegate ()
			{
				Assert.AreEqual (can_seek, mel.CanSeek, "CanSeek: " + (string) mel.Tag);
			});
		}

		class MediaStreamSourcePoker : MediaStreamSource {
			public void CallErrorOccurred (string s)
			{
				ErrorOccurred (s);
			}

			public void CallReportGetSampleCompleted (MediaStreamSample mss)
			{
				ReportGetSampleCompleted (mss);
			}

			public void CallReportOpenMediaCompleted (IDictionary<MediaSourceAttributesKeys, string> mediaStreamAttributes, IEnumerable<MediaStreamDescription> availableMediaStreams)
			{
				ReportOpenMediaCompleted (mediaStreamAttributes, availableMediaStreams);
			}

			public void CallReportSeekCompleted (long timeSeekedTo)
			{
				ReportSeekCompleted (timeSeekedTo);
			}

			public void CallReportSwitchMediaStreamCompleted (MediaStreamDescription msd)
			{
				ReportSwitchMediaStreamCompleted (msd);
			}

			protected override void CloseMedia ()
			{
				throw new NotImplementedException ();
			}

			protected override void GetDiagnosticAsync (MediaStreamSourceDiagnosticKind diagnosticKind)
			{
				throw new NotImplementedException ();
			}

			protected override void GetSampleAsync (MediaStreamType mediaStreamType)
			{
				throw new NotImplementedException ();
			}

			protected override void OpenMediaAsync ()
			{
				throw new NotImplementedException ();
			}

			protected override void SeekAsync (long seekToTime)
			{
				throw new NotImplementedException ();
			}

			protected override void SwitchMediaStreamAsync (MediaStreamDescription mediaStreamDescription)
			{
				throw new NotImplementedException ();
			}
		}

		[TestMethod]
		[MoonlightBug ("overzealous validations")]
		public void ErrorOccurred ()
		{
			MediaStreamSourcePoker mss = new MediaStreamSourcePoker ();
			mss.CallErrorOccurred (null);
		}

		[TestMethod]
		[MoonlightBug ("overzealous validations")]
		public void ReportGetSampleCompleted ()
		{
			MediaStreamSourcePoker mss = new MediaStreamSourcePoker ();
			// documented as 'the stream has ended'
			mss.CallReportGetSampleCompleted (null);
		}

		[TestMethod]
		[MoonlightBug ("overzealous validations")]
		public void ReportOpenMediaCompleted ()
		{
			MediaStreamSourcePoker mss = new MediaStreamSourcePoker ();
			mss.CallReportOpenMediaCompleted (null, null);
		}

		[TestMethod]
		[MoonlightBug ("overzealous validations")]
		public void ReportSeekCompleted ()
		{
			MediaStreamSourcePoker mss = new MediaStreamSourcePoker ();
			mss.CallReportSeekCompleted (-1);
		}

		[TestMethod]
		[MoonlightBug ("overzealous validations")]
		public void ReportSwitchMediaStreamCompleted ()
		{
			MediaStreamSourcePoker mss = new MediaStreamSourcePoker ();
			mss.CallReportSwitchMediaStreamCompleted (null);
		}
	}
}
