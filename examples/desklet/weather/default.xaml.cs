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

using Gtk;

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

	class WindDirection
	{
		public string Name;
		public double X;
		public double Y;

		public WindDirection (string name, double x, double y)
		{
			this.Name = name;
			this.X = x;
			this.Y = y;
		}
	}
	
	public class Default : Canvas
	{
		readonly double defaultUpdateInterval = 30.0; // minutes
		readonly WindDirection[] wind_directions = new WindDirection [] {
			new WindDirection ("North", 48, 0),
			new WindDirection ("North-Northeast", 62, 15),
			new WindDirection ("Northeast", 78, 19),
			new WindDirection ("East-Northeast", 82, 34),
			new WindDirection ("East", 96, 48),
			new WindDirection ("East-Southeast", 82, 62),
			new WindDirection ("Southeast", 78, 78),
			new WindDirection ("South-Southeast", 62, 82),
			new WindDirection ("South", 48, 96),
			new WindDirection ("South-Southwest", 34, 82),
			new WindDirection ("Southwest", 19, 78),
			new WindDirection ("West-Southwest", 15, 62),
			new WindDirection ("West", 0, 48),
			new WindDirection ("West-Northwest", 15, 34),
			new WindDirection ("Northwest", 19, 19),
			new WindDirection ("North-Northwest", 34, 15),
			new WindDirection ("North", 48, 0)
		};

		Storyboard run;
		
		Canvas updCanvas;
		Canvas temperaturePanel;
		Canvas cloudsPanel;
		Canvas windPanel;

		System.Windows.Controls.Image weatherIcon;
		System.Windows.Controls.Image windIcon;
		Ellipse windIndicator;
		
		TextBlock stationID;
		TextBlock temperature;
		TextBlock dewPoint;
		TextBlock dewPointLabel;
		TextBlock loadingMessage;
		TextBlock skyConditions;
		TextBlock windConditions;
		
		Storyboard show_updating;
		Storyboard hide_updating;

		Polygon closeButton;
		
		string iconsDir;
		Metar metar;
		
		TemperatureScale scale = TemperatureScale.Celcius;
		CloudsUnits cloudsUnits = CloudsUnits.Meters;
		WindUnits windUnits = WindUnits.KilometersPerHour;

		double windIconTop;
		double windIconLeft;

		Brush buttonHilite = new SolidColorBrush (Color.FromArgb (0xAA, 0xFF, 0xFF, 0xFF));
		Brush buttonNormal = new SolidColorBrush (Color.FromArgb (0x66, 0xFF, 0xFF, 0xFF));
		
		GConfConfigStorage config = new GConfConfigStorage ("Weather", 0);
		
		public void DownloadComplete (Downloader downloader)
		{			
			string response = downloader.GetResponseText ("");

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
		
		public void UpdateData (object sender, EventArgs e)
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
				new Uri (String.Format ("http://weather.noaa.gov/cgi-bin/mgetmetar.pl?cccc={0}", stationID.Text)));
			downloader.Send ();
		}
		
		void SetWind ()
		{
			if (metar == null || metar.Wind == null)
				return;

			Wind w = metar.Wind;
			StringBuilder sb = new StringBuilder ("Wind ");
			WindDirection dir = wind_directions [(int)Math.Round (w.Direction / 22.5)];
			sb.AppendFormat ("from {0}", dir.Name);

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

			// move wind indicator
			Canvas.SetLeft(windIndicator, dir.X + windIconLeft - (windIndicator.Width / 2));
			Canvas.SetTop (windIndicator, dir.Y + windIconTop - (windIndicator.Height / 2));
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
			double pos = (double)Canvas.GetTop (temperature) + temperature.ActualWidth + 5.0;
			Canvas.SetLeft (dewPointLabel, pos);
			pos += (double)dewPointLabel.ActualWidth + 5.0;
			Canvas.SetLeft (dewPoint, pos);
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

		string GetTypeName (Element e)
		{
			if (e is Location)
				return "Location";
			else if (e is City)
				return "City";
			else if (e is State)
				return "State";
			else if (e is Country)
				return "Country";
			else if (e is Region)
				return "Region";
			else
				return "Unknown";
		}
		
		void AddTreeEntries (Element e, TreeStore store, TreeIter parent)
		{
			if (e is Location)
				AddTreeEntries ((Location)e, store, parent);
			else if (e is City)
				AddTreeEntries ((City)e, store, parent);
			else if (e is State)
				AddTreeEntries ((State)e, store, parent);
			else if (e is Country)
				AddTreeEntries ((Country)e, store, parent);
			else if (e is Region)
				AddTreeEntries ((Region)e, store, parent);
		}
		
		void AddTreeEntries (Location l, TreeStore store, TreeIter parent)
		{
			store.AppendValues (parent, l.Name, GetTypeName (l), l.Code);
		}

		void AddTreeEntries (City c, TreeStore store, TreeIter parent)
		{
			foreach (Location l in c.Locations) {
				if (l == null)
					continue;
//				TreeIter iter = store.AppendValues (parent, l.Name, GetTypeName (l));
				AddTreeEntries (l, store, parent);
			}
		}
		
		void AddTreeEntries (State s, TreeStore store, TreeIter parent)
		{
			foreach (Element e in s.Locations) {
				if (e == null)
					continue;
				if (!(e is Location)) {
					TreeIter iter = store.AppendValues (parent, e.Name, GetTypeName (e), String.Empty);
					AddTreeEntries (e, store, iter);
				} else
					AddTreeEntries (e, store, parent);
			}
		}
				
		void AddTreeEntries (Country c, TreeStore store, TreeIter parent)
		{
			foreach (Element e in c.Locations) {
				if (e == null)
					continue;
				if (!(e is Location)) {
					TreeIter iter = store.AppendValues (parent, e.Name, GetTypeName (e), String.Empty);
					AddTreeEntries (e, store, iter);
				} else
					AddTreeEntries (e, store, parent);
			}
		}
		
		void AddTreeEntries (Region r, TreeStore store, TreeIter parent)
		{
			foreach (Country c in r.Countries) {
				if (c == null)
					continue;
				TreeIter iter = store.AppendValues (parent, c.Name, GetTypeName (c), String.Empty);
				AddTreeEntries (c, store, iter);
			}
		}
		
		void ChooseStation ()
		{
			Locations loc = new Locations ();
			TreeStore store = new TreeStore (typeof (string), typeof (string), typeof (string));

			List <Region> regions = loc.Regions;
			foreach (Region r in regions) {
				TreeIter iter = store.AppendValues (r.Name, GetTypeName (r), String.Empty);
				AddTreeEntries (r, store, iter);
			}
			
			Window win = new Window ("Select your location");
			win.SetDefaultSize (400,400);

			VBox vbox = new VBox (false, 0);
			win.Add (vbox);
			
			ScrolledWindow sw = new ScrolledWindow ();
			vbox.PackStart (sw, true, true, 0);
			
			TreeView tv = new TreeView (store);
			tv.HeadersVisible = true;
			tv.EnableSearch = true;
			
			tv.AppendColumn ("Location", new CellRendererText (), "text", 0);
			tv.AppendColumn ("Type", new CellRendererText (), "text", 1);
                        tv.AppendColumn ("Code", new CellRendererText (), "text", 2);
			
			sw.Add (tv);
			sw.Show ();

			HBox hbox = new HBox (true, 0);
			Button closeBtn = new Button (Stock.Close);
			closeBtn.Clicked += delegate {
				TreeSelection sel = tv.Selection;
				TreeModel model;
				TreeIter iter;

				string val = null;
				if (sel.GetSelected (out model, out iter)) {
					val = (string) model.GetValue (iter, 1);
					if (val == "Location")
						val = (string) model.GetValue (iter, 2);
					else
						val = null;
				}
				
				win.Hide ();
				win.Destroy ();

				if (!String.IsNullOrEmpty (val)) {
					stationID.Text = val;
					StoreConfig ();
					UpdateData (null, null);
				}
			};
			
			hbox.PackEnd (closeBtn, false, false, 0);
			vbox.PackStart (hbox, false, false, 1);
			
			win.ShowAll ();
		}
		
		void AdjustLayout ()
		{
			TextBlock stationIdLabel = FindName ("StationIDLabel") as TextBlock;

			double val = (double)Canvas.GetLeft (stationID);
			Canvas.SetLeft (stationIdLabel, val - stationIdLabel.ActualWidth - 5.0);

			// doesn't work - Height is 0, should it be?
//			val = (double)temperaturePanel.GetValue (Canvas.HeightProperty);
//			cloudsPanel.SetValue (Canvas.TopProperty, val + 5.0);
		}
		
		void LoadControls ()
		{
			run = Moonlight.Gtk.Desklet.FindElement (this, "run", typeof (Storyboard)) as Storyboard;
			weatherIcon = Moonlight.Gtk.Desklet.FindElement (this, "WeatherIcon", typeof (System.Windows.Controls.Image))
				as System.Windows.Controls.Image;
			windIcon = Moonlight.Gtk.Desklet.FindElement (this, "WindIcon", typeof (System.Windows.Controls.Image))
				as System.Windows.Controls.Image;
			stationID = Moonlight.Gtk.Desklet.FindElement (this, "StationID", typeof (TextBlock)) as TextBlock;
			temperature = Moonlight.Gtk.Desklet.FindElement (this, "Temperature", typeof (TextBlock)) as TextBlock;
			dewPoint = Moonlight.Gtk.Desklet.FindElement (this, "DewPoint", typeof (TextBlock)) as TextBlock;
			dewPointLabel = Moonlight.Gtk.Desklet.FindElement (this, "DewPointLabel", typeof (TextBlock)) as TextBlock;
			temperaturePanel = Moonlight.Gtk.Desklet.FindElement (this, "TemperaturePanel", typeof (Canvas)) as Canvas;
			show_updating = Moonlight.Gtk.Desklet.FindElement (this, "show_updating", typeof (Storyboard)) as Storyboard;
			hide_updating = Moonlight.Gtk.Desklet.FindElement (this, "hide_updating", typeof (Storyboard)) as Storyboard;
			updCanvas = Moonlight.Gtk.Desklet.FindElement (this, "UpdatingCanvas", typeof (Canvas)) as Canvas;
			cloudsPanel = Moonlight.Gtk.Desklet.FindElement (this, "CloudsVisPanel", typeof (Canvas)) as Canvas;
			loadingMessage = Moonlight.Gtk.Desklet.FindElement (this, "LoadingMessage", typeof (TextBlock)) as TextBlock;
			closeButton = Moonlight.Gtk.Desklet.FindElement (this, "desklet-close", typeof (Polygon)) as Polygon;
			skyConditions = Moonlight.Gtk.Desklet.FindElement (this, "SkyConditions", typeof (TextBlock)) as TextBlock;
			windPanel = Moonlight.Gtk.Desklet.FindElement (this, "WindPanel", typeof (Canvas)) as Canvas;
			windConditions = Moonlight.Gtk.Desklet.FindElement (this, "WindConditions", typeof (TextBlock)) as TextBlock;
			windIndicator = Moonlight.Gtk.Desklet.FindElement (this, "WindIndicator", typeof (Ellipse)) as Ellipse;
		}

		void StoreConfig ()
		{
			try {
				config.Store ("StationID", stationID.Text);
			} catch {
				// ignore
			}
		}
		
		void LoadConfig ()
		{
			try {
				string id = config.Retrieve ("StationID") as string;
				if (id == null)
					config.Store ("StationID", stationID.Text);
				else
					stationID.Text = id;
			} catch {
				// ignore
			}
		}
		
		void HighlightButton (Polygon button)
		{
			button.Stroke = buttonHilite;
		}

		void UnhighlightButton (Polygon button)
		{
			button.Stroke = buttonNormal;
		}
		
		public void Page_Loaded (object sender, EventArgs e)
		{
			LoadControls ();
			Moonlight.Gtk.Desklet.SetupToolbox (this);
			
			if (!Moonlight.Gtk.Desklet.AllElementsFound) {
				Console.WriteLine ("Elements are missing from the xaml file");
				return;
			}
			
			iconsDir = IO.Path.Combine (IO.Path.Combine (Environment.CurrentDirectory, "data"), "icons");
			weatherIcon.Source = new Uri (IO.Path.Combine (iconsDir, "clouds_nodata.png"));
			windIcon.Source = new Uri (IO.Path.Combine (iconsDir, "wind.png"));
			windIconLeft = (double) Canvas.GetLeft (windIcon);
			windIconTop = (double) Canvas.GetTop (windIcon);

			stationID.MouseLeftButtonUp += delegate {
				ChooseStation ();
			};
			
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

			LoadConfig ();
			AdjustLayout ();
			UpdateData (null, null);

			DoubleAnimation timer = new DoubleAnimation ();
                        //run.Children.Add (timer);
			((TimelineCollection)run.GetValue(TimelineGroup.ChildrenProperty)).Add(timer);
                        timer.Duration = new Duration (TimeSpan.FromMinutes (defaultUpdateInterval));
                        run.Completed += new EventHandler (UpdateData);
                        run.Begin ();
		}
	}
}
