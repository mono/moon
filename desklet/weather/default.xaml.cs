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
	enum TemperatureScale {
		Min,
		Celcius,
		Kelvin,
		Fahrenheit,
		Max
	};
	
	public class Default : Canvas
	{
		Canvas updCanvas;
		Canvas temperaturePanel;
		
		Image weatherIcon;
		
		TextBlock stationID;
		TextBlock temperature;
		TextBlock dewPoint;
		TextBlock dewPointLabel;
		
		string iconsDir;
		Metar metar;
		
		TemperatureScale scale = TemperatureScale.Celcius;
		
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

			try {
				metar = new Metar (metarData);
			} catch (Exception ex) {
				Console.WriteLine ("metar failed: {0}", ex);
				metar = null;
			}
			SetTemperature ();
		}

		public void DownloadFailed (Downloader downloader)
		{
			// report failure
			Console.WriteLine ("Download failed");
		}
		
		public void UpdateData ()
		{
			updCanvas.Visibility = Visibility.Visible;

			Downloader downloader = new Downloader ();
			downloader.Completed += delegate {
				DownloadComplete (downloader);
				updCanvas.Visibility = Visibility.Hidden;
			};

			downloader.DownloadFailed += delegate {
				DownloadFailed (downloader);
				updCanvas.Visibility = Visibility.Hidden;
			};
			
			downloader.Open (
				"GET",
				new Uri (String.Format ("http://weather.noaa.gov/cgi-bin/mgetmetar.pl?cccc={0}", stationID.Text)),
				true);
			downloader.Send ();
		}

		void SetTemperature ()
		{
			if (metar == null)
				return;

			if (temperaturePanel.Visibility == Visibility.Hidden)
				temperaturePanel.Visibility = Visibility.Visible;
			
			double temp = 0.0, dew = 0.0;

			switch (scale) {
				case TemperatureScale.Celcius:
					temp = metar.Temperature.Celcius;
					dew = metar.DewPoint.Celcius;
					break;

				case TemperatureScale.Kelvin:
					temp = metar.Temperature.Kelvin;
					dew = metar.DewPoint.Kelvin;
					break;

				case TemperatureScale.Fahrenheit:
					temp = metar.Temperature.Fahrenheit;
					dew = metar.DewPoint.Fahrenheit;
					break;
			}

			char scaleChar = scale.ToString () [0];
			temperature.Text = String.Format ("{0} °{1}", temp, scaleChar);
			dewPoint.Text = String.Format ("{0} °{1}", dew, scaleChar);
			
			AdjustTemperatureLayout ();
		}

		void AdjustTemperatureLayout ()
		{
			double pos = (double)temperature.GetValue (Canvas.LeftProperty) + temperature.ActualWidth + 5.0;
			dewPointLabel.SetValue (Canvas.LeftProperty, pos);
			pos += (double)dewPointLabel.ActualWidth + 5.0;
			dewPoint.SetValue (Canvas.LeftProperty, pos);
		}
		
		void ChangeTemperatureScale ()
		{
			scale++;
			if (scale == TemperatureScale.Max)
				scale = TemperatureScale.Min + 1;
			SetTemperature ();
		}
		
		void AdjustLayout ()
		{
			TextBlock stationIdLabel = FindName ("StationIDLabel") as TextBlock;

			double idLeft = (double)stationID.GetValue (Canvas.LeftProperty);
			stationIdLabel.SetValue (Canvas.LeftProperty, idLeft - stationIdLabel.ActualWidth - 5.0);
		}
		
		public void Page_Loaded (object sender, EventArgs e)
		{
			iconsDir = IO.Path.Combine (IO.Path.Combine (Environment.CurrentDirectory, "data"), "icons");
			weatherIcon = FindName ("WeatherIcon") as Image;
			weatherIcon.Source = new Uri (IO.Path.Combine (iconsDir, "clouds_nodata.png"));

			stationID = FindName ("StationID") as TextBlock;
			temperature = FindName ("Temperature") as TextBlock;
			dewPoint = FindName ("DewPoint") as TextBlock;
			dewPointLabel = FindName ("DewPointLabel") as TextBlock;
			temperaturePanel = FindName ("TemperaturePanel") as Canvas;

			temperaturePanel.Visibility = Visibility.Hidden;
			temperaturePanel.MouseLeftButtonUp += delegate {
				ChangeTemperatureScale ();
			};

			updCanvas = FindName ("Updating") as Canvas;
			updCanvas.Visibility = Visibility.Visible;
			
			AdjustLayout ();
			UpdateData ();
		}
	}
}
