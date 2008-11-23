// App.xaml.cs created with MonoDevelop
// User: gutz at 3:13 PM 11/10/2008
//
// To change standard headers go to Edit->Preferences->Coding->Standard Headers
//

using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;

namespace Desklets.Clock
{
	
	
	public partial class App : Application
	{
		
		public App()
		{
			this.Startup += this.Application_Startup;
			this.InitializeComponent();
		}
		
		void Application_Startup(object sender, StartupEventArgs args)
		{
			this.RootVisual = new Clock();
		}
	}
}
