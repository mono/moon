//
// System.Windows.Browser.BrowserInformation class
//
// Authors:
//	Atsushi Enomoto  <atsushi@ximian.com>
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
using System;
using System.Globalization;
using System.Runtime.InteropServices;
using System.Threading;

namespace System.Windows.Browser
{
	public sealed class BrowserRuntimeSettings
	{
		CultureInfo culture;
		CultureInfo ui_culture;

		bool debug, html, httpnet, script;

		[DllImport ("moonplugin")]
		static extern void plugin_instance_get_browser_runtime_settings (out bool debug, out bool html_access,
																		 out bool httpnet_access, out bool script_access);

		internal BrowserRuntimeSettings ()
		{
			culture = Thread.CurrentThread.CurrentCulture;
			ui_culture = Thread.CurrentThread.CurrentUICulture;
			plugin_instance_get_browser_runtime_settings (out debug, out html, out httpnet, out script);
		}

		public CultureInfo Culture {
			get { return culture; }
		}

		public bool EnableDebugging {
			get { return debug; }
		}

		public bool EnableHtmlAccess {
			get { return html; }
		}

		public bool EnableHttpNetworkAccess {
			get { return httpnet; }
		}

		public bool EnableScriptAccess {
			get { return script; }
		}

		public CultureInfo UICulture {
			get { return ui_culture; }
		}
	}
}

