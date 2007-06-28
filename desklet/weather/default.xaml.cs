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
using System.Collections.Generic;
using System.Globalization;
using IO=System.IO;
using System.Text;

using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

using Mono.Desklets;

namespace Desklet.Weather
{
	enum TemperatureScale {
		Min,
		Celcius,
		Kelvin,
		Fahrenheit,
		Max
	};

	enum CloudsUnits {
		Min,
		Feet,
		Meters,
		Kilometers,
		Max
	}

	enum WindUnits {
		Min,
		Knots,
		MetersPerSecond,
		KilometersPerHour,
		MilesPerHour,
		Max
	};
	
	public class Default : Canvas
	{
		readonly string[] wind_directions = new string [] {
			"North",
			"North-Northeast",
			"Northeast",
			"East-Northeast",
			"East",
			"East-Southeast",
			"Southeast",
			"South-Southeast",
			"South",
			"South-Southwest",
			"Southwest",
			"West-Southwest",
			"West",
			"West-Northwest",
			"Northwest",
			"North-Northwest",
			"North"
		};
		
		Canvas updCanvas;
		Canvas temperaturePanel;
		Canvas cloudsPanel;
		Canvas windPanel;
		
		Image weatherIcon;
		
		TextBlock stationID;
		TextBlock temperature;
		TextBlock dewPoint;
		TextBlock dewPointLabel;
		TextBlock loadingMessage;
		TextBlock skyConditions;
		TextBlock windConditions;
		
		Storyboard show_updating;
		Storyboard hide_updating;

		Path closeButton;
		Path dragButton;
		
		bool allControlsPresent= true;
		string iconsDir;
		Metar metar;
		
		TemperatureScale scale = TemperatureScale.Celcius;
		CloudsUnits cloudsUnits = CloudsUnits.Meters;
		WindUnits windUnits = WindUnits.KilometersPerHour;
		
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
			SetClouds ();
			SetWind ();
		}

		public void DownloadFailed (Downloader downloader)
		{
			// report failure
			Console.WriteLine ("Download failed");
		}
		
		public void UpdateData ()
		{
			show_updating.Begin ();
			
			Downloader downloader = new Downloader ();
			downloader.Completed += delegate {
				show_updating.Stop ();
				hide_updating.Begin ();
				DownloadComplete (downloader);
			};

			downloader.DownloadFailed += delegate {
				show_updating.Stop ();
				hide_updating.Begin ();
				DownloadFailed (downloader);
			};
			
			downloader.Open (
				"GET",
				new Uri (String.Format ("http://weather.noaa.gov/cgi-bin/mgetmetar.pl?cccc={0}", stationID.Text)),
				true);
			downloader.Send ();
		}
		
		void SetWind ()
		{
			if (metar == null || metar.Wind == null)
				return;

			Wind w = metar.Wind;
			StringBuilder sb = new StringBuilder ("Wind ");
			sb.AppendFormat ("from {0}", wind_directions [(int)Math.Round (w.Direction / 22.5)]);

			double speed = 0.0;
			string unit = String.Empty;
			
			switch (windUnits) {
				case WindUnits.Knots:
					unit = "kt";
					speed = w.Knots;
					break;

				case WindUnits.MetersPerSecond:
					unit = "m/s";
					speed = w.MetersPerSecond;
					break;

				case WindUnits.KilometersPerHour:
					unit = "km/h";
					speed = w.KilometersPerHour;
					break;

				case WindUnits.MilesPerHour:
					unit = "mph";
					speed = w.MilesPerHour;
					break;
			}
			sb.AppendFormat (", {0} {1}", speed, unit);
			
			windConditions.Text = sb.ToString ();
		}
		
		void SetClouds ()
		{
			if (metar == null || metar.Clouds == null)
				return;

			StringBuilder sb = new StringBuilder ();
			List<Clouds> clouds = metar.Clouds;

			foreach (Clouds c in clouds) {
				if (sb.Length > 0)
					sb.Append ("\n");
				
				if (c.Coverage == CloudsCoverage.Clear) {
					skyConditions.Text = "Clear sky";
					return;
				}
			
				switch (c.Coverage) {
					case CloudsCoverage.Few:
					case CloudsCoverage.Scatterred:
					case CloudsCoverage.Broken:
					case CloudsCoverage.Overcast:
						sb.Append (c.Coverage.ToString ());
						break;
				}

				switch (c.Kind) {
					case CloudsKind.Cumulus:
					case CloudsKind.Cumulonimbus:
					case CloudsKind.Cirrus:
						sb.AppendFormat (" {0}", c.Kind.ToString ().ToLower (CultureInfo.InvariantCulture));
						break;
					
					case CloudsKind.ToweringCumulus:
						sb.Append (" towering cumulus");
						break;
				}
				sb.Append (" clouds");

				bool showHeight = true;
				switch (c.Accuracy) {
					case CloudsAccuracy.LessThan:
						sb.Append (" at less than");
						break;

					case CloudsAccuracy.Exactly:
						sb.Append (" at");
						break;

					default:
						showHeight = false;
						break;
				}

				if (showHeight) {
					double height = 0.0;
					switch (cloudsUnits) {
						case CloudsUnits.Feet:
							height = c.Feet;
							break;

						case CloudsUnits.Meters:
							height = c.Meters;
							break;

						case CloudsUnits.Kilometers:
							height = c.Kilometers;
							break;
					}
				
					sb.AppendFormat (" {0} {1}", height, cloudsUnits.ToString ().ToLower (CultureInfo.InvariantCulture));
				}
			}
			
			skyConditions.Text = sb.ToString ();
		}
		
