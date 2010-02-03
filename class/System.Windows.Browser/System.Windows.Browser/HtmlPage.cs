//
// System.Windows.Browser.HtmlPage class
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2007, 2009 Novell, Inc (http://www.novell.com)
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
using System.Reflection;
using System.Runtime.InteropServices;

using Mono;

namespace System.Windows.Browser{

	public static class HtmlPage {

		private static BrowserInformation browser_info;
		private static HtmlWindow window;
		private static HtmlDocument document;
		private static HtmlElement plugin;
		private static Dictionary<string, Type> scriptableTypes;
		private static Dictionary<IntPtr, object> cachedObjects;

		static HtmlPage ()
		{
			scriptableTypes = new Dictionary<string, Type> ();
			cachedObjects = new Dictionary<IntPtr, object> ();

			// we don't call RegisterScriptableObject since we're registering a private type
			ScriptableObjectWrapper wrapper = ScriptableObjectGenerator.Generate (new ScriptableObjectWrapper ());
			wrapper.Register ("services");
		}

		static internal Dictionary<string, Type> ScriptableTypes {
			get { return scriptableTypes;}
		}

		static internal Dictionary<IntPtr, object> CachedObjects {
			get { return cachedObjects;}
		}

		public static BrowserInformation BrowserInformation {
			get {
				CheckHtmlAccess();

				if (browser_info == null)
					browser_info = new BrowserInformation (Window);

				return browser_info;
			}
		}
		
		[MonoTODO ("This property should return false when we're not running in a browser")]
		public static bool IsEnabled {		
			get {
				// FIXME: add condition for out-of-browser
				return Application.Current.Host.Settings.EnableHTMLAccess;
			}
		}
		
		public static HtmlDocument Document {
			get {
				CheckHtmlAccess();

				if (document == null)
					document = HtmlObject.GetPropertyInternal<HtmlDocument> (IntPtr.Zero, "document");

				return document;
			}
		}

		static void CheckName (string name, string parameterName)
		{
			if (name == null)
				throw new ArgumentNullException (parameterName);
			if ((name.Length == 0) || (name.IndexOf ('\0') != -1))
				throw new ArgumentException (parameterName);
		}

		public static void RegisterScriptableObject (string scriptKey, object instance)
		{
			// no call to CheckHtmlAccess(); -- see DRT364
			CheckName (scriptKey, "scriptKey");
			if (instance == null)
				throw new ArgumentNullException ("instance");
			Type t = instance.GetType ();
			if (!t.IsPublic && !t.IsNestedPublic)
				throw new InvalidOperationException ("'instance' type is not public.");

			if (!ScriptableObjectGenerator.IsScriptable (t))
				throw new ArgumentException ("No public [ScriptableMember] method was found.", "instance");

			ScriptableObjectWrapper wrapper = ScriptableObjectGenerator.Generate (instance);
			wrapper.Register (scriptKey);
		}

		public static void RegisterCreateableType (string scriptAlias, Type type)
		{
			// no call to CheckHtmlAccess(); -- see DRT365
			CheckName (scriptAlias, "scriptAlias");
			if (type == null)
				throw new ArgumentNullException ("type");

			if (!ScriptableObjectGenerator.IsCreateable (type))
				throw new ArgumentException (type.ToString (), "type");

			if (ScriptableTypes.ContainsKey (scriptAlias))
				throw new ArgumentException ("scriptAlias");

			ScriptableTypes [scriptAlias] = type;
		}

		public static void UnregisterCreateableType (string scriptAlias)
		{
			CheckName (scriptAlias, "scriptAlias");

			if (!ScriptableTypes.ContainsKey (scriptAlias))
				throw new ArgumentException ("scriptAlias");

			ScriptableTypes.Remove (scriptAlias);
		}

		public static HtmlWindow Window {
			get {
				CheckHtmlAccess();

				if (window == null)
					window = HtmlObject.GetPropertyInternal<HtmlWindow> (IntPtr.Zero, "window");

				return window;
			}
		}

		public static HtmlElement Plugin {
			get {
				CheckHtmlAccess();

				if (plugin == null)
					plugin = new HtmlElement (NativeMethods.plugin_instance_get_browser_host (Mono.Xaml.XamlLoader.PluginInDomain));

				return plugin;
			}
		}

		[MonoTODO ("This property should return the value of the plugin's AllowHtmlPopupWindow property (and other stuff)")]
		public static bool IsPopupWindowAllowed {
			// TODO: the action must be coming the the user (e.g. click) and only once (per click)
			get {
				return NativeMethods.plugin_instance_get_allow_html_popup_window (Mono.Xaml.XamlLoader.PluginInDomain);
			}
		}

		public static HtmlWindow PopupWindow (Uri navigateToUri, string target, HtmlPopupWindowOptions options)
		{
			
			// TODO: documentation says this method turns off (temporarily) the browser popup blocker
			// http://msdn.microsoft.com/en-us/library/system.windows.browser.htmlpage.popupwindow(VS.95).aspx
			if (options == null)
				return (HtmlWindow) HtmlPage.Window.Invoke ("open", navigateToUri.ToString (), target);

			string targetFeatures = string.Format ("height={0},width={1},left={2},top={3},directories={4},location={5},menubar={6},resizable={7},scrollbars={8},status={9},toolbar={10}",
							       options.Height, options.Width, options.Left, options.Top,
							       options.Directories ? "yes" : "no",
							       options.Location ? "yes" : "no",
							       options.Menubar ? "yes" : "no",
							       options.Resizeable ? "yes" : "no",
							       options.Scrollbars ? "yes" : "no",
							       options.Status ? "yes" : "no",
							       options.Toolbar ? "yes" : "no"
							       );

			return (HtmlWindow) HtmlPage.Window.Invoke ("open", navigateToUri.ToString (), target, targetFeatures);
		}

		// The HTML bridge can be disable by the plugin 'enableHTMLAccess' parameter (defaults to true)
		// or can be unavailable if the plugin is hosted outside a browser (e.g. inside an IDE)
		static void CheckHtmlAccess ()
		{
			if (!IsEnabled)
				throw new InvalidOperationException ("HTML Bridge is not enabled or available.");
		}
	}
}
