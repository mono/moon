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
	public class SlideShow : Canvas 
	{
		int image_index = 1;

		Storyboard change;
		Storyboard fadein;
		Storyboard fadeout;
		ImageBrush image;

		public void ChangePicture (object o, EventArgs e)
		{
			fadein.Begin ();
		}

		public void FadeInPicture (object o, EventArgs e)
		{
			image_index = (image_index < 8) ? image_index + 1 : 1;
			
			string uri = "data/image0" + image_index.ToString() + ".jpg";
			image.SetValue (ImageBrush.ImageSourceProperty, uri);

			fadeout.Begin ();
		}

		public void FadeOutPicture (object o, EventArgs e)
		{
			change.Begin ();
		}

		public void PageLoaded (object o, EventArgs e)
		{
			change  = FindName ("change")  as Storyboard;
			fadein  = FindName ("fadein")  as Storyboard;
			fadeout = FindName ("fadeout") as Storyboard;
			image   = FindName ("image")   as ImageBrush;

			change.Completed  += new EventHandler (ChangePicture);
			fadein.Completed  += new EventHandler (FadeInPicture);
			fadeout.Completed += new EventHandler (FadeOutPicture);

			change.Begin ();
		}
	}
}
