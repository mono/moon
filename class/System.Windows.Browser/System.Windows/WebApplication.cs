//
// System.Windows.WebApplication class
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
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Windows.Browser;

namespace System.Windows
{
	public class WebApplication
	{
		static readonly object lockobj = new object ();
		static WebApplication current;

		public static WebApplication Current {
			get {
				if (current == null)
					lock (lockobj) {
						current = new WebApplication ();
					}
				return current;
			}
		}

		readonly IntPtr plugin_handle;
		IDictionary<string, string> startup_args;

		private WebApplication ()
		{
			object o = AppDomain.CurrentDomain.GetData ("PluginInstance");
			if (o is IntPtr) {
				plugin_handle = (IntPtr) o;

				string initParams = plugin_instance_get_init_params (plugin_handle);
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
			}

		}

		internal IntPtr PluginHandle {
			get { return plugin_handle; }
		}

		[MonoTODO]
		public void RegisterScriptableObject (string scriptKey, object instance)
		{
			if (scriptKey == null)
				throw new ArgumentNullException ("scriptKey");
			if (instance == null)
				throw new ArgumentNullException ("instance");

			if (scriptKey.Length == 0)
				throw new ArgumentException ("scriptKey");

			ScriptableObjectGenerator gen = new ScriptableObjectGenerator (instance);

			ScriptableObjectWrapper wrapper = gen.Generate (true);

			ScriptableNativeMethods.register (plugin_handle, scriptKey, wrapper.UnmanagedWrapper);
		}

		[MonoTODO]
		public IDictionary<string, string> StartupArguments {
			get { return startup_args; }
		}

		public event EventHandler<ApplicationUnhandledExceptionEventArgs> ApplicationUnhandledException;

		internal static T GetProperty<T> (IntPtr obj, string name)
		{
			return (T) GetPropertyInternal (Current.plugin_handle, obj, name);
		}

		internal static void SetProperty (IntPtr obj, string name, object value)
		{
			SetPropertyInternal (Current.plugin_handle, obj, name, value);
		}

		internal static void InvokeMethod (IntPtr obj, string name, params object [] args)
		{
			InvokeMethodInternal (Current.plugin_handle, obj, name, args);
		}

		internal static T InvokeMethod<T> (IntPtr obj, string name, params object [] args)
		{
			return (T) InvokeMethodInternal (Current.plugin_handle, obj, name, args);
		}

		[DllImport ("moonplugin")]
		static extern string plugin_instance_get_init_params (IntPtr plugin_instance);

		// note that those functions do not exist
		[DllImport ("moonplugin")]
		static extern object GetPropertyInternal (IntPtr xpp, IntPtr obj, string name);

		[DllImport ("moonplugin")]
		static extern object SetPropertyInternal (IntPtr xpp, IntPtr obj, string name, object value);

		[DllImport ("moonplugin")]
		static extern object InvokeMethodInternal (IntPtr xpp, IntPtr obj, string name, object [] args);
	}
}

