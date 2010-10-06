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
using System.Reflection;
using System.Security;
using System.Threading;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Media;
using System.Windows.Interop;
using Mono;
using Mono.Xaml;

namespace System.Windows {

	public sealed partial class Deployment : DependencyObject {
		GlyphTypefaceCollection typefaces;
		Assembly entry_point_assembly;
		List<Assembly> assemblies;
		int pending_assemblies;
		string xap_dir;
		Types types;
		Stack<Uri> parse_uri_stack;

		static List<Action> shutdown_actions = new List<Action> ();
		static bool is_shutting_down;

		internal int MajorVersion {
			get { return int.Parse (RuntimeVersion.Split ('.') [0]); }
		}

		private new void Initialize ()
		{
			UriHelper.Initialize (this);

			parse_uri_stack = new Stack<Uri>();
		}

		~Deployment ()
		{
			if (!NativeMethods.deployment_is_safe_to_die (native)) {
				should_free_in_finalizer = false;
				GC.ReRegisterForFinalize (this);
			}
			else
				should_free_in_finalizer = true;
		}

		Application current_app;

		internal override void AddStrongRef (IntPtr referent, string name)
		{
			if (name == "CurrentApplication")
				current_app = NativeDependencyObjectHelper.FromIntPtr (referent) as Application;
			else
				base.AddStrongRef (referent, name);
		}

		internal override void ClearStrongRef (IntPtr referent, string name)
		{
			if (name == "CurrentApplication")
				current_app = null;
			else
				base.ClearStrongRef (referent, name);
		}

#if HEAPVIZ
		public void GraphManagedHeap (string name)
		{
			HeapUtil.GraphManagedHeap (name);
		}
#endif
		
		internal Surface Surface {
			get {
				// FIXME: find a way to move this to the new managed-ref stuff

				/* we need to use the thread-safe version of Deployment::GetSurface since this property
				 * may get called from several threads. */
				IntPtr surface = NativeMethods.deployment_get_surface_reffed (this.native);
				Surface result;
                               
				if (surface == IntPtr.Zero)
					return null;
                               
				result = NativeDependencyObjectHelper.FromIntPtr (surface) as Surface;
                               
				/* we got a reffed surface, release that ref now that we'll have a managed ref */
				NativeMethods.event_object_unref (surface);
                               
				return result;
			}
		}
		
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
			Current.AddAssembly (assembly);
		}

		void AddAssembly (Assembly asm)
		{
			if (!assemblies.Contains (asm))
				assemblies.Add (asm);
		}
		
		public static void SetCurrentApplication (Application application)
		{
			if ((application == null && Application.IsCurrentSet) ||
				(application != null && Application.IsCurrentSet && Application.Current != application))
				Application.Current.Free ();

			NativeMethods.deployment_set_current_application (Current.native, application == null ? IntPtr.Zero : application.NativeHandle);
		}

		internal GlyphTypefaceCollection SystemTypefaces {
			get {
				if (typefaces == null) {
					IntPtr retval = NativeMethods.deployment_get_system_typefaces (native);
					typefaces = new GlyphTypefaceCollection (retval, false);
				}
				
				return typefaces;
			}
		}

		internal Types Types {
			get {
				if (types == null)
					types = new Types (NativeMethods.deployment_get_types (native));
				return types;
			}
		}

		internal Stack<Uri> ParseUriStack {
			get {
				return parse_uri_stack;
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
					if (versions[2] <= 30524)
						throw new MoonException (2106, "Failed to load the application. It was built with an obsolete version of Silverlight");
			}
			else if (versions[0] == 3) {
				// we need to add validation stuff for SL3 rtm here
			}
			else if (versions[0] == 4) {
				// we don't actually validate any 4.0
				// runtime versions yet since there's
				// no telling what it'll be at RTM.
			}
			else {
				throw new MoonException (2105, "invalid RuntimeVersion");
			}
		}

		void ReadManifest ()
		{
			XamlLoader loader = XamlLoaderFactory.CreateLoader (typeof (DependencyObject).Assembly, null, Surface.Native, PluginHost.Handle);
			string app_manifest = Path.Combine (XapDir, "appmanifest.xaml");

			if (!File.Exists (app_manifest))
				throw new MoonException(2103, "Invalid or malformed application: Check manifest");

			Stream app_manifest_stream = null;
			try {
				app_manifest_stream = File.OpenRead (app_manifest);
				loader.Hydrate (this, app_manifest_stream);
			} catch (Exception e) {
				throw new MoonException (7016, e.Message);
			} finally {
				if (app_manifest_stream != null)
					app_manifest_stream.Close ();
			}

			if (RuntimeVersion == null)
				throw new MoonException (2105, "No RuntimeVersion specified in AppManifest");

			CompareRuntimeVersions ();

			if (EntryPointAssembly == null)
				throw new MoonException (2103, "Invalid or malformed application: Check manifest");

			if (EntryPointType == null)
				throw new Exception ("No entrypoint defined in the AppManifest.xaml");
		}

