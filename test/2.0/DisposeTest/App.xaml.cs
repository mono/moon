using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace DisposeTest {
	public partial class App : Application {
		public App ()
		{
			this.Startup += this.Application_Startup;
			InitializeComponent ();
		}
		
		private void Application_Startup (object sender, StartupEventArgs e)
		{
			System.Windows.Media.RadialGradientBrush brush = new RadialGradientBrush ();
			Console.WriteLine (brush.GradientStops);
		}
	}
}
