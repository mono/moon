//
// Deployment.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2008-2010 Novell, Inc.
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
using System.IO;
using System.Net;
using System.Net.Browser;
using System.Reflection;
using System.Threading;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Interop;
using Mono;
using Mono.Xaml;

namespace System.Windows {

	public sealed partial class Deployment : DependencyObject {
		Types types;
		List<Assembly> assemblies;
		Assembly entry_point_assembly;
		string xap_dir;
		int pending_assemblies;
		
		static List<Action> shutdown_actions = new List<Action> ();
		static bool is_shutting_down;
		
#if NET_2_1
		static Deployment ()
		{
			// set the default to the browser stack
			WebRequest.RegisterDefaultStack (WebRequestCreator.BrowserHttp);
		}
#endif

		internal static bool IsShuttingDown {
			get { return is_shutting_down; }
		}
		
		internal static bool Shutdown ()
		{
			lock (shutdown_actions) {
				is_shutting_down = true;
				while (shutdown_actions.Count > 0) {
					Action a;
					a = shutdown_actions [shutdown_actions.Count - 1];
					shutdown_actions.RemoveAt (shutdown_actions.Count - 1);
					a ();
				}
			}
			// Console.WriteLine ("Deployment.Shutdown ()");
			return true;
		}
		
		/* 
		 * thread-safe
		 * return false if we're shutting down already.
		 */
		internal static bool QueueForShutdown (Action a)
		{
			lock (shutdown_actions) {
				if (is_shutting_down)
					return false;
				shutdown_actions.Add (a);
			}
			
			return true;
		}
		
		/* thread-safe */
		internal static void UnqueueForShutdown (Action a)
		{
			lock (shutdown_actions) {
				shutdown_actions.Remove (a);
			}
		}
		
		public static Deployment Current {
			get {
				IntPtr dep = NativeMethods.deployment_get_current ();
				return NativeDependencyObjectHelper.Lookup (Kind.DEPLOYMENT, dep) as Deployment;
			}

			internal set {
				NativeMethods.deployment_set_current (value == null ? IntPtr.Zero : value.native);
			}
		}
	
		public static void RegisterAssembly (Assembly assembly)
		{
			throw new System.NotImplementedException ();
		}
		
		public static void SetCurrentApplication (Application application)
		{
			if ((application == null && Application.IsCurrentSet) ||
				(application != null && Application.IsCurrentSet && Application.Current != application))
				Application.Current.Free ();

			NativeMethods.deployment_set_current_application (Current.native, application == null ? IntPtr.Zero : application.NativeHandle);
		}

		internal Types Types {
			get {
				if (types == null)
					types = new Types (NativeMethods.deployment_get_types (native));
				return types;
			}
		}

		internal List<Assembly> Assemblies {
			get {
				return assemblies;
			}
		}

		internal string XapDir {
			get {
				return xap_dir;
			}
			set {
				xap_dir = value;
			}
		}
		
		internal Assembly EntryAssembly {
			get {
				return entry_point_assembly;
			}
			set {
				entry_point_assembly = value;
			}
		}
		
		internal void InitializePluginHost (IntPtr plugin) {
			if (plugin != IntPtr.Zero)
				PluginHost.SetPluginHandle (plugin);
		}

		void ExtractXap (string xapPath)
		{
			xap_dir = NativeMethods.xap_unpack (xapPath);
			if (xap_dir == null)
				throw new MoonException (2103, "Invalid or malformed application: Check manifest");
		}