		void TerminateAndSetCulture (string culture, string uiCulture)
		{
			TerminateCurrentApplication ();

			try {
				if (culture != null && culture.ToLower () != "auto")
					Thread.CurrentThread.CurrentCulture = new CultureInfo (culture);
				if (uiCulture != null && uiCulture.ToLower() != "auto")
					Thread.CurrentThread.CurrentUICulture = new CultureInfo (uiCulture);
			}
			catch (Exception e) {
				// 2105 is required by the Localization drt (#352)
				throw new MoonException (2105, e.Message);
			}
		}

		internal bool InitializeDeployment (string culture, string uiCulture)
		{
			TerminateAndSetCulture (culture, uiCulture);

			NativeMethods.deployment_set_initialization (native, true);
			try {
				EntryPointType = "System.Windows.Application";
				EntryPointAssembly = typeof (Application).Assembly.GetName ().Name;
				EntryAssembly = typeof (Application).Assembly;
				return LoadAssemblies ();
			}
			finally {
				NativeMethods.deployment_set_initialization (native, false);
			}
		}
			
		internal bool InitializeDeployment (IntPtr plugin, string xapPath, string culture, string uiCulture)
		{
			TerminateAndSetCulture (culture, uiCulture);

			if (plugin == IntPtr.Zero) {
				Uri source_uri = UriHelper.FromNativeUri (NativeMethods.surface_get_source_location (Surface.Native));

				if (source_uri != null) {
					// full uri including xap

					// IsolatedStorage (inside mscorlib.dll) needs some information about the XAP file
					// to initialize it's application and site directory storage. WebClient is another user of this
					AppDomain.CurrentDomain.SetData ("xap_uri", PluginHost.GetApplicationIdentity (source_uri));
				}
			}
			else {
				PluginHost.SetPluginHandle (plugin);
			}
			
			if (!Directory.Exists (xapPath))
				ExtractXap (xapPath);
			else
				XapDir = xapPath;
			
			// this is currently disabled for the 3.0 desktop profile.  we'll
			// need it to be done by unmanaged code there, on every deployment
			// switch.
#if NET_2_1
			AppDomain.CurrentDomain.SetupInformationNoCopy.ApplicationBase = XapDir;
#endif

			NativeMethods.deployment_set_ensure_managed_peer_callback (native, ensure_managed_peer);
			NativeMethods.deployment_set_initialization (native, true);
			try {
				ReadManifest ();

				NativeMethods.deployment_set_is_loaded_from_xap (native, true);
				return LoadAssemblies ();
			}
			catch (Exception exc) {
				Console.WriteLine (exc);
				throw;
			}
			finally {
				NativeMethods.deployment_set_initialization (native, false);
			}
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

						DownloadAssembly (ext.Source, 2152);

						pending_assemblies++;
					} catch (Exception e) {
						int error = (e is MethodAccessException) ? 4004 : 2152;
						throw new MoonException (error, string.Format ("Error while loading the '{0}' ExternalPart: {1}", ext.Source, e.Message));
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
						// If the exact name does not exist, try to find a lowercased version. #7007.OOB hits this,
						// it has a case-mismatched dll in the xap.
						if (!File.Exists (filename)) {
							string lowercased = Path.Combine (Path.GetDirectoryName (filename), Helper.CanonicalizeResourceName (Path.GetFileName (filename)));
							if (File.Exists (lowercased))
								filename = lowercased;
						}

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

					DownloadAssembly (new Uri (source, UriKind.RelativeOrAbsolute), 2105);
				} catch (Exception e) {
					int error = (e is MethodAccessException) ? 4004 : 2105;
					throw new MoonException (error, string.Format ("Error while loading the '{0}' assembly : {1}", source, e.Message));
				}
			}

			// unmanaged (JS/XAML only) applications can go directly to create the application
			return pending_assemblies == 0 ? CreateApplication () : true;
		}

		private static bool IsZip (byte [] buf)
		{
			if (buf == null || buf.Length < 4)
				return false;

			if (buf [0] != 0x50 || buf [1] != 0x4B || buf [2] != 0x03 || buf [3] != 0x04)
				return false;

			return true;
		}

		void DownloadAssembly (Uri uri, int errorCode)
		{
			Uri xap = new Uri (NativeMethods.plugin_instance_get_source_location (PluginHost.Handle));
			// WebClient deals with relative URI but HttpWebRequest does not
			// but we need the later to detect redirection
			if (!uri.IsAbsoluteUri) {
				uri = new Uri (xap, uri);
			} else if (xap.Scheme != uri.Scheme) {
				throw new SecurityException ("Cross scheme URI downloading " + uri.ToString ());
			}
			HttpWebRequest req = (HttpWebRequest) WebRequest.Create (uri);
			req.BeginGetResponse (AssemblyGetResponse, new object[] { req, errorCode });
		}

		// note: throwing MoonException from here is NOT ok since this code is called async
		// and the exception won't be reported, directly, to the caller
		void AssemblyGetResponse (IAsyncResult result)
		{
			Assembly asm;
			object[] tuple = (object []) result.AsyncState;
			WebRequest wreq = (WebRequest) tuple [0];
			int error_code = (int) tuple [1];
			try {
				HttpWebResponse wresp = (HttpWebResponse) wreq.EndGetResponse (result);

				if (wresp.StatusCode != HttpStatusCode.OK) {
					wresp.Close ();
					EmitError (error_code, String.Format ("Error while downloading the '{0}'.", wreq.RequestUri));
					return;
				}

				if (wresp.ResponseUri != wreq.RequestUri) {
					wresp.Close ();
					EmitError (error_code, "Redirection not allowed to download assemblies.");
					return;
				}

				using (Stream responseStream = wresp.GetResponseStream ()) {
					AssemblyPart a = new AssemblyPart ();
					byte [] buffer = a.StreamToBuffer (responseStream);

					if (IsZip (buffer)) {
						// unzip it.
						using (MemoryStream dest = new MemoryStream ()) {
							using (MemoryStream source = new MemoryStream (buffer)) {
								ManagedStreamCallbacks source_cb;
								ManagedStreamCallbacks dest_cb;
								StreamWrapper source_wrapper;
								StreamWrapper dest_wrapper;

								source_wrapper = new StreamWrapper (source);
								dest_wrapper = new StreamWrapper (dest);

								source_cb = source_wrapper.GetCallbacks ();
								dest_cb = dest_wrapper.GetCallbacks ();

								// the zip files I've come across have a single file in them, the
								// dll.  so we assume that any/every zip file will contain a single
								// file, and just get the first one from the zip file directory.
								if (NativeMethods.managed_unzip_stream_to_stream_first_file (ref source_cb, ref dest_cb))
									buffer = dest.ToArray ();
							}
						}
					}

					asm = a.Load (buffer);
				}

				if (asm != null)
					Dispatcher.BeginInvoke (new AssemblyRegistration (AssemblyRegister), asm);
				else
					EmitError (2153, String.Format ("Error while loading '{0}'.", wreq.RequestUri));
			}
			catch (Exception e) {
				// we need to report everything since any error means CreateApplication won't be called
				EmitError (error_code, e.ToString ());
			}
		}

		internal delegate void AssemblyRegistration (Assembly asm);

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
			if (Thread.CurrentThread == DependencyObject.moonlight_thread) {
				NativeMethods.surface_emit_error (Surface.Native, 8, errorCode, message);
			} else {
				Dispatcher.BeginInvoke (() => {
					// call again but, this time, from main thread
					EmitError (errorCode, message);
				});
			}
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

		[System.ComponentModel.EditorBrowsable (System.ComponentModel.EditorBrowsableState.Never)]
		public static string GetAppIdForUri (Uri AppUri, out string xapLocationStr)
		{
			Console.WriteLine ("System.Windows.Deployment.GetAppIdForUri ({0}): Not implemented (returning empty string)", AppUri);
			/* Returning "" for now to try to avoid any NREs */
			xapLocationStr = string.Empty;
			return string.Empty;
		}

		private static EnsureManagedPeerCallback ensure_managed_peer = new EnsureManagedPeerCallback(EnsureManagedPeer);
		/// <summary>
		///   Ensures that a managed peer has been created for a given native dependencyobject.
		/// </summary>
		private static void EnsureManagedPeer (IntPtr forDO)
		{
			try {
				var o = NativeDependencyObjectHelper.Lookup (forDO);
				if (o == null) {
					o = NativeDependencyObjectHelper.FromIntPtr (forDO);
#if DEBUG_REF
					Console.WriteLine ("Creating managed peer {0}/{1} for {2:X}", o.GetHashCode(), o.GetType(), forDO);
#endif
					// this next line is just to keep mcs from giving us a warning about "o" being unused
					GC.KeepAlive (o);
				}
			} catch (Exception ex) {
				try {
					Console.WriteLine ("Moonlight: Unhandled exception in Deployment.EnsureManagedPeer: {0}", ex);
				} catch {
				}
			}
		}
		
	}
}
