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

namespace Mono.Moonlight.UnitTesting
{
	public static class Testing
	{
		static MoonLogProvider moonlog;
		static UIElement test_page;

		public static UIElement CreateTestPage (Application app)
		{
			UnitTestSettings settings = new UnitTestSettings ();
			
			app.UnhandledException += Application_UnhandledException;
			
			moonlog = new MoonLogProvider ();
			settings.TestHarness = new Microsoft.Silverlight.Testing.UnitTesting.Harness.UnitTestHarness ();
			settings.TestAssemblies.Add (app.GetType ().Assembly);
			UnitTestSystem.PrepareCustomLogProviders (settings);
			settings.LogProviders.Add (moonlog);
			if (!string.IsNullOrEmpty (HtmlPage.Document.DocumentUri.Query))
				settings.TagExpression = HtmlPage.Document.DocumentUri.Query.Substring (1);
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
				if (!test_page.CheckAccess ())
					test_page.Dispatcher.BeginInvoke (ShutdownHarness);
				else
					HtmlPage.Window.Eval ("try { ShutdownHarness (); } catch (e) { }");
			} catch (Exception ex) {
				Console.WriteLine (ex.Message);
			}
		}
		
		private static void Application_UnhandledException(object sender, ApplicationUnhandledExceptionEventArgs e)
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
