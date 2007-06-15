using System;
using Gtk;
using Gtk.Moonlight;
using System.Windows;

class X {
	static void Main ()
	{
		Application.Init ();

		GtkSilver silver = new GtkSilver (400, 400);
		
		Downloader d = new Downloader ();
		d.Completed += delegate {
			Console.WriteLine ("DOWNLOADER: completed");
		};

		d.DownloadProgressChanged += delegate {
			Console.WriteLine ("DOWNLOADER: Progress is now");
			Console.WriteLine ("          : {0}", d.DownloadProgress);
		};

		d.Open ("GET", new Uri ("file:///tmp/image.png"), true);
		d.Send ();
		Application.Run ();
	}
}
