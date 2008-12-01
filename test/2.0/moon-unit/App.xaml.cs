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

namespace Mono.Moonlight.UnitTesting
{
	public partial class App : Application
	{
		MoonLogProvider moonlog;
		UIElement test_page;

		public App()
		{
			this.Startup += this.Application_Startup;
			this.Exit += this.Application_Exit;
			this.UnhandledException += this.Application_UnhandledException;

			InitializeComponent();
		}

		private void Application_Startup(object sender, StartupEventArgs e)
		{
			UnitTestSettings settings = new UnitTestSettings ();
			moonlog = new MoonLogProvider ();
			settings.TestHarness = new Microsoft.Silverlight.Testing.UnitTesting.Harness.UnitTestHarness ();
			settings.TestAssemblies.Add (Assembly.GetExecutingAssembly ());
			UnitTestSystem.PrepareCustomLogProviders (settings);
			settings.LogProviders.Add (moonlog);
			test_page = UnitTestSystem.CreateTestPage (settings);
			settings.TestHarness.TestHarnessCompleted += new EventHandler<TestHarnessCompletedEventArgs> (Harness_Completed);
			this.RootVisual = test_page;
		}

		void Harness_Completed (object sender, TestHarnessCompletedEventArgs e)
		{
			try {
				if (moonlog.ProcessCompleted (e, ShutdownHarness)) {
					ShutdownHarness ();
				}
			} catch (Exception ex) {
				Console.WriteLine (ex.Message);
			}
		}

		void ShutdownHarness ()
		{
			try {
				if (!test_page.CheckAccess ())
					test_page.Dispatcher.BeginInvoke (ShutdownHarness);
				else
					HtmlPage.Window.Eval ("try { ShutdownHarness (); } catch (e) { }");
			} catch (Exception ex) {
				Console.WriteLine (ex.Message);
			}
		}

		private void Application_Exit(object sender, EventArgs e)
		{

		}
		private void Application_UnhandledException(object sender, ApplicationUnhandledExceptionEventArgs e)
		{
			// If the app is running outside of the debugger then report the exception using
			// the browser's exception mechanism. On IE this will display it a yellow alert 
			// icon in the status bar and Firefox will display a script error.
			if (!System.Diagnostics.Debugger.IsAttached)
			{

				// NOTE: This will allow the application to continue running after an exception has been thrown
				// but not handled. 
				// For production applications this error handling should be replaced with something that will 
				// report the error to the website and stop the application.
				e.Handled = true;

				try
				{
					string errorMsg = e.ExceptionObject.Message + e.ExceptionObject.StackTrace;
					errorMsg = errorMsg.Replace('"', '\'').Replace("\r\n", @"\n");

					System.Windows.Browser.HtmlPage.Window.Eval("throw new Error(\"Unhandled Error in Silverlight 2 Application " + errorMsg + "\");");
				}
				catch (Exception)
				{
				}
			}
		}
	}


}