		void SetTemperature ()
		{
			if (metar == null)
				return;
			
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
			temperaturePanel.Opacity = 1;
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

		void ChangeCloudsUnits ()
		{
			cloudsUnits++;
			if (cloudsUnits == CloudsUnits.Max)
				cloudsUnits = CloudsUnits.Min + 1;
			SetClouds ();
		}

		void ChangeWindUnits ()
		{
			windUnits++;
			if (windUnits == WindUnits.Max)
				windUnits = WindUnits.Min + 1;
			SetWind ();
		}
		
		void AdjustLayout ()
		{
			TextBlock stationIdLabel = FindName ("StationIDLabel") as TextBlock;

			double val = (double)stationID.GetValue (Canvas.LeftProperty);
			stationIdLabel.SetValue (Canvas.LeftProperty, val - stationIdLabel.ActualWidth - 5.0);

			// doesn't work - Height is 0, should it be?
//			val = (double)temperaturePanel.GetValue (Canvas.HeightProperty);
//			cloudsPanel.SetValue (Canvas.TopProperty, val + 5.0);
		}

		public DependencyObject LoadControl (string name)
		{
			DependencyObject ret = FindName (name);
			if (ret == null)
				allControlsPresent = false;
			return ret;
		}
		
		public void LoadControls ()
		{
			weatherIcon = LoadControl ("WeatherIcon") as Image;
			stationID = LoadControl ("StationID") as TextBlock;
			temperature = LoadControl ("Temperature") as TextBlock;
			dewPoint = LoadControl ("DewPoint") as TextBlock;
			dewPointLabel = LoadControl ("DewPointLabel") as TextBlock;
			temperaturePanel = LoadControl ("TemperaturePanel") as Canvas;
			show_updating = LoadControl ("show_updating") as Storyboard;
			hide_updating = LoadControl ("hide_updating") as Storyboard;
			updCanvas = LoadControl ("UpdatingCanvas") as Canvas;
			cloudsPanel = LoadControl ("CloudsVisPanel") as Canvas;
			loadingMessage = LoadControl ("LoadingMessage") as TextBlock;
			closeButton = LoadControl ("desklet-close") as Path;
			dragButton = LoadControl ("desklet-drag") as Path;
			skyConditions = LoadControl ("SkyConditions") as TextBlock;
			windPanel = LoadControl ("WindPanel") as Canvas;
			windConditions = LoadControl ("WindConditions") as TextBlock;
		}

		void HighlightButton (Path button)
		{
			button.Stroke=new SolidColorBrush (Color.FromArgb (0xAA, 0xFF, 0xFF, 0xFF));
		}

		void UnhighlightButton (Path button)
		{
			button.Stroke=new SolidColorBrush (Color.FromArgb (0x66, 0xFF, 0xFF, 0xFF));
		}
		
		public void Page_Loaded (object sender, EventArgs e)
		{
			LoadControls ();
			Mono.Desklets.Desklet.SetupToolbox (this);
			
			if (!allControlsPresent) {
				Console.WriteLine ("Elements are missing from the xaml file");
				return;
			}
			
			iconsDir = IO.Path.Combine (IO.Path.Combine (Environment.CurrentDirectory, "data"), "icons");
			weatherIcon.Source = new Uri (IO.Path.Combine (iconsDir, "clouds_nodata.png"));
			temperaturePanel.MouseLeftButtonUp += delegate {
				ChangeTemperatureScale ();
			};

			cloudsPanel.MouseLeftButtonUp += delegate {
				ChangeCloudsUnits ();
			};

			windPanel.MouseLeftButtonUp += delegate {
				ChangeWindUnits ();
			};
			
			closeButton.MouseEnter += delegate {
				HighlightButton (closeButton);
			};

			closeButton.MouseLeave += delegate {
				UnhighlightButton (closeButton);
			};

			dragButton.MouseEnter += delegate {
				HighlightButton (dragButton);
			};

			dragButton.MouseLeave += delegate {
				UnhighlightButton (dragButton);
			};
			
			AdjustLayout ();
			UpdateData ();
		}
	}
}
