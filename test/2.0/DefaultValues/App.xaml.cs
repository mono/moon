using System;
using System.Windows;
using System.Windows.Controls;


namespace DefaultValues {
	public partial class App : Application {
		public App ()
		{
			System.Threading.Thread.CurrentThread.CurrentCulture = new System.Globalization.CultureInfo ("en-US");
			System.Threading.Thread.CurrentThread.CurrentUICulture = new System.Globalization.CultureInfo ("en-US");
			this.Startup += this.Application_Startup;
			InitializeComponent ();
		}
		
		private void Application_Startup (object sender, StartupEventArgs e)
		{
			this.RootVisual = new Page();
		}
	}
}
