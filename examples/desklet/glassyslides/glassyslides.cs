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

namespace Desklets.GlassySlides
{
	public partial class SlideShow : Canvas 
	{
		int image_index = 1;

/*		Storyboard change;
		Storyboard fadein;
		Storyboard fadeout;
		Storyboard replace;
		Rectangle irect;
		ImageBrush image;
*/
		Polygon closeButton;
		
		Brush buttonHilite = new SolidColorBrush (Color.FromArgb (0xAA, 0xFF, 0xFF, 0xFF));
		Brush buttonNormal = new SolidColorBrush (Color.FromArgb (0x66, 0xFF, 0xFF, 0xFF));
		
		public SlideShow () {
		
		InitializeComponent();
}
		public void ChangePicture (object o, EventArgs e)
		{
			fadein.Begin ();
		}

		public void FadeInPicture (object o, EventArgs e)
		{
			image_index = (image_index < 8) ? image_index + 1 : 1;

			string uri = "data/image0" + image_index.ToString() + ".jpg";
			image.SetValue (ImageBrush.ImageSourceProperty, uri);
			Children.Remove (irect);

			replace.Begin ();
		}

		public void ReplacePicture (object o, EventArgs e)
		{
			Children.Add (irect);
			fadeout.Begin ();
		}

		public void FadeOutPicture (object o, EventArgs e)
		{
			change.Begin ();
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
			
			change  = FindName ("change")  as Storyboard;
			fadein  = FindName ("fadein")  as Storyboard;
			fadeout = FindName ("fadeout") as Storyboard;
			replace = FindName ("replace") as Storyboard;
			irect   = FindName ("irect")   as Rectangle;
			image   = FindName ("image")   as ImageBrush;

			change.Completed  += new EventHandler (ChangePicture);
			fadein.Completed  += new EventHandler (FadeInPicture);
			fadeout.Completed += new EventHandler (FadeOutPicture);
			replace.Completed += new EventHandler (ReplacePicture);

			change.Begin ();

			closeButton = FindName ("desklet_close") as Polygon;

			closeButton.MouseEnter += delegate {
				HighlightButton (closeButton);
			};

			closeButton.MouseLeave += delegate {
				UnhighlightButton (closeButton);
			};
		}
	}
}
