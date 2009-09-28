
using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media.Animation;
using System.Windows.Media;
using System.Windows.Threading;

namespace clock43 {
	public partial class Canvas : System.Windows.Controls.Canvas {
		DispatcherTimer timer;

		public Canvas ()
		{
			InitializeComponent ();

			timer = new DispatcherTimer ();
			timer.Interval = TimeSpan.FromMilliseconds(1500);

			timer.Tick += (o, i) => {
				storyboard.Begin();
				timer.Stop ();
			};

			MouseMove += (o, e) => {
				storyboard.Stop();
				timer.Stop ();
				timer.Start ();
			};

			Loaded += (o,e) => {
				storyboard.Begin();
			};
		}
		
	}
}
