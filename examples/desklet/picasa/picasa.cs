using System;
using IO=System.IO;
using System.Xml; 
using System.Net;
using System.Collections.Generic; 
using Google.GData.Client;

using System.Windows;
using System.Windows.Input;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

namespace Desklets
{
	public class SlideImage {
		private Image image = new Image ();
		private bool ready=false;
		private Canvas canvas;

		public SlideImage (Canvas canvas, String uri, Downloader downloader) {
			this.canvas = canvas;
			image.Height = canvas.Height;
			image.Width = canvas.Width;

			downloader.Completed += delegate {
				this.ready = true;
				if (ImageReady != null)
					ImageReady (this, null);
			};
			image.Stretch = Stretch.Uniform;
			image.SetSource (downloader, uri);
		}

		public void Remove ()
		{
			canvas.Children.Remove (image);
		}

		public void Add ()
		{
			canvas.Children.Add (image);
		}

		public bool Ready { 
			get { return ready; } 
		}

		public event EventHandler ImageReady;
	}

	public class PicasaPanel : Canvas 
	{
		String[] urls;
		
		List<SlideImage> images = new List<SlideImage>();
		int imgIdx=0;
		
		Storyboard storyBoard;
		SlideImage active;
		
		public void DownloadImage (int idx)
		{
			Console.WriteLine ("Downloading "+idx);
			
			//Download images in a pipelines fashion
			//FIXME: use only one downloader instance once it supports been used multiple times.
			Downloader downloader = new Downloader ();
			if (idx + 1 < urls.Length)
				downloader.Completed += delegate { DownloadImage (idx + 1); };

			SlideImage img = new SlideImage (this, urls [idx], downloader);
			images.Add (img); 
			if (images.Count == 1)
				DisplayNextImage ();
		}
		
		public void DisplayNextImage () {
			Console.WriteLine ("-----displaying image "+imgIdx+" - "+DateTime.Now);

			SlideImage img = images[imgIdx];
			if (!img.Ready) {
				Console.WriteLine ("----bummer image not ready "+DateTime.Now);
				img.ImageReady += delegate { DisplayNextImage(); };
				return;
			}
			if (active != null)
				active.Remove ();
			img.Add();
			active = img;

			if (++imgIdx >= images.Count) 
				imgIdx = 0;

			storyBoard.Begin ();
		}

		public void PageLoaded (object o, EventArgs e) 
	    {
			storyBoard = FindName ("run") as Storyboard;
			storyBoard.Completed += delegate { DisplayNextImage (); };

			EventHandler handler = delegate { enumerateImages (); };
			handler.BeginInvoke(null, null, 
			null, null);
		}


		public void enumerateImages ()
		{
			//FIXME make this configurable, it should at least flip between all albuns of the user
			String feed = "http://picasaweb.google.com/data/feed/api/user/kumpera/album/RandomPhotos?kind=photo";

            FeedQuery query = new FeedQuery();
            Service service = new Service("picasa", "MoonLight Desklets");
            
            query.Uri = new Uri(feed);
			AtomFeed picasaFeed = service.Query(query);
			urls = new String [picasaFeed.Entries.Count];
			
			int i=0;
			foreach (AtomEntry entry in picasaFeed.Entries) {
				urls[i++] = entry.Content.Src.Content;
				Console.WriteLine ("content: "+entry.Content.Src.Content);
			}
			
			Moonlight.Gtk.Desklet.Invoke(delegate { DownloadImage (0); });
		}
	}
}