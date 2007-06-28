using System;
using System.Globalization;

using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

namespace Desklet
{
	public class Calculator : Canvas 
	{
		public void PageLoaded (object o, EventArgs e) 
		{
			this.MouseEvents ();
			
			Storyboard sb = FindName ("run") as Storyboard;
			DoubleAnimation timer = new DoubleAnimation ();
			sb.Children.Add (timer);
			timer.Duration = new Duration (TimeSpan.FromSeconds (3600));

			sb.Completed += delegate {
				sb.Begin ();
			};
			sb.Begin ();
		}

		private void MouseEvents ()
		{
			Shape button7 = FindName ("7ButtonFrame") as Shape;
			button7.MouseLeftButtonUp += delegate {
				Console.WriteLine ("7 button");
			};
			Shape button8 = FindName ("8ButtonFrame") as Shape;
			button8.MouseLeftButtonUp += delegate {
				Console.WriteLine ("8 button");
			};
			Shape button9 = FindName ("9ButtonFrame") as Shape;
			button9.MouseLeftButtonUp += delegate {
				Console.WriteLine ("9 button");
			};
			Shape button6 = FindName ("6ButtonFrame") as Shape;
			button6.MouseLeftButtonUp += delegate {
				Console.WriteLine ("6 button");
			};
			Shape button5 = FindName ("5ButtonFrame") as Shape;
			button5.MouseLeftButtonUp += delegate {
				Console.WriteLine ("5 button");
			};
			Shape button4 = FindName ("4ButtonFrame") as Shape;
			button4.MouseLeftButtonUp += delegate {
				Console.WriteLine ("4 button");
			};
			Shape button3 = FindName ("3ButtonFrame") as Shape;
			button3.MouseLeftButtonUp += delegate {
				Console.WriteLine ("3 button");
			};
			Shape button2 = FindName ("2ButtonFrame") as Shape;
			button2.MouseLeftButtonUp += delegate {
				Console.WriteLine ("2 button");
			};
			Shape button1 = FindName ("1ButtonFrame") as Shape;
			button1.MouseLeftButtonUp += delegate {
				Console.WriteLine ("1 button");
			};       
			Shape button0 = FindName ("0ButtonFrame") as Shape;
			button0.MouseLeftButtonUp += delegate {
				Console.WriteLine ("0 button");
			};
			Shape button = FindName ("MulButtonFrame") as Shape;
			button.MouseLeftButtonUp += delegate {
				Console.WriteLine ("+ button");
			};        
		}
	}
}
