/*
 * monitor.cs: Glassy network monitor desklet.
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
using System.IO;

namespace Desklets
{
	public class Monitor : Canvas 
	{
		// TODO: use eth0 for now, later we must have a place to select it.
		string device = "eth0";

		Storyboard storyboard;
		TextBlock device_text;
		TextBlock received_text;
		TextBlock sent_text;

		Polygon closeButton;
		
		Brush buttonHilite = new SolidColorBrush (Color.FromArgb (0xAA, 0xFF, 0xFF, 0xFF));
		Brush buttonNormal = new SolidColorBrush (Color.FromArgb (0x66, 0xFF, 0xFF, 0xFF));
		
		public void UpdateInfo (object o, EventArgs e)
		{
			using (StreamReader sr = new StreamReader ("/proc/net/dev")) {
				string line;

				// Ignore two first lines.
				sr.ReadLine();
				sr.ReadLine();

				while ((line = sr.ReadLine()) != null) {
					String[] pieces = line.Split (new char[] {':'});

					if ((pieces.Length > 1) && (pieces [0].Trim() == device)) {
						String[] fields = pieces [1].Split (new char[] {' '}, 
										    StringSplitOptions.RemoveEmptyEntries);

						device_text.Text   = device;
						received_text.Text = fields [1];
						sent_text.Text     = fields [9];

						break;
					}
				}
			}

			storyboard.Begin ();
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
			Mono.Desklets.Desklet.SetupToolbox (this);

			closeButton = FindName ("desklet-close") as Polygon;			

			closeButton.MouseEnter += delegate {
				HighlightButton (closeButton);
			};

			closeButton.MouseLeave += delegate {
				UnhighlightButton (closeButton);
			};
			
			storyboard    = FindName ("storyboard") as Storyboard;
			device_text   = FindName ("device") as TextBlock;
			received_text = FindName ("received") as TextBlock;
			sent_text     = FindName ("sent") as TextBlock;

			UpdateInfo (null, null);

			DoubleAnimation timer = new DoubleAnimation ();
			storyboard.Children.Add (timer);
			timer.Duration = new Duration (TimeSpan.FromSeconds (1.0));
			storyboard.Completed += new EventHandler (UpdateInfo);
			storyboard.Begin ();
		}
	}
}