		void CompareRuntimeVersions ()
		{
			int[] versions = new int[4];
			string[] version_strings = RuntimeVersion.Split ('.');

			for (int i = 0; i < version_strings.Length; i ++) {
				if (!Int32.TryParse (version_strings[i], out versions[i]))
					throw new MoonException (2105, "invalid RuntimeVersion");
			}

			if (versions[0] == 2) {
				/* SL2 accepts anything newer than 2.0.30524.0 */
				if (version_strings.Length > 1)
					if (versions[1] != 0)
						throw new MoonException (2106, "Failed to load the application. It was built with an obsolete version of Silverlight");

				if (version_strings.Length > 2)
					if (versions[2] < 30524)
						throw new MoonException (2106, "Failed to load the application. It was built with an obsolete version of Silverlight");
			}
			else if (versions[0] == 3) {
				// we don't actually validate any 3.0
				// runtime versions yet since there's
				// no telling what it'll be at RTM.
			}
			else {
				throw new MoonException (2105, "invalid RuntimeVersion");
			}
		}

		void ReadManifest ()
		{
			XamlLoader loader = XamlLoader.CreateManagedXamlLoader (null, Surface.Native, PluginHost.Handle);
			string app_manifest = Path.Combine (XapDir, "appmanifest.xaml");

			if (!File.Exists (app_manifest))
				throw new MoonException(2103, "Invalid or malformed application: Check manifest");

			string app_manifest_contents;

			using (StreamReader r = new StreamReader (app_manifest))
				app_manifest_contents = r.ReadToEnd();

			try {
				loader.Hydrate (Value.FromObject (this), app_manifest_contents);
			}
			catch (Exception e) {
				throw new MoonException (7016, e.Message);
			}

			if (RuntimeVersion == null)
				throw new MoonException (2105, "No RuntimeVersion specified in AppManifest");

			CompareRuntimeVersions ();

			if (EntryPointAssembly == null)
				throw new MoonException (2103, "Invalid or malformed application: Check manifest");

			if (EntryPointType == null)
				throw new Exception ("No entrypoint defined in the AppManifest.xaml");

			try {
				// this is a "set once" property, we set it to its default in case it was not part of the manifest
				ExternalCallersFromCrossDomain = CrossDomainAccess.NoAccess;
			}
			catch (ArgumentException) {
				// a value was already set (should be quite rare)
			}
		}

		internal bool InitializeDeployment (string culture, string uiCulture) {
			TerminateCurrentApplication ();

			try {
				if (culture != null)
					Thread.CurrentThread.CurrentCulture = new CultureInfo (culture);
				if (uiCulture != null)
					Thread.CurrentThread.CurrentUICulture = new CultureInfo (uiCulture);
			}
			catch (Exception e) {
				// 2105 is required by the Localization drt (#352)
				throw new MoonException (2105, e.Message);
			}

			EntryPointType = "System.Windows.Application";
			EntryPointAssembly = typeof (Application).Assembly.GetName ().Name;
			EntryAssembly = typeof (Application).Assembly;
			return LoadAssemblies ();
		}
			
		internal bool InitializeDeployment (IntPtr plugin, string xapPath, string culture, string uiCulture) {
			TerminateCurrentApplication ();

			try {
				if (culture != null)
					Thread.CurrentThread.CurrentCulture = new CultureInfo (culture);
				if (uiCulture != null)
					Thread.CurrentThread.CurrentUICulture = new CultureInfo (uiCulture);
			}
			catch (Exception e) {
				// 2105 is required by the Localization drt (#352)
				throw new MoonException (2105, e.Message);
			}

			InitializePluginHost (plugin);
			ExtractXap (xapPath);
			ReadManifest ();

			NativeMethods.deployment_set_is_loaded_from_xap (native, true);
			return LoadAssemblies ();
		}

