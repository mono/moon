//
// TestApplication.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2009 Novell, Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

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
using Microsoft.Silverlight.Testing.UnitTesting.Harness;

namespace Mono.Moonlight.UnitTesting
{
	public static class Testing
	{
		static MoonLogProvider moonlog;
		static UIElement test_page;
		static UnitTestSettings settings;
		static UnitTestHarness harness;

		// for tests which test that a unhandled application exception is raised,
		// we need to disable the harness' handling of the unhandled application exception,
		// otherwise those tests will always fail. So we have a custom unhandled exception
		// event, which, if anybody is listening is raised, and if nobody is listening
		// the harness handles it.
		static int custom_unhandled_exception_handler_counter;
		static EventHandler<ApplicationUnhandledExceptionEventArgs> custom_unhandled_exception_handler;

		public static event EventHandler<ApplicationUnhandledExceptionEventArgs> CustomUnhandledExceptionHandler
		{
			add { 
				custom_unhandled_exception_handler += value;
				custom_unhandled_exception_handler_counter++;
				harness.InterceptAllExceptions = false;
			}
			remove {
				custom_unhandled_exception_handler -= value;
				custom_unhandled_exception_handler_counter--;
				harness.InterceptAllExceptions = custom_unhandled_exception_handler_counter == 0;
			}
		}

		public static void EnablePluginErrors ()
		{
			HtmlPage.Window.Eval ("enablePluginErrors ();");
		}

		public static void DisablePluginErrors ()
		{
			HtmlPage.Window.Eval ("disablePluginErrors ();");
		}

		public static UIElement CreateTestPage (Application app)
		{
			settings = new UnitTestSettings ();
			
			app.UnhandledException += Application_UnhandledException;
			
			moonlog = new MoonLogProvider ();
			harness = new Microsoft.Silverlight.Testing.UnitTesting.Harness.UnitTestHarness ();
			settings.TestHarness = harness; 
			settings.TestAssemblies.Add (app.GetType ().Assembly);
			UnitTestSystem.PrepareCustomLogProviders (settings);
			settings.LogProviders.Add (moonlog);
			settings.RuntimeVersion = Int32.Parse (Deployment.Current.RuntimeVersion.Split('.')[0]);
            // Silverlight thinks HtmlPage.Document.DocumentUri.Query is empty
            // so lets just manually parse instead. This allows tagging to work on SL.
			if (HtmlPage.Document.DocumentUri.OriginalString.IndexOf ('?') > 0) {
				settings.TagExpression = HtmlPage.Document.DocumentUri.OriginalString.Substring (HtmlPage.Document.DocumentUri.OriginalString.IndexOf ('?') + 1);
				if (settings.TagExpression.IndexOf ('#') > 0)
					settings.TagExpression = settings.TagExpression.Remove (settings.TagExpression.IndexOf ('#'));
				
				List<string> exps = new List<string> (settings.TagExpression.Split ('&'));
				for (int i = exps.Count - 1; i >= 0; i--) {
					if (exps [i].StartsWith ("version=") || exps [i].StartsWith ("bot_mode=")) {
						exps.RemoveAt (i);
					}
				}
				settings.TagExpression = string.Join ("&", exps.ToArray ());

			}
			test_page = UnitTestSystem.CreateTestPage (settings);
			
			settings.TestHarness.TestHarnessCompleted += new EventHandler<TestHarnessCompletedEventArgs> (Harness_Completed);
			
			return test_page;
		}
		
		private static void Harness_Completed (object sender, TestHarnessCompletedEventArgs e)
		{
			try {
				if (moonlog.ProcessCompleted (e, ShutdownHarness)) {
					ShutdownHarness ();
				}
			} catch (Exception ex) {
				Console.WriteLine (ex.Message);
			}
		}

		private static void ShutdownHarness ()
		{
			try {
				if (!test_page.CheckAccess ()) {
					Console.WriteLine ("ShutdownHarness (): Invoking");
					test_page.Dispatcher.BeginInvoke (ShutdownHarness);
				} else {
					Console.WriteLine ("ShutdownHarness (): Evaling");
					HtmlPage.Window.Eval ("try { ShutdownHarness (); } catch (e) { alert (e); }");
				}
			} catch (Exception ex) {
				Console.WriteLine (ex.Message);
			}
		}
		
		private static void Application_UnhandledException(object sender, ApplicationUnhandledExceptionEventArgs e)
		{
			if (custom_unhandled_exception_handler_counter != 0) {
				custom_unhandled_exception_handler (sender, e);
				return;
			}

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
