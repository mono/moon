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

namespace PopupExample
{
	public partial class Page : Canvas
	{
		public Page()
		{
			InitializeComponent();
			Children.Add (Log.Standard);
			Canvas.SetZIndex (Log.Standard, 100);
			
			var child = new Button { Content = "Hide", Width = 100, Height = 50 };
			var child2 = new Button { Content = "Hide 2", Width = 100, Height = 200 };
			var b = new Button { Content = "Popup" };
			var b2 = new Button { Content = "Popup2" };
			var p = new Popup { Child = child };
			var p2 = new Popup { Child = child2 };
			Canvas.SetTop (b2, 50);
			Canvas.SetTop (p2, 100);
			p2.RenderTransform = new RotateTransform { Angle = -15, CenterX = 50 };
			
			b.Click += (sender, args) => { p.IsOpen = ! p.IsOpen; };
			child.Click += (sender, args) => { p.IsOpen = ! p.IsOpen; };

			b2.Click += (sender, args) => { p2.IsOpen = ! p2.IsOpen; };
			child2.Click += (sender, args) => { p2.IsOpen = ! p2.IsOpen; };
			
			Children.Add (p);
			Children.Add (p2);
			Children.Add (b2);
			Children.Add (b);
		}

		
	}
}