		// note: throwing MoonException from here is ok since this code is called (sync) from the plugin
		internal bool LoadAssemblies ()
		{
			assemblies = new List <Assembly> ();
			assemblies.Add (typeof (Application).Assembly);
			
			pending_assemblies = Parts != null ? Parts.Count : 0;

			for (int i = 0; i < ExternalParts.Count; i++) {
				ExtensionPart ext = ExternalParts[i] as ExtensionPart;

				if (ext != null) {
					try {
						// TODO These ExternalPart assemblies should really be placed in
						// a global long term cache but simply make sure we load them for now
						Console.WriteLine ("Attempting To Load ExternalPart {0}", ext.Source);

						Uri uri = ext.Source;

						if (!uri.IsAbsoluteUri) {
							string xap = NativeMethods.plugin_instance_get_source_location (PluginHost.Handle);
							uri = new Uri (new Uri (xap), uri);
						}

						HttpWebRequest req = (HttpWebRequest) WebRequest.Create (uri);
						req.BeginGetResponse (AssemblyGetResponse, req);
						
						pending_assemblies++;
					} catch (Exception e) {
						// FIXME this is probably not the right exception id and message 
						// but at least pass it up
						throw new MoonException (2105, string.Format ("Error while loading the '{0}' ExternalPart: {1}", ext.Source, e.Message));
					}
				}
			}

			for (int i = 0; Parts != null && i < Parts.Count; i++) {
				var source = Parts [i].Source;

				try {
					bool try_downloading = false;
					string canon = Helper.CanonicalizeAssemblyPath (source);
					string filename = Path.GetFullPath (Path.Combine (XapDir, canon));
					// note: the content of the AssemblyManifest.xaml file is untrusted
					if (filename.StartsWith (XapDir)) {
						try {
							Assembly asm = Assembly.LoadFrom (filename);
							AssemblyRegister (asm);
							if (pending_assemblies == 0)
								return true;
						} catch (FileNotFoundException) {
							try_downloading = true;
						}
					} else {
						// we can hit a Part with a relative URI starting with a '/' which fails the above test
						try_downloading = true;
					}

					if (!try_downloading)
						continue;

					Uri uri = new Uri (source, UriKind.RelativeOrAbsolute);
					// WebClient deals with relative URI but HttpWebRequest does not
					// but we need the later to detect redirection
					if (!uri.IsAbsoluteUri) {
						string xap = NativeMethods.plugin_instance_get_source_location (PluginHost.Handle);
						uri = new Uri (new Uri (xap), uri);
					}
					HttpWebRequest req = (HttpWebRequest) WebRequest.Create (uri);
					req.BeginGetResponse (AssemblyGetResponse, req);
				} catch (Exception e) {
					throw new MoonException (2105, string.Format ("Error while loading the '{0}' assembly : {1}", source, e.Message));
				}
			}

			// unmanaged (JS/XAML only) applications can go directly to create the application
			return pending_assemblies == 0 ? CreateApplication () : true;
		}

		// note: throwing MoonException from here is NOT ok since this code is called async
		// and the exception won't be reported, directly, to the caller
		void AssemblyGetResponse (IAsyncResult result)
		{
			try {
				WebRequest wreq = (WebRequest) result.AsyncState;
				HttpWebResponse wresp = (HttpWebResponse) wreq.EndGetResponse (result);
				if (wresp.StatusCode != HttpStatusCode.OK) {
					EmitError (2105, String.Format ("Error while downloading the '{0}'.", wreq.RequestUri));
					return;
				}

				if (wresp.ResponseUri != wreq.RequestUri) {
					EmitError (2105, "Redirection not allowed to download assemblies.");
					return;
				}

				Stream responseStream = wresp.GetResponseStream ();

				AssemblyPart a = new AssemblyPart ();
				Assembly asm = a.Load (responseStream);

				if (asm == null) {
					// it's not a valid assembly, try to unzip it.
					MemoryStream ms = new MemoryStream ();
					ManagedStreamCallbacks source_cb;
					ManagedStreamCallbacks dest_cb;
					StreamWrapper source_wrapper;
					StreamWrapper dest_wrapper;

					responseStream.Seek (0, SeekOrigin.Begin);

					source_wrapper = new StreamWrapper (responseStream);
					dest_wrapper = new StreamWrapper (ms);

					source_cb = source_wrapper.GetCallbacks ();
					dest_cb = dest_wrapper.GetCallbacks ();

					// the zip files I've come across have a single file in them, the
					// dll.  so we assume that any/every zip file will contain a single
					// file, and just get the first one from the zip file directory.
					if (NativeMethods.managed_unzip_stream_to_stream_first_file (ref source_cb, ref dest_cb)) {
						ms.Seek (0, SeekOrigin.Begin);
						asm = a.Load (ms);

						if (asm == null) {
							// if we still fail after treating it like a zip, give up
							EmitError (2105, String.Format ("Error while loading '{0}'.", wreq.RequestUri));
						}

						ms.Close ();
					}
				}
				wresp.Close ();

				if (asm != null)
					Dispatcher.BeginInvoke (new AssemblyRegistration (AssemblyRegister), asm);
			}
			catch (Exception e) {
				// we need to report everything since any error means CreateApplication won't be called
				Dispatcher.BeginInvoke (() => {
					EmitError (2103, e.ToString ());
				});
			}
		}

