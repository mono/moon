using System;
using System.Windows;
using System.Windows.Controls;
using System.Threading;
using System.Windows.Threading;
using System.Diagnostics;
using System.Windows.Media;

// tests when dispathed delegates are actually invoked
// looks like it's after the rendering event

namespace threads6 {
	public partial class Page : UserControl {
		long start, ticks;
		int render;
		int fps;
		int c;
		bool dispatched = true;

		public Page () {
			InitializeComponent ();
			start = DateTime.Now.Ticks;
			Application.Current.Host.Settings.MaxFrameRate = 1;
			Application.Current.Host.Settings.EnableFrameRateCounter = true;
			this.Loaded += new RoutedEventHandler (Page_Loaded);
		}

		void Page_Loaded (object sender, RoutedEventArgs e) {
			myStoryboard.Begin ();
			CompositionTarget.Rendering += new EventHandler (CompositionTarget_Rendering);
		}

		void CompositionTarget_Rendering (object sender, EventArgs e) {
			c++;
			render++;
			Console.WriteLine ("rendering " + c);
			ticks = DateTime.Now.Ticks;
			if (TimeSpan.FromTicks(ticks - start).Seconds >= 1) {
				if (fps == 0) {
					fps = render;
					start = DateTime.Now.Ticks;
					render = 0;
					Console.WriteLine ("Rendering fps: " + fps);
				}
				
			} else 
				fps = 0;

			if (dispatched) {
				dispatched = false;
				Dispatcher.BeginInvoke (new dispatchevent (dispatch), new object[]{c});
				
			}
			
			Console.WriteLine ("rendering out");

		}

		delegate void dispatchevent (int i);
		void dispatch (int i) {
			Console.WriteLine ("dispatched from " + i);
			dispatched = true;
		}
	}
}
