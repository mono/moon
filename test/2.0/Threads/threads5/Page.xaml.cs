using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Threading;
using System.Diagnostics;

namespace threads5 {
	public partial class Page : UserControl {

		int x, y;
		bool down = false;
		double start, ticks;
		int render;
		int fps;

		public Page () {
			start = DateTime.Now.Millisecond;
			InitializeComponent ();
			Application.Current.Host.Settings.MaxFrameRate = 200;
			this.Loaded += new RoutedEventHandler (Page_Loaded);

		}

		void Page_Loaded (object sender, RoutedEventArgs e) {
			y = x = 0;
			CompositionTarget.Rendering += new EventHandler (CompositionTarget_Rendering);
		}

		void CompositionTarget_Rendering (object sender, EventArgs e) {
			render++;
			ticks = DateTime.Now.Millisecond;
			if (ticks > start && fps == 0) {
				fps = render;
				render = 0;
				Console.WriteLine ("Rendering fps: " + fps);
			} else if (ticks < start)
				fps = 0;

			if (x == 400) {
				down = true;
			} else if (x == 0) {
				down = false;
			}
			if (down)
				x--;
			else
				x++;
			MyAnimatedRectangle.SetValue (Canvas.LeftProperty, (double)x);
			if (x % 10 == 0) {
				//Console.WriteLine ("Slowing down...");
				Thread.Sleep (100);
			}
		}

	}
}
