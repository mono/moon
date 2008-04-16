//
// System.Windows.Browser.BrowserInformation class
//
// Authors:
//	Sebastien Pouliot  <sebastien@ximian.com>
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

using System.Runtime.InteropServices;

namespace System.Windows.Browser {

	public sealed class BrowserInformation {

		private string name;
		private string version;
		private string platform;
		private string user_agent;
		private bool cookie_enabled;

		[DllImport ("moonplugin")]
		static extern void plugin_instance_get_browser_information (IntPtr plugin_handle,
			out IntPtr name, out IntPtr version, out IntPtr platform, out IntPtr userAgent,
			[MarshalAs(UnmanagedType.I1)] ref bool cookieEnabled);

		internal BrowserInformation ()
		{
			IntPtr name, version, platform, user_agent;
			plugin_instance_get_browser_information (WebApplication.Current.PluginHandle,
								 out name, out version, out platform, out user_agent,
								 ref cookie_enabled);
			this.name = Marshal.PtrToStringAnsi (name);
			this.version = Marshal.PtrToStringAnsi (version);
			this.platform = Marshal.PtrToStringAnsi (platform);
			this.user_agent = Marshal.PtrToStringAnsi (user_agent);
		}

		public BrowserInformation (string name, string version, string platform, string userAgent, bool cookieEnabled)
		{
			this.name = name;
			this.version = version;
			this.platform = platform;
			this.user_agent = userAgent;
			this.cookie_enabled = cookieEnabled;
		}

		public string BrowserVersion {
			get { return version; }
		}

		public bool CookiesEnabled {
			get { return cookie_enabled; }
		}

		public string Name {
			get { return name; }
		}

		public string Platform {
			get { return platform; }
		}

		public string UserAgent {
			get { return user_agent; }
		}
	}
}
