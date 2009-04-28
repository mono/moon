
using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media.Animation;
using System.Windows.Media;

namespace clock42 {
	public partial class Canvas : System.Windows.Controls.Canvas {
		public Canvas ()
		{
			InitializeComponent ();

			Loaded += (o, e) => {
				rectAnimation.Completed += (co, ce) => Background = new SolidColorBrush (Colors.Green);
				storyboard.Begin ();
			};
		}
		
	}
}
