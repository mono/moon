using System;
using IO=System.IO;

using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

namespace Desklets
{
	public class Clock : Canvas 
	{
		RotateTransform secondsHand;
		RotateTransform minuteHand;
		RotateTransform hourHand;
		Timer timer;

		void UpdateTime ()
		{
			DateTime now = DateTime.Now;

			secondsHand.Angle = now.Second * 6;
			minuteHand.Angle  = now.Minute * 6;
			hourHand.Angle    = now.Hour   * 30;
		}

		public void PageLoaded (object o, EventArgs e)
		{
			secondsHand = FindName ("secondsHand") as RotateTransform;
			minuteHand  = FindName ("minuteHand")  as RotateTransform;
			hourHand    = FindName ("hourHand")    as RotateTransform;
			Storyboard sb = FindName ("run") as Storyboard;
			
			if (secondsHand == null || minuteHand == null || hourHand == null || sb == null)
				return;

			UpdateTime ();
			DoubleAnimation timer = new DoubleAnimation ();
			sb.Children.Add (timer);
			timer.Duration = new Duration (TimeSpan.FromSeconds (0.5));
			sb.Completed += delegate {
				UpdateTime ();
				sb.Begin ();
			};
			sb.Begin ();
		}
	}
}
