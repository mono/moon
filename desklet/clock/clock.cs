using System;
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
		Storyboard storyboard;

		void UpdateClock (object o, EventArgs e)
		{
			DateTime now = DateTime.Now;

			secondsHand.Angle = now.Second * 6;
			minuteHand.Angle  = now.Minute * 6;
			hourHand.Angle    = now.Hour   * 30;

			storyboard.Begin ();
		}

		public void PageLoaded (object o, EventArgs e)
		{
			secondsHand = FindName ("secondsHand") as RotateTransform;
			minuteHand  = FindName ("minuteHand")  as RotateTransform;
			hourHand    = FindName ("hourHand")    as RotateTransform;
			storyboard  = FindName ("run")         as Storyboard;
			
			if (secondsHand == null || minuteHand == null || hourHand == null || storyboard == null)
				return;

			UpdateClock (null, null);

			DoubleAnimation timer = new DoubleAnimation ();
			storyboard.Children.Add (timer);
			timer.Duration = new Duration (TimeSpan.FromSeconds (0.5));
			storyboard.Completed += new EventHandler (UpdateClock);
			storyboard.Begin ();
		}
	}
}
