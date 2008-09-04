using System;
using System.Windows;
using System.Windows.Media;
using System.Windows.Controls;

namespace DeepZoomSample {
	public partial class Page : UserControl
	{
		public Page()
		{
			InitializeComponent();
		}

		void msi_loaded (object o, RoutedEventArgs e)
		{
			Console.WriteLine ("Loaded");
			DeepZoomImageTileSource dzits = new DeepZoomImageTileSource ();
			msimage.Source = dzits;
			dzits.UriSource = new Uri ("http://192.168.42.10/~sde/output/info.xml");
		}
	}
}
