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
using System.Windows.Threading;
using System.Diagnostics;

namespace threads8 {
	public partial class Page : UserControl {

		long start, ticks;
		int render;
		int fps;
		int c;
		bool dispatched = true;
		int remaining = 0;

		public Page () {
			InitializeComponent ();
			Application.Current.Host.Settings.MaxFrameRate = 200;
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
			//Console.WriteLine ("rendering " + c + ":" + remaining);
			ticks = DateTime.Now.Ticks;
			if (TimeSpan.FromTicks (ticks - start).Seconds >= 1) {
				if (fps == 0) {
					fps = render;
					start = DateTime.Now.Ticks;
					render = 0;
					Console.WriteLine ("Rendering fps: " + fps + ":" + Application.Current.Host.Settings.MaxFrameRate);
					//Application.Current.Host.Settings.MaxFrameRate+=5;
				}

			} else
				fps = 0;

			if (dispatched && remaining == 0) {
				dispatched = false;
				for (int i = 0; i < 10; i++) {
					remaining++;
					Dispatcher.BeginInvoke (new dispatchevent (dispatch), new object [] { c });
				}


			}

			//Console.WriteLine ("rendering out");

		}

		delegate void dispatchevent (int i);
		void dispatch (int i) {
			
			//Console.WriteLine ("dispatched from " + i);
			dispatched = true;
			remaining--;
			//Thread.Sleep (1);
		}
	}
}
