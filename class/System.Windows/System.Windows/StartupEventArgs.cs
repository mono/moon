//
// System.Windows.StartupEventArgs
//
// <mono@novell.com>
//
// Copyright (C) 2008 Novell, Inc (http://www.novell.com)
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
using System.Windows.Interop;
using Mono;

namespace System.Windows {
	public sealed class StartupEventArgs : EventArgs {
		private Dictionary<string,string> init_params;

		internal StartupEventArgs () {}

		public IDictionary<string,string> InitParams {
			get {
				if (init_params != null)
					return init_params;

				init_params = new Dictionary<string,string> ();

				if (PluginHost.Handle != IntPtr.Zero) {
					char [] param_separator = new char [] { ',' };
					
					string param_string = NativeMethods.plugin_instance_get_init_params (PluginHost.Handle);
					// Console.WriteLine ("params = {0}", param_string);
					if (!String.IsNullOrEmpty (param_string)) {
						foreach (string val in param_string.Split (param_separator)) {
							int split = val.IndexOf ('=');
							if (split >= 0) {
								string k = val.Substring (0, split).Trim ();
								string v = val.Substring (split + 1).Trim ();
								if (k.Length > 0)
									init_params.Add (k, v);
							}
						}
					}
				}

				return init_params;
			}
		}
	}
}
