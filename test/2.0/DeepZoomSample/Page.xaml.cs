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
			msimage.Source = new DeepZoomImageTileSource (new Uri ("file:///home/sde/Mono/moon/test/2.0/DeepZoomSample/output/info.xml"));
		}
	}
}
