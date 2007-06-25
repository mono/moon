using System.Windows;
using System;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

namespace Clock {
	
	public class Display : Canvas {
		TextBlock hour, minute;
		DateTime last = DateTime.MinValue;
		
		void UpdateTime ()
		{
			DateTime dt = DateTime.Now;

			if (dt.Hour == last.Hour && dt.Minute == last.Minute)
				return;

			if (dt.Hour != last.Hour)
				hour.Text = dt.Hour.ToString ("00");

			minute.Text = ":" + dt.Minute.ToString ("00");
			last = dt;
		}
		
		public void PageLoaded(object o, EventArgs e)
		{
			bool visible = false;
			
			Rectangle r = FindName ("dotcover") as Rectangle;
			Storyboard sb = FindName ("run") as Storyboard;
			hour = FindName ("hour") as TextBlock;
			minute = FindName ("minute") as TextBlock;
			
			if (sb == null || r == null || hour == null || minute == null){
				Console.WriteLine ("Elements are missing from the xaml file\n");
				return;
			}
			
			DoubleAnimation timer = new DoubleAnimation ();
			sb.Children.Add (timer);
			timer.Duration = new Duration (TimeSpan.FromSeconds (0.5));

			
			sb.Completed += delegate {
				visible = !visible;
				UpdateTime ();
				Console.WriteLine ("here");
				r.Opacity = visible ? 0.0 : 1.0;
				sb.Begin ();
			};
			UpdateTime ();
			sb.Begin ();
		}
	}
}
