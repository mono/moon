using System;
using System.Windows;
using System.Windows.Controls;
using System.Threading;
using System.Windows.Threading;
using System.Diagnostics;
using System.Windows.Media;

// tests when dispatch timer gets invoked

namespace threads7 {
	public partial class Page : UserControl {
		long start, ticks, ticks2, start2, ticking, render, fps, fps2;
		int c;
		bool dispatched = true;
		int cached;
		DispatcherTimer t = new DispatcherTimer ();

		public Page () {
			start = start2 = ticks = ticks2 = DateTime.Now.Ticks;
			InitializeComponent ();
			
			Application.Current.Host.Settings.MaxFrameRate = 10;
			Application.Current.Host.Settings.EnableFrameRateCounter = true;

			t.Tick += delegate (object sender1, EventArgs e1) {
				ticking++;

				ticks2 = DateTime.Now.Ticks;
				if (TimeSpan.FromTicks (ticks2 - start2).Seconds >= 1) {
					if (fps2 == 0) {
						fps2 = ticking;
						start2 = DateTime.Now.Ticks;
						ticking = 0;
						Console.WriteLine ("Ticking fps: " + fps2);
					}
				} else
					fps2 = 0;
				
			};
			t.Interval = new TimeSpan (0, 0, 0, 0, 50);


			this.Loaded += new RoutedEventHandler (Page_Loaded);
			
		}

		void Page_Loaded (object sender, RoutedEventArgs e) {
			myStoryboard.Begin ();
			CompositionTarget.Rendering += new EventHandler (CompositionTarget_Rendering);
		}

		void CompositionTarget_Rendering (object sender, EventArgs e) {
			if (!t.IsEnabled)
				t.Start ();
			c++;
			render++;
			//Console.WriteLine ("rendering " + c + ":" + TimeSpan.FromTicks (DateTime.Now.Ticks - start).Ticks / 10000);
			ticks = DateTime.Now.Ticks;
			if (TimeSpan.FromTicks (ticks - start).Seconds >= 1) {
				if (fps == 0) {
					fps = render;
					start = DateTime.Now.Ticks;
					render = 0;
					Console.WriteLine ("Rendering fps: " + fps);
					Application.Current.Host.Settings.MaxFrameRate++;
				}

			} else
				fps = 0;

			//Console.WriteLine ("rendering out");

		}
	}
}
