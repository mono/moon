//
// System.Windows.WebApplication class
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

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Windows.Browser;
using System.Windows.Interop;

namespace System.Windows
{
	internal class WebApplication
	{
		static readonly object lockobj = new object ();
		static WebApplication current;

		public static WebApplication Current {
			get {
				if (current == null) {
					lock (lockobj) {
						current = new WebApplication ();
					}
					current.Init ();
				}
				return current;
			}
		}

		readonly IntPtr plugin_handle;
		IDictionary<string, string> startup_args;

		static Dictionary<string, Type> scriptableTypes;
		static internal Dictionary<string, Type> ScriptableTypes {
			get { return scriptableTypes;}
		}

		static Dictionary<IntPtr, object> cachedObjects;
		static internal Dictionary<IntPtr, object> CachedObjects {
			get { return cachedObjects;}
		}

		private WebApplication ()
		{
			plugin_handle = PluginHost.Handle;

			string initParams = Mono.NativeMethods.plugin_instance_get_init_params (plugin_handle);
			if (initParams != null) {
				startup_args = new Dictionary<string,string> ();

				string[] kvs = initParams.Split (',');

				foreach (string kv in kvs) {
					string[] stuff = kv.Split ('=');
					if (stuff.Length > 1)
						startup_args[stuff[0]] = stuff[1];
					else
						startup_args[stuff[0]] = String.Empty;
				}
			}

			scriptableTypes = new Dictionary<string, Type>();
			cachedObjects = new Dictionary<IntPtr, object>();
		}

		void Init ()
		{
			RegisterScriptableObject ("services", new ScriptableObjectWrapper ());
		}

		internal IntPtr PluginHandle {
			get { return plugin_handle; }
		}

		public void RegisterScriptableObject (string scriptKey, object instance)
		{
			if (scriptKey == null)
				throw new ArgumentNullException ("scriptKey");
			if (instance == null)
				throw new ArgumentNullException ("instance");

			if (scriptKey.Length == 0)
				throw new ArgumentException ("scriptKey");

			ScriptableObjectWrapper wrapper = ScriptableObjectGenerator.Generate (instance, true);
			wrapper.Register (scriptKey);
		}

		public void RegisterCreateableType (string scriptAlias, Type type)
		{
			if (ScriptableTypes.ContainsKey (scriptAlias))
				throw new ArgumentException ("scriptAlias");
			ScriptableTypes[scriptAlias] = type;
		}

		public void UnregisterCreateableType (string scriptAlias)
		{
			if (ScriptableTypes.ContainsKey (scriptAlias))
				ScriptableTypes.Remove (scriptAlias);
		}	

		[MonoTODO]
		public IDictionary<string, string> StartupArguments {
			get { return startup_args; }
		}
	}
}

