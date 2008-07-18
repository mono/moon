//
// moonlight.cs
//
// Author:
//   Miguel de Icaza (miguel@ximian.com)
//   Jackson Harper  (jackson@ximian.com)
//   Rolf Bjarne Kvinge  (RKvinge@novell.com)
//   Jb Evain        (jbevain@novell.com)
//
// Copyright 2007 Novell, Inc.
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

using Mono;

using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Windows;

namespace Moonlight {

	/// <summary>
	///   A class that provides basic services to launch the managed Silvelright code
	/// </summary>
	/// <remarks>
	///   This class is used to load a XAML file that contains
	///   managed code or a XAP application into a new AppDomain. 
	///
	///   A new AppDomain is created to host the content, so each new
	///   file or application is isoalted by AppDomains.
	/// </remarks>
	
	public class ApplicationLauncher {

		delegate void PluginUnloadCallback (IntPtr plugin);

		[DllImport ("moonplugin")]
		static extern void plugin_set_unload_callback (IntPtr plugin, PluginUnloadCallback puc);

		//
		// Tracks a list of callbacks to invoke on domain
		// unload, indexed by the plugin handle
		//
		static Dictionary<IntPtr, PluginUnloadCallback> callbacks = new Dictionary<IntPtr, PluginUnloadCallback> ();

		//
		// Tracks all the created domains, it is indexed by
		// the plugin handle (IntPtr value).
		//
		static Dictionary<IntPtr, AppDomain> domains = new Dictionary<IntPtr, AppDomain> ();

		/// <summary>
		///   Creates a new Loader for a XAML file.
		/// </summary>
		public static Loader CreateXamlLoader (IntPtr native_loader, IntPtr plugin, IntPtr surface, string filename, string contents)
		{
			try {
				return new Loader (GetDomain (plugin), native_loader, plugin, surface, filename, contents);
			} catch (Exception ex) {
				Console.Error.WriteLine ("Loader::CreateXamlLoader: Unable to create managed xaml loader: {0}", ex.Message);
				return null;
			}
		}

		/// <summary>
		///   Loads the Application contained in the XAP file into the current AppDomain
		/// </summary>
		public static bool CreateApplication (IntPtr plugin, IntPtr surface, string xapFile)
		{
			try {
				AppDomain d = GetDomain (plugin);
				
				object rl = Helper.CreateInstanceAndUnwrap (d, typeof (DependencyObject).Assembly.FullName, "System.Windows.XapHackProxyImpl");
				bool v = (bool) rl.GetType ().GetMethod ("Setup").Invoke (rl, new object [] { plugin, surface, xapFile });

				return v;
			} catch (Exception e){
				Console.WriteLine (e);
			}

			return true;
		}
				

		/// <summary>
		///   Shutsdown the Application contained in the XAP file in the current AppDomain
		/// </summary>
		public static void DestroyApplication (IntPtr plugin)
		{
			try {
				AppDomain d = GetDomain (plugin);
				
				Console.WriteLine (typeof (Application).Assembly.FullName);
				object rl = Helper.CreateInstanceAndUnwrap (d, typeof (DependencyObject).Assembly.FullName, "System.Windows.XapHackProxyImpl");
				rl.GetType ().GetMethod ("Terminate").Invoke (rl, new object [0]);
			} catch (Exception e){
				Console.WriteLine (e);
			}
		}
		
		static AppDomain CreateDomain (IntPtr plugin)
		{
			AppDomain domain = Helper.CreateDomain (plugin);
			if (plugin == IntPtr.Zero)
				return null;

			// Set the plugin unload callback so that we get a chance to unload the domain when 
			// the plugin unloads.
			PluginUnloadCallback callback = new PluginUnloadCallback (UnloadDomain);
			plugin_set_unload_callback (plugin, callback);
			callbacks.Add (plugin, callback);
			return domain;
		}

		public static AppDomain GetDomain (IntPtr plugin)
		{
			AppDomain domain;
			if (domains.TryGetValue (plugin, out domain))
				return domain;

			domain = CreateDomain (plugin);
			domains.Add (plugin, domain);

			return domain;
		}

		static void UnloadDomain (IntPtr plugin, AppDomain domain)
		{
			Console.WriteLine ("Moonlight.Loader::UnloadDomain ({0}): {1}.", plugin, domain.FriendlyName);

			domains.Remove (plugin);
			callbacks.Remove (plugin);

			Helper.UnloadDomain (domain);
		}

		static void UnloadDomain (IntPtr plugin)
		{
			try {
				AppDomain domain;
				if (!domains.TryGetValue (plugin, out domain))
					return;

				UnloadDomain (plugin, domain);
			} catch (Exception ex) {
				Console.Error.WriteLine ("Moonlight.Loader::UnloadDomain ({0}): Unable to unload domain:\n {1}.", plugin, ex);
			}
		}
	}

	//
	// Used for XAML Loading
	//
	public class Loader {
		//
		// Points to the XamlLoader for this plugin, this is a
		// MarshalByRef object which points to the actual
		// loader on the individual domain.
		//
		// Mono.Xaml.XamlLoader rl;
		object rl;

		// [DONE] 1. Load XAML file 
		// 2. Make sure XAML file exposes a few new properites:
		//    a. Loaded  (this is the method to call)
		//    b. x:Class (it specifies the assembly to request from the server, and the class to load).
		// 3. Request the browser to download the files
		// 4. From the assembly, lookup the specified class
		// 5. From the class lookup the specified method
		// 6. Run the method
		// 7. If none of those properties are there, we can return

		public Loader (AppDomain domain, IntPtr native_loader, IntPtr plugin, IntPtr surface, string filename, string contents)
		{
			//
			// Mono.Xaml.Loader is defined in agmono, the actual
			// instance of Mono.Xaml.ManagedXamlLoader is defined
			// as an internal class in agclr's Mono namespace
			//
			rl = Helper.CreateInstanceAndUnwrap (
				domain, typeof (DependencyObject).Assembly.FullName, "Mono.Xaml.ManagedXamlLoader");

			rl.GetType ().GetMethod ("Setup").Invoke (rl, new object [] { native_loader, plugin, surface, filename, contents });

			//rl.Setup (native_loader, plugin, surface, filename, contents);
		}
	}

	class Moonlight {
		static int count;
		
		static void Main ()
		{
			Console.WriteLine ("Running Moonlight.cs, boot instance: {0}", count++);
			Console.WriteLine ("  mscorlib: {0}", typeof (object).Module.FullyQualifiedName);
			Console.WriteLine ("  agclr: {0}", typeof (DependencyObject).Module.FullyQualifiedName);
			Console.WriteLine ("  agmono: {0}", typeof (Helper).Module.FullyQualifiedName);
		}
	}
}
