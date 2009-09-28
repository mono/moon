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

namespace threads4 {
	public partial class Page : UserControl {

		double ticks;
		int render;
		int timer;
		double start;
		int fps, fps2;

		public Page () {
			InitializeComponent ();
			this.Loaded += new RoutedEventHandler (Page_Loaded);

			Application.Current.Host.Settings.MaxFrameRate = 20;

		}

		void Page_Loaded (object sender, RoutedEventArgs e) {
			ticks = start = DateTime.Now.Millisecond;
			Console.WriteLine (ticks);
			CompositionTarget.Rendering += new EventHandler (CompositionTarget_Rendering);
			DispatcherTimer t = new DispatcherTimer ();
			t.Tick += new EventHandler (t_Tick);
			t.Start ();
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
			} else if (ticks < start)
				fps = 0;
		}

		void CompositionTarget_Rendering (object sender, EventArgs e) {
			render++;
			ticks = DateTime.Now.Millisecond;
			//Console.WriteLine (ticks + ":" + timer + ":" + render + ": rendering tick");
			if (ticks > start && fps2 == 0) {
				fps2 = render;
				render = 0;
				Console.WriteLine ("Rendering fps: " + fps2);
			} else if (ticks < start)
				fps2 = 0;
		}
	}
}
