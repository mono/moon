using System;
using System.Windows;
using System.Windows.Controls;


namespace HelloWorld
{
	public partial class App : Application
	{

		public App()
		{
			this.Startup += this.Application_Startup;
			InitializeComponent();
		}

		private void Application_Startup(object sender, StartupEventArgs e)
		{
			this.RootVisual = new Page();
		}
	}
}
