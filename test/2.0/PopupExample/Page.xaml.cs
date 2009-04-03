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
			
			var child = new Button { Content = "Hello" };
			var b = new Button { Content = "Popup" };
			var p = new Popup { Child = child };

			b.Click += (sender, args) => { p.IsOpen = ! p.IsOpen; };
			child.Click += (sender, args) => { p.IsOpen = ! p.IsOpen; };
			
			Children.Add (p);
			Children.Add (b);
		}
	}
}
