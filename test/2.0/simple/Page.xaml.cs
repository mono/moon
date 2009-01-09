using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

namespace simple
{
	public partial class Page : Canvas
	{
		public Page()
		{
			InitializeComponent();

			var image = new Image ();
			Children.Add (image);
			image.Source = new BitmapImage (new Uri ("mono.png", UriKind.RelativeOrAbsolute));
		}
	}
}
