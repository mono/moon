using System;
using Gtk;
using Gtk.Moonlight;
using System.Windows;

class X {
	static void Main (string [] args)
	{
		Application.Init ();

		GtkSilver silver = new GtkSilver (400, 400);
		
		Downloader d = new Downloader ();
		d.Completed += delegate {
			Console.WriteLine ("DOWNLOADER: completed");

			if (args.Length> 0){
				Console.WriteLine ("Got:");
				Console.WriteLine (d.GetResponseText (""));
			}
		};

		d.DownloadProgressChanged += delegate {
			Console.WriteLine ("DOWNLOADER: Progress is now");
			Console.WriteLine ("          : {0}", d.DownloadProgress);
		};

		if (args.Length == 0){
			d.Open ("GET", new Uri ("file:///tmp/image.png"), true);
			d.Send ();
		} else {
			d.Open ("GET", new Uri (args [0]), true);
			d.Send ();
		}
		Application.Run ();
	}
}
