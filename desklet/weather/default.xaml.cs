/*
 * default.xaml.cs: Moonlight Desklets Weather Desklet default class
 *
 * Author:
 *   Marek Habersack (mhabersack@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */
using System;
using IO=System.IO;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Animation;

namespace Desklet.Weather
{
	public class Default : Canvas
	{
		Canvas updCanvas;
		Image weatherIcon;
		TextBlock stationID;
		string iconsDir;

		public void DownloadComplete (Downloader downloader)
		{
			string response = downloader.GetResponseText ("What?");

			// We're getting full HTML from the CGI script, need to extract our data
			int idxStart = response.IndexOf (String.Format ("\n{0}", stationID.Text));
			int idxEnd;

			if (idxStart < 0)
				return;
			idxStart++;
			idxEnd = response.IndexOf ('\n', idxStart);
			if (idxEnd < 0)
				idxEnd = response.Length;
			
			string metarData = response.Substring (idxStart, idxEnd - idxStart);
			Metar metar;

			try {
				metar = new Metar (metarData);
			} catch {
				metar = null;
			}

			if (metar == null)
				return;
		}

		public void DownloadFailed (Downloader downloader)
		{
			// report failure
		}
		
		public void UpdateData ()
		{
			updCanvas.Visibility = Visibility.Visible;

			Storyboard animation = FindName ("animation") as Storyboard;
			animation.Begin ();

			Downloader downloader = new Downloader ();
			downloader.Completed += delegate {
				DownloadComplete (downloader);
				animation.Stop ();
				updCanvas.Visibility = Visibility.Hidden;
			};

			downloader.DownloadFailed += delegate {
				DownloadFailed (downloader);
				animation.Stop ();
				updCanvas.Visibility = Visibility.Hidden;
			};
			
			downloader.Open (
				"GET",
				new Uri (String.Format ("http://weather.noaa.gov/cgi-bin/mgetmetar.pl?cccc={0}", stationID.Text)),
				true);
			downloader.Send ();
		}

		public void AdjustLayout ()
		{
			TextBlock stationIdLabel = FindName ("StationIDLabel") as TextBlock;

			double idLeft = (double)stationID.GetValue (Canvas.LeftProperty);
			stationIdLabel.SetValue (Canvas.LeftProperty, idLeft - stationIdLabel.ActualWidth - 5.0);
		}
		
		public void Page_Loaded (object sender, EventArgs e)
		{
			updCanvas = FindName ("Updating") as Canvas;
			updCanvas.Visibility = Visibility.Hidden;

			iconsDir = IO.Path.Combine (IO.Path.Combine (Environment.CurrentDirectory, "data"), "icons");
			weatherIcon = FindName ("WeatherIcon") as Image;
			weatherIcon.Source = new Uri (IO.Path.Combine (iconsDir, "clouds_nodata.png"));

			stationID = FindName ("StationID") as TextBlock;
			
			Storyboard run = FindName ("run") as Storyboard;
			run.Begin ();

			AdjustLayout ();
			UpdateData ();
		}
	}
}
