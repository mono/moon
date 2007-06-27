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

			image.MediaEnded += delegate {
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
		String[] urls;// = new String[] { "mono.png", "img1.png", "img2.png", "img3.png"};
		
		List<SlideImage> images = new List<SlideImage>();
		int imgIdx=0;
		
		Storyboard storyBoard;
		SlideImage active;
		Rectangle rec;
		
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
			Console.WriteLine ("-----displaying image "+imgIdx);

			SlideImage img = images[imgIdx];
			if (!img.Ready) {
				Console.WriteLine ("----bummer image not ready\n");
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
			rec = FindName ("rec") as Rectangle;

			//FIXME, make this download in the background (or try to)
			enumerateImages ();

			storyBoard.Completed += delegate { DisplayNextImage (); };
			DownloadImage (0);			

			/*img = new Image ();
			img2 = new Image ();
			
			Children.Add (img);

			Downloader downloader = new Downloader ();
			img.SetSource (downloader, images[0]);
			
			downloader.Completed += delegate {
				downloader = new Downloader ();
				img2.SetSource (downloader, images[1]);
			};

			img.MediaEnded += delegate {
				Console.WriteLine ("img1 ended :"+DateTime.Now);
			};

			img2.MediaEnded += delegate {
				Console.WriteLine ("img2 is ready :"+DateTime.Now);
				storyBoard.Begin();
			};
			
			
			storyBoard.Completed += delegate {
				Console.WriteLine ("story board finished :"+DateTime.Now);
				
				Children.Remove (img);
				Children.Add (img2);
			};*/
		}


		public void enumerateImages ()
		{
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
		}
	}
}