		delegate void AssemblyRegistration (Assembly asm);

		// note: only access 'assemblies' from the main thread
		void AssemblyRegister (Assembly assembly)
		{
			assemblies.Add (assembly);
			SetEntryAssembly (assembly);

			pending_assemblies--;
			
			if (pending_assemblies == 0)
				CreateApplication ();				
		}

		// extracted since NativeMethods.surface_emit_error is security critical
		internal void EmitError (int errorCode, string message)
		{
			// FIXME: 8 == EXECUTION_ENGINE_EXCEPTION code.  should it be something else?
			NativeMethods.surface_emit_error (Surface.Native, 8, errorCode, message);
		}

		// extracted since Assembly.GetName is security critical
		void SetEntryAssembly (Assembly asm)
		{
			if (string.Equals (asm.GetName ().Name, EntryPointAssembly, StringComparison.OrdinalIgnoreCase))
				EntryAssembly = asm;
		}

		void TerminateCurrentApplication ()
		{
			if (Application.Current != null) {
				Application.Current.Terminate ();
			}
			if (Deployment.Current != null && Deployment.Current.Surface != null)
				NativeMethods.surface_attach (Deployment.Current.Surface.Native, IntPtr.Zero);
		}

		// will be called when all assemblies are loaded (can be async for downloading)
		// which means we need to report errors to the plugin, since it won't get it from calling managed code
		internal bool CreateApplication ()
		{
			SetCurrentApplication (null);

			if (EntryAssembly == null) {
				EmitError (2103, "Could not find the entry point assembly") ;
				return false;
			}

			Type entry_type = EntryAssembly.GetType (EntryPointType);
			if (entry_type == null) {
				EmitError (2103, String.Format ("Could not find the startup type {0} on the {1}", 
					EntryPointType, EntryPointAssembly));
				return false;
			}

			if (!entry_type.IsSubclassOf (typeof (Application)) && entry_type != typeof (Application)) {
#if SANITY
				Type t = entry_type;
				int spacing = 0;
				while (t != null) {
					if (spacing > 0) {
						for (int i = 0; i < spacing; i ++)
							Console.Write (" ");
						Console.Write ("+ ");
					}
					Console.WriteLine ("{0}", t);
					spacing += 2;
					t = t.BaseType;
				}
#endif

				EmitError (2103, "Startup type does not derive from System.Windows.Application");
				return false;
			}
			
			foreach (Assembly a in Assemblies)
				Application.LoadXmlnsDefinitionMappings (a);

			Application instance = null;

			try {
				instance = (Application) Activator.CreateInstance (entry_type);
			} catch {
				EmitError (2103, String.Format ("Error while creating the instance of type {0}", entry_type));
				return false;
			}

			SetCurrentApplication (instance);

			StartupEventArgs args = new StartupEventArgs();
			instance.OnStartup (args);

			return true;
		}

		internal event EventHandler LayoutUpdated {
			add {
				RegisterEvent (EventIds.Deployment_LayoutUpdatedEvent, value, Events.CreateNullSenderEventHandlerDispatcher (value));
			}
			remove {
				UnregisterEvent (EventIds.Deployment_LayoutUpdatedEvent, value);
			}
		}
	}
}
