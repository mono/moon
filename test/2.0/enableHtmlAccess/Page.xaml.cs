using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Browser;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

namespace enableHtmlAccess
{
	// Sadly SL Unit tests don't work with EnableHTMLAccess == false

	public partial class Page : Canvas
	{
		private Color result = Colors.Green;

		public Page()
		{
			InitializeComponent();
			
			try {
				// the same application test EnableHTMLAccess == true and false using different web pages
				// I'd like to read the web page title to adjust automatically but well, this is what this
				// test is all about ;-)
				if (Application.Current.Host.Settings.EnableHTMLAccess) {
					EnableHTMLAccess_True ();
				} else {
					EnableHTMLAccess_False ();
				}
			}
			catch (Exception e) {
				Fail (String.Format ("Unexcepted exception: '{0}'", e));
				result = Colors.Red;
			}
			finally {
				Info ("END");
				Background = new SolidColorBrush (result);
			}
		}

		private void Success (string message)
		{
			AddMessage (String.Format ("OK: {0}", message));
		}

		private void Fail (string message)
		{
			AddMessage (message);
			result = Colors.Red;
		}

		private void Info (string message)
		{
			AddMessage (message);
		}

		private void AddMessage (string s)
		{
			TextBlock tb = new TextBlock ();
			tb.Text = s;
			info.Children.Add (tb);
		}

		public void IsFalse (bool value, string message)
		{
			if (value) {
				Fail (String.Format ("IsFalse failed: {0},", message));
				result = Colors.Red;
			} else {
				Success (message);
			}
		}

		public void IsTrue (bool value, string message)
		{
			if (!value) {
				Fail (String.Format ("IsTrue failed: {0},", message));
			} else {
				Success (message);
			}
		}

		public void IsNotNull (object obj, string message)
		{
			if (obj == null) {
				Fail (String.Format ("IsNotNull failed: {0},", message));
			} else {
				Success (message);
			}
		}

		public delegate void TestCode ();

		public void Throws<TException> (TestCode code, string message) where TException : Exception
		{
			Type expected_exception = typeof (TException);
			bool failed = false;
			try {
				code ();
				failed = true;
			} catch (Exception ex) {
				if (ex.GetType () != expected_exception) { 
					Fail (string.Format ("Expected '{0}', got '{1}'. {2}", expected_exception.FullName, ex.GetType ().FullName, message));
					return;
				}
			}
			if (failed)
				Fail (string.Format ("Expected '{0}', but got no exception. {1}", expected_exception.FullName, message));
			else
				Success (message);
		}

		public void EnableHTMLAccess_True ()
		{
			Info ("EnableHTMLAccess == true");
			// "real" values tested under the "normal" unit tests
			IsNotNull (HtmlPage.BrowserInformation, "HtmlPage.BrowserInformation");
			IsNotNull (HtmlPage.Document, "HtmlPage.Document");
			IsTrue (HtmlPage.IsEnabled, "HtmlPage.IsEnabled");
			IsFalse (HtmlPage.IsPopupWindowAllowed, "HtmlPage.IsPopupWindowAllowed");
			IsNotNull (HtmlPage.Plugin, "HtmlPage.Plugin");
			IsNotNull (HtmlPage.Window, "HtmlPage.Window");

			CheckSettingsAccess ();
		}

		public void EnableHTMLAccess_False ()
		{
			Info ("EnableHTMLAccess == false");

			// according to "Security Settings in HTML Bridge" only four properties are affected
			// http://msdn.microsoft.com/en-us/library/cc645023(VS.95).aspx
			Throws<InvalidOperationException> (delegate {
				Console.WriteLine (HtmlPage.BrowserInformation);
			}, "HtmlPage.BrowserInformation");
			Throws<InvalidOperationException> (delegate {
				Console.WriteLine (HtmlPage.Document);
			}, "HtmlPage.Document");
			IsFalse (HtmlPage.IsEnabled, "HtmlPage.IsEnabled");
			IsFalse (HtmlPage.IsPopupWindowAllowed, "HtmlPage.IsPopupWindowAllowed");
			Throws<InvalidOperationException> (delegate {
				Console.WriteLine (HtmlPage.Plugin);
			}, "HtmlPage.Plugin");
			Throws<InvalidOperationException> (delegate {
				Console.WriteLine (HtmlPage.Window);
			}, "HtmlPage.Window");

			// Settings are not affected by EnableHTMLAccess
			CheckSettingsAccess ();
		}

		public void CheckSettingsAccess ()
		{
			Settings s = Application.Current.Host.Settings;
			IsFalse (s.EnableFrameRateCounter, "Settings.EnableFrameRateCounter");
			s.EnableFrameRateCounter = true;
// not yet implemented in Moonlight
//			IsTrue (s.EnableFrameRateCounter, "Settings.EnableFrameRateCounter/Set");
			IsFalse (s.EnableRedrawRegions, "Settings.EnableRedrawRegions");
			s.EnableRedrawRegions = true;
// not yet implemented in Moonlight
//			IsTrue (s.EnableRedrawRegions, "Settings.EnableRedrawRegions/Set");
			IsFalse (s.MaxFrameRate <= 0, "Settings.MaxFrameRate");
			s.MaxFrameRate = 1;
			IsTrue (s.MaxFrameRate == 1, "Settings.MaxFrameRate/Set");
			IsFalse (s.Windowless, "Settings.Windowless");
		}
	}
}
