
using System;
using System.Windows;
using System.Windows.Media.Animation;
using System.Windows.Controls;
using Moon.Windows.Desktop;

namespace windowtest {

	public partial class MainWindow : Window {
		public MainWindow ()
		{
			InitializeComponent ();

			Storyboard.SetTarget (pulseAnim, this);
			Storyboard.SetTarget (transparentAnim, this);
			Storyboard.SetTarget (opacityAnim, this);

			pulseButton.Click += (o, e) => {
				pulseAnim.Begin ();
			};

			opacityButton.Checked += (o, e) => {
				transparentAnim.Begin ();
			};

			opacityButton.Unchecked += (o, e) => {
				opacityAnim.Begin ();
			};
		}

		void windowMoved (object o, EventArgs e)
		{
			
		}
	}

}
