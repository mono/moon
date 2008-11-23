using System.Windows;
using System;
using System.Globalization;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

namespace Desklets.Clock {
	
	public partial class Display : Canvas {
/*		TextBlock hour, minute, ampm;
		Storyboard to_config, to_clock;
*/		DateTime last = DateTime.MinValue;
		bool isAm, use24h;
		public Display() { InitializeComponent(); }		
		void UpdateTime ()
		{
			DateTime dt = DateTime.Now;
			
			if (dt.Hour == last.Hour && dt.Minute == last.Minute)
				return;
			
			if (dt.Hour != last.Hour) {
				int h = dt.Hour;
				if (!use24h) {
					h %= 12;
					if (h == 0)
						h = 12;
				}
				hour.Text = h.ToString ("00");
			}
			
			minute.Text = dt.Minute.ToString ("00");
			last = dt;
			
			if (!use24h) {
				if (isAm && dt.Hour >= 12) {
					isAm = false;
					ampm.Text = "pm";
				} else if (!isAm && dt.Hour < 12) {
					isAm = true;
					ampm.Text = "am";
				}
			}
		}

		bool in_config = false;

		void DoConfigTransition ()
		{
			if (in_config){
				in_config = false;
				to_clock.Begin ();
			} else {
				in_config = true;
				to_config.Begin ();
			}
		}
		
		public void PageLoaded(object o, EventArgs e)
		{
			bool visible = false;
			
			UIElement r = FindName ("dotcover") as UIElement;
			Storyboard sb = FindName ("run") as Storyboard;
			hour = FindName ("hour") as TextBlock;
			minute = FindName ("minute") as TextBlock;
			ampm = FindName ("ampm") as TextBlock;
			Canvas config = FindName ("configcanvas") as Canvas;
			Canvas config_button = FindName ("config_button") as Canvas;
			to_config = FindName ("to_config") as Storyboard;
			to_clock = FindName ("to_clock") as Storyboard;
			
			if (ampm != null) {
				if (use24h)
					ampm.Visibility = Visibility.Collapsed;
				else
					isAm = ampm.Text.ToLower () == "am";
			}

			
			if (sb == null || r == null || hour == null || minute == null || config == null ||
			    to_config == null || to_clock == null){
				Console.WriteLine ("Elements are missing from the xaml file\n");
				return;
			}
			
			DoubleAnimation timer = new DoubleAnimation ();
			sb.Children.Add (timer);
			timer.Duration = new Duration (TimeSpan.FromSeconds (0.5));
			
			sb.Completed += delegate {
				visible = !visible;
				UpdateTime ();
				r.Opacity = visible ? 0.0 : 1.0;
				sb.Begin ();
			};
			UpdateTime ();

			bool in_config = false;
			
			config_button.MouseLeftButtonUp += delegate {
				DoConfigTransition ();
			};

			config.MouseLeftButtonUp += delegate {
				DoConfigTransition ();
			};
	
			sb.Begin ();
			
		}
	}
}
