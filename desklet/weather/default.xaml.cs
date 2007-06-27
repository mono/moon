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
using System.Windows.Controls;
using System.Windows.Media;

namespace Desklet.Weather
{
	public class Default : Canvas
	{
		Canvas updCanvas;
		Image weatherIcon;
		string iconsDir;
		
		public void Page_Loaded (object sender, EventArgs e)
		{
			updCanvas = FindName ("Updating") as Canvas;
			updCanvas.Visibility = Visibility.Hidden;

			iconsDir = IO.Path.Combine (IO.Path.Combine (Environment.CurrentDirectory, "data"), "icons");
			weatherIcon = FindName ("WeatherIcon") as Image;
			weatherIcon.Source = new Uri (IO.Path.Combine (iconsDir, "clouds_nodata.png"));
		}
	}
}
