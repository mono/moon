using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using DebugLog;
using DebugLog.Extensions;

namespace GridTest
{
	public partial class Page : Canvas
	{
		public Page()
		{
			InitializeComponent();
			Children.Add (Log.Standard);
			Canvas.SetZIndex (Log.Standard, 100);

			Grid grid = new Grid ();
			
			Border b = new Border ();
			b.Background = new SolidColorBrush (Colors.Red);
			
			Border b2 = new Border ();
			b2.Background = new SolidColorBrush (Colors.Green);
			b2.Width = b2.Height = 50;
			b2.CornerRadius = new CornerRadius (50,25,25,25);

			grid.Children.Add (b2);
			Canvas.SetZIndex (b2, 20);
			grid.Children.Add (b);
			Canvas.SetTop (b, 50);
			Canvas.SetLeft (b, 50);
			
			Image image = new Image ();
			image.Source = new BitmapImage (new Uri ("mono.png", UriKind.RelativeOrAbsolute));
			image.Width = 25;
			image.Height = 25;
			b2.Child = image;

			var control = new UserBackgroundTest ();
			Canvas.SetTop (control, 300);
			Children.Add (control);

			grid.Width = 100;
			grid.Height = 100;

			grid.Log (grid.ColumnDefinitions.Count.ToString ());
			
			Children.Add (grid);

			grid.Loaded += (sender, args) => { 
				grid.Log (grid.ColumnDefinitions.Count.ToString ());
			};

			grid.LayoutUpdated += (sender, args) => { 
				grid.Log (grid.ColumnDefinitions.Count.ToString ());
				image.Log (LayoutInformation.GetLayoutSlot (image).ToString ());
				b.Log (LayoutInformation.GetLayoutSlot (b).ToString ());
				image.Log (((MatrixTransform)image.RenderTransform).Matrix.ToString ());
				b.Log (((MatrixTransform)b.RenderTransform).Matrix.ToString ());

				try {
					var transform = b.TransformToVisual (image);
					b.Log (((MatrixTransform)transform).Matrix.ToString ());
					transform = image.TransformToVisual (b);
					b.Log (((MatrixTransform)transform).Matrix.ToString ());
				} 
				catch (Exception e) {
					b.Log (e.ToString ());
				}
				
			};
		}

	        
	}
}
