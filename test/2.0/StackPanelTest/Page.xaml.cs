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

namespace StackPanelTest
{
	public partial class Page : Canvas
	{
		public Page()
		{
			InitializeComponent();
			Children.Add (Log.Standard);
			Canvas.SetZIndex (Log.Standard, 100);

			Border b = new Border ();
			var stack = new StackPanel ();
			stack.Children.Add (CreateColoredBorder (Colors.Red));
			stack.Children.Add (CreateColoredBorder (Colors.Blue));
			stack.Children.Add (CreateColoredBorder (Colors.Green));
			stack.HorizontalAlignment = HorizontalAlignment.Right;
			b.Width = 50;
			b.Background = new SolidColorBrush (Colors.Yellow);
			stack.Width = 50;
			stack.Height = 30;
			b.Child = stack;
			Children.Add (b);
			
			stack.Loaded += (sender, args) => { 
				stack.Log ("loaded");
			};

			stack.SizeChanged += (sender, args) => { 
				FrameworkElement fe = (FrameworkElement)stack.Children[0];
				Rect slot = LayoutInformation.GetLayoutSlot (fe);
				stack.Log ("child 0 slot = " + slot.ToString ());
				stack.Log ("updated");
				fe.Height = 5;
				fe.Log ("DesiredSize = " + fe.DesiredSize.ToString ());
				fe.SizeChanged += (sender1, args1) => {
					fe.Log ("child 0 slot = " + LayoutInformation.GetLayoutSlot ((FrameworkElement)sender1));
					fe.Log ("DesiredSize = " + fe.DesiredSize.ToString ());
					fe.Log ("updated");
				};
			};
		}

		public FrameworkElement CreateColoredBorder (Color c)
		{
			var b = new Border ();
			b.Background = new SolidColorBrush (c);
			return b;
		}
	}
}
