/*
 * clock.cs: Glassy clock desklet.
 *
 * Author:
 *   Everaldo Canuto (ecanuto@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

namespace Desklets.Clock
{
	public partial class Clock : Canvas 
	{
/*		RotateTransform secondsHand;
		RotateTransform minuteHand;
		RotateTransform hourHand;
*/
		Polygon closeButton;
		
		Brush buttonHilite = new SolidColorBrush (Color.FromArgb (0xAA, 0xFF, 0xFF, 0xFF));
		Brush buttonNormal = new SolidColorBrush (Color.FromArgb (0x66, 0xFF, 0xFF, 0xFF));

		public Clock()
		{
			InitializeComponent(); 
		}

		void HighlightButton (Polygon button)
		{
			button.Stroke = buttonHilite;
		}

		void UnhighlightButton (Polygon button)
		{
			button.Stroke = buttonNormal;
		}
		
		public void PageLoaded (object o, EventArgs e)
		{
			Moonlight.Gtk.Desklet.SetupToolbox (this);
			
			secondsHand = FindName ("secondsHand") as RotateTransform;
			minuteHand  = FindName ("minuteHand")  as RotateTransform;
			hourHand    = FindName ("hourHand")    as RotateTransform;
			closeButton = FindName ("desklet-close") as Polygon;
			
			if (secondsHand == null || minuteHand == null || hourHand == null || closeButton == null)
				return;

			closeButton.MouseEnter += delegate {
				HighlightButton (closeButton);
			};

			closeButton.MouseLeave += delegate {
				UnhighlightButton (closeButton);
			};
			
			DateTime now = DateTime.Now;

			secondsHand.Angle = now.Second * 6;
			minuteHand.Angle  = now.Minute * 6;
			hourHand.Angle    = now.Hour * 30;
		}
	}
}
