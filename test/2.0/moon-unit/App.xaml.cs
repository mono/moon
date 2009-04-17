using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Reflection;
using System.Windows;
using System.Windows.Browser;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

using Microsoft.Silverlight.Testing;
using Microsoft.Silverlight.Testing.Harness;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace Mono.Moonlight.UnitTesting
{
	public partial class App : Application
	{
		public static App Instance;
	
		public App()
		{
			Instance = this;
			this.Startup += this.Application_Startup;
			
			InitializeComponent();
		}

		private void Application_Startup(object sender, StartupEventArgs e)
		{
			this.RootVisual = Testing.CreateTestPage (this);
		}

		public event EventHandler<ApplicationUnhandledExceptionEventArgs> CustomUnhandledExceptionHandler
		{
			add { Testing.CustomUnhandledExceptionHandler += value; }
			remove { Testing.CustomUnhandledExceptionHandler -= value; }
		}
	}


}
