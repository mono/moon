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

namespace MoonTest.System.Windows
{
	[TestClass]
	public partial class MediaElementTest
	{
		MediaElement media = new MediaElement ();
		WebClient client = new WebClient ();
			
		[TestMethod]
		public void SetSourceTest () 
		{
			client.DownloadProgressChanged += new DownloadProgressChangedEventHandler (client_DownloadProgressChanged);
			client.OpenReadCompleted += new OpenReadCompletedEventHandler (client_OpenReadCompleted);
			client.OpenReadAsync (new Uri ("http://localhost:8080/elephants-dream-320x180-first-minute.wmv", UriKind.Absolute));
		}

		void client_DownloadProgressChanged (object sender, DownloadProgressChangedEventArgs e) {
			Debug.WriteLine ("client_DownloadProgressChanged: " + e.ProgressPercentage.ToString ());
		}

		void client_OpenReadCompleted (object sender, OpenReadCompletedEventArgs e) {
			Console.WriteLine ("client_OpenRead: e.Result: {0}", e.Result == null ? "null" : e.Result.ToString ());
			if (e.Result != null)
				media.SetSource (new SlowStream (e.Result));
		}
	}
}
