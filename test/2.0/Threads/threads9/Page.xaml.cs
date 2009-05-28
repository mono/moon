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
using System.Windows.Threading;
using System.Diagnostics;

// DispatchTimer and CompositionTarget events are bound together in this case

namespace threads9 {
	public partial class Page : UserControl {

		double ticks, ticks2, ticks3;
		int render;
		int timer, timer2;
		double start;
		int fps, fps2, fps3;
		int count = 0;
		DispatcherTimer t, t2;

		public Page () {
			InitializeComponent ();
			this.Loaded += new RoutedEventHandler (Page_Loaded);

			Application.Current.Host.Settings.MaxFrameRate = 20;

		}

		void Page_Loaded (object sender, RoutedEventArgs e) {
			ticks = start = DateTime.Now.Millisecond;
			Console.WriteLine (ticks);
			CompositionTarget.Rendering += new EventHandler (CompositionTarget_Rendering);
			t = new DispatcherTimer ();
			t.Tick += new EventHandler (t_Tick);
			t.Start ();

			t2 = new DispatcherTimer ();
			t2.Tick += new EventHandler (t2_Tick);
			t2.Start ();
			ticks = DateTime.Now.Millisecond;
			Console.WriteLine (ticks);
			myStoryboard.Begin ();

		}

		void t_Tick (object sender, EventArgs e) {
			timer++;
			ticks = DateTime.Now.Millisecond;
			//Console.WriteLine (ticks + ":" + timer + ":" + render + ": timer tick");
			if (ticks > start && fps == 0) {
				fps = timer;
				timer = 0;
				Console.WriteLine ("DispatchTimer fps: " + fps);
				if (count++ > 3) {
					Console.WriteLine ("stopping DispatcherTimer 2");
					t2.Stop ();
				}
			} else if (ticks < start)
				fps = 0;
		}

		void t2_Tick (object sender, EventArgs e) {
			timer2++;
			ticks2 = DateTime.Now.Millisecond;
			//Console.WriteLine (ticks + ":" + timer + ":" + render + ": timer tick");
			if (ticks2 > start && fps2 == 0) {
				fps2 = timer2;
				timer2 = 0;
				Console.WriteLine ("DispatchTimer 2 fps: " + fps2);
			} else if (ticks2 < start)
				fps2 = 0;
		}

		void CompositionTarget_Rendering (object sender, EventArgs e) {
			render++;
			ticks3 = DateTime.Now.Millisecond;
			//Console.WriteLine (ticks + ":" + timer + ":" + render + ": rendering tick");
			if (ticks3 > start && fps3 == 0) {
				fps3 = render;
				render = 0;
				Console.WriteLine ("Rendering fps: " + fps3);
			} else if (ticks3 < start)
				fps3 = 0;
		}
	}
}
