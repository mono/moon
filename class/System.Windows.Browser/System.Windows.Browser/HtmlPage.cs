//
// System.Windows.Browser.HtmlPage class
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2007 Novell, Inc (http://www.novell.com)
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

using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Security;

using Mono;

namespace System.Windows.Browser{

	public static class HtmlPage {

		private static BrowserInformation browser_info;
		private static HtmlWindow window;
		private static HtmlDocument document;
		private static HtmlElement plugin;
		static List<string> scriptableObjects;

		static HtmlPage ()
		{
			scriptableObjects = new List<string>();
		}

		public static BrowserInformation BrowserInformation {
			get {
				if (browser_info == null)
					browser_info = new BrowserInformation (Window);

				return browser_info;
			}
		}
		
		[MonoTODO ("This method should return false when we're not running in a browser.")]
		public static bool IsEnabled {		
			[SecuritySafeCritical]
			get {
				return true;
			}
		}
		
		public static HtmlDocument Document {
			[SecuritySafeCritical]
			get {
				if (document == null)
					document = HtmlObject.GetPropertyInternal<HtmlDocument> (IntPtr.Zero, "document");

				return document;
			}
		}

		public static void RegisterScriptableObject (string scriptKey, object instance)
		{
			WebApplication.Current.RegisterScriptableObject (scriptKey, instance);
		}
		
		public static void RegisterCreateableType (string scriptAlias, Type type)
		{
			WebApplication.Current.RegisterCreateableType (scriptAlias, type);
		}

		public static void UnregisterCreateableType (string scriptAlias)
		{
			WebApplication.Current.UnregisterCreateableType (scriptAlias);
		}

		public static HtmlWindow Window {
			get {
				if (window == null)
					window = HtmlObject.GetPropertyInternal<HtmlWindow> (IntPtr.Zero, "window");

				return window;
			}
		}

		public static HtmlElement Plugin {
			[SecuritySafeCritical]
			get {
				if (plugin == null)
					plugin = new HtmlElement (NativeMethods.plugin_instance_get_host (Mono.Xaml.XamlLoader.PluginInDomain));

				return plugin;
			}
		}

		public static bool IsPopupWindowAllowed {
			get { throw new System.NotImplementedException (); }
		}

		public static HtmlWindow PopupWindow (Uri navigateToUri, string target, HtmlPopupWindowOptions options)
		{
			throw new System.NotImplementedException ();
		}
	}
}
