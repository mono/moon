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
using System.Security;
using System.Threading;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Media;
using System.Windows.Interop;
using System.Windows.Threading;
using Mono;
using Mono.Xaml;

namespace System.Windows {

	enum ManifestAssemblyKind {
		SourceAssembly,
		ExternalAssembly
	}

	public sealed partial class Deployment : DependencyObject {
		GlyphTypefaceCollection typefaces;
		Assembly entry_point_assembly;
		List<Assembly> assemblies;
		int pending_downloads;
		string xap_dir;
		Types types;
		Stack<Uri> parse_uri_stack;

		static bool is_shutting_down;

		internal int MajorVersion {
			get { return int.Parse (RuntimeVersion.Split ('.') [0]); }
		}

		private new void Initialize ()
		{
			NativeMethods.deployment_initialize_app_domain (this.native, GetType ().Assembly.FullName);
			UriHelper.Initialize (this);

			parse_uri_stack = new Stack<Uri>();
		}

		Application current_app;

		internal override void AddStrongRef (IntPtr referent, string name)
		{
			if (name == "CurrentApplication")
				current_app = Value.ToObject (referent) as Application;
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
			is_shutting_down = true;
			return true;
		}
		
		public static Deployment Current {
			get {
				IntPtr dep = NativeMethods.deployment_get_current ();
				if (dep == IntPtr.Zero)
					throw null;
				return (Deployment) (NativeDependencyObjectHelper.Lookup (dep) ?? NativeDependencyObjectHelper.CreateObject (Kind.DEPLOYMENT, dep));
			}

			internal set {
				NativeMethods.deployment_set_current (value == null ? IntPtr.Zero : value.native);
			}
		}
	
		public static void RegisterAssembly (Assembly assembly)
		{
			Current.AssemblyRegister (assembly, ManifestAssemblyKind.SourceAssembly);
		}
		
		public static void SetCurrentApplication (Application application)
		{
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

		internal IEnumerable<Assembly> Assemblies {
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
#if NET_2_1
				// note: not enabled for NET_3_0 (desktop) since we're linking against FX2.0 (not 4.0) assemblies
				// see DRT 924 and link below
				// http://msdn.microsoft.com/en-us/library/system.appdomain.iscompatibilityswitchset%28VS.95%29.aspx
				AppDomain.CurrentDomain.SetCompatibilitySwitch ("APP_EARLIER_THAN_SL4.0");
#endif
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
			// called on failure - this can occur when TPE (transparent platform extensions) being loaded
			// do not match the version against which the XAP has been linked (i.e. it was updated independently)
			AppDomain.CurrentDomain.AssemblyResolve += new ResolveEventHandler (ResolveMissingAssemblies);

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

		// calling Assembly.GetName means we're SSC so we avoid using a anonymous delegate (uncontrolled name)
		Assembly ResolveMissingAssemblies (object sender, ResolveEventArgs args)
		{
			AssemblyName failed = new AssemblyName (args.Name);
			foreach (Assembly assembly in assemblies) {
				if (Compare (failed, assembly.GetName ()))
					return assembly;
			}
			return null;
		}

		static bool Compare (AssemblyName a, AssemblyName b)
		{
			if (a.Name != b.Name)
				return false;

			// we could have something more recent (see DRT1001 where a TPE is updated)
			if (a.Version > b.Version)
				return false;

			if (a.CultureInfo.LCID != b.CultureInfo.LCID)
				return false;

			byte[] apkt = a.GetPublicKeyToken ();
			byte[] bpkt = b.GetPublicKeyToken ();
			if (apkt == null) {
				return (bpkt == null);
			} else {
				if ((bpkt == null) || (apkt.Length != bpkt.Length))
					return false;

				for (int i=0; i < apkt.Length; i++) {
					if (apkt [i] != bpkt [i])
						return false;
				}
			}
			return true;
		}

		// note: throwing MoonException from here is ok since this code is called (sync) from the plugin
		internal bool LoadAssemblies ()
		{
			assemblies = new List <Assembly> ();
			assemblies.Add (typeof (Application).Assembly);
			
			pending_downloads = 0;

			for (int i = 0; i < ExternalParts.Count; i++) {
				ExtensionPart ext = ExternalParts[i] as ExtensionPart;

				if (ext != null) {
					try {
						// note: cache is handled by the browser (and indirectly by the server)
						DownloadAssembly (ext.Source, ManifestAssemblyKind.ExternalAssembly);
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
							AssemblyRegister (asm, ManifestAssemblyKind.SourceAssembly);
						} catch (FileNotFoundException) {
							try_downloading = true;
						}
					} else {
						// we can hit a Part with a relative URI starting with a '/' which fails the above test
						try_downloading = true;
					}

					if (!try_downloading)
						continue;

					DownloadAssembly (new Uri (source, UriKind.RelativeOrAbsolute), ManifestAssemblyKind.SourceAssembly);
				} catch (Exception e) {
					int error = (e is MethodAccessException) ? 4004 : 2105;
					throw new MoonException (error, string.Format ("Error while loading the '{0}' assembly : {1}", source, e.Message));
				}
			}

			// unmanaged (JS/XAML only) applications can go directly to create the application
			return pending_downloads == 0 ? CreateApplication () : true;
		}

		private static bool IsZip (byte [] buf)
		{
			if (buf == null || buf.Length < 4)
				return false;

			if (buf [0] != 0x50 || buf [1] != 0x4B || buf [2] != 0x03 || buf [3] != 0x04)
				return false;

			return true;
		}

		void DownloadAssembly (Uri uri, ManifestAssemblyKind kind)
		{
			Uri xap = new Uri (NativeMethods.plugin_instance_get_source_location (PluginHost.Handle));
			// WebClient deals with relative URI but HttpWebRequest does not
			// but we need the later to detect redirection
			if (!uri.IsAbsoluteUri) {
				uri = new Uri (xap, uri);
			} else if (xap.Scheme != uri.Scheme) {
				throw new SecurityException ("Cross scheme URI downloading " + uri.ToString ());
			}
#if NET_3_0
			HttpWebRequest req = (HttpWebRequest) WebRequest.Create (uri);
#else
			HttpWebRequest req = (HttpWebRequest) WebRequestCreator.BrowserHttp.Create (uri);
#endif
			req.BeginGetResponse (AssemblyGetResponse, new object[] { req, kind });
			pending_downloads ++;
		}

		// note: throwing MoonException from here is NOT ok since this code is called async
		// and the exception won't be reported, directly, to the caller
		void AssemblyGetResponse (IAsyncResult result)
		{
			Assembly asm;
			object[] tuple = (object []) result.AsyncState;
			WebRequest wreq = (WebRequest) tuple [0];
			ManifestAssemblyKind kind = (ManifestAssemblyKind) tuple [1];
			int error_code = (kind == ManifestAssemblyKind.ExternalAssembly) ? 2152 : 2105;
			try {
				HttpWebResponse wresp = (HttpWebResponse) wreq.EndGetResponse (result);

				if (wresp.StatusCode != HttpStatusCode.OK) {
					wresp.Close ();
					EmitError (error_code, String.Format ("Error while downloading the '{0}'.", wreq.RequestUri));
					return;
				}

				if ((kind != ManifestAssemblyKind.ExternalAssembly) && (wresp.ResponseUri != wreq.RequestUri)) {
					wresp.Close ();
					EmitError (error_code, "Redirection not allowed to download assemblies.");
					return;
				}

				using (Stream responseStream = wresp.GetResponseStream ()) {
					byte [] buffer = AssemblyPart.StreamToBuffer (responseStream);

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

								// Zip files may contain multiple assemblies, all of which need to be loaded. Keep
								// attempting to open subsequent files until it fails.
								for (int i = 0; ; i ++) {
									if (!NativeMethods.managed_unzip_stream_to_stream_nth_file (ref source_cb, ref dest_cb, i))
										break;
									if (Load (dest.ToArray (), kind) == null)
										EmitError (2153, String.Format ("Error while loading '{0}'.", wreq.RequestUri));
									source.Position = 0;
									dest.SetLength (0);
								}
							}
						}
					} else {
						if (Load (buffer, kind) == null)
							EmitError (2153, String.Format ("Error while loading '{0}'.", wreq.RequestUri));
					}
				}
				Dispatcher.BeginInvoke (AsyncDownloadComplete);
			}
			catch (Exception e) {
				// we need to report everything since any error means CreateApplication won't be called
				EmitError (error_code, e.ToString ());
			}
		}

		// Called on the main thread
		void AsyncDownloadComplete ()
		{
			pending_downloads --;
			if (pending_downloads == 0) {
				try {
					CreateApplication ();
				} catch (Exception e) {
					int error = (e is MethodAccessException) ? 4004 : 2153;
					EmitError (error, string.Format ("Error while creating the Application: {0}", e.Message));
				}
			}
		}

		internal Assembly Load (byte [] buffer, ManifestAssemblyKind kind)
		{
			try {
				Assembly result = Assembly.Load (buffer);
				Dispatcher.Invoke (new AssemblyRegistration (AssemblyRegister), new object[] { result, kind });
				return result;
			}
			catch {
				return null;
			}
		}

		internal delegate void AssemblyRegistration (Assembly asm, ManifestAssemblyKind kind);

		// note: only access 'assemblies' from the main thread
		void AssemblyRegister (Assembly assembly, ManifestAssemblyKind kind)
		{
			if (!assemblies.Contains (assembly)) {
				assemblies.Add (assembly);
				if (kind == ManifestAssemblyKind.SourceAssembly) {
					if (string.Equals (assembly.GetName ().Name, EntryPointAssembly, StringComparison.OrdinalIgnoreCase))
						EntryAssembly = assembly;
				}
			}
		}

		// extracted since NativeMethods.surface_emit_error is security critical
		internal void EmitError (int errorCode, string message)
		{
			// FIXME: 8 == EXECUTION_ENGINE_EXCEPTION code.  should it be something else?
			if (Thread.CurrentThread == DependencyObject.moonlight_thread) {
				NativeMethods.surface_emit_error (Surface.Native, IntPtr.Zero, 8, errorCode, message);
			} else {
				Dispatcher.BeginInvoke (() => {
					// call again but, this time, from main thread
					EmitError (errorCode, message);
				});
			}
		}

		void TerminateCurrentApplication ()
		{
			if (Application.Current != null) {
				Application.Current.Terminate ();
			}
			if (Deployment.Current != null && Deployment.Current.Surface != null)
				NativeMethods.surface_attach (Deployment.Current.Surface.Native, IntPtr.Zero);
		}

		bool IsElevatedPermissionsRequired {
			get {
				OutOfBrowserSettings oobs = OutOfBrowserSettings;
				if (oobs == null)
					return false;
				SecuritySettings ss = oobs.SecuritySettings;
				if (ss == null)
					return false;
				return ss.ElevatedPermissions == ElevatedPermissions.Required;
			}
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
				Console.WriteLine ("Application type: {0}", typeof (Application).AssemblyQualifiedName);
				while (t != null) {
					if (spacing > 0) {
						for (int i = 0; i < spacing; i ++)
							Console.Write (" ");
						Console.Write ("+ ");
					}
					Console.WriteLine ("{0} from {1}", t.AssemblyQualifiedName, t.Assembly.Location);
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
				// set HasElevatedPermissions based on IsRunningOutOfBrowser and ElevatedPermissions.Required
				if (IsElevatedPermissionsRequired) {
					instance.HasElevatedPermissions = instance.IsRunningOutOfBrowser;
				}
			} catch {
				EmitError (2103, String.Format ("Error while creating the instance of type {0}", entry_type));
				return false;
			}

			if (instance.IsRunningOutOfBrowser) {
				WindowSettings ws = OutOfBrowserSettings.WindowSettings;
				if (ws != null) {
					Window w = instance.MainWindow;
					w.Width = ws.Width;
					w.Height = ws.Height;

					switch (ws.WindowStartupLocation) {
					case WindowStartupLocation.Manual:
						w.Left = ws.Left;
						w.Top = ws.Top;
						break;
					case WindowStartupLocation.CenterScreen:
						// FIXME PAL : export screen width and height
						w.Left = ((1280 - w.Width) / 2);
						w.Top = ((800 - w.Height) / 2);
						break;
					default:
						// let the OS decide where to show the window
						break;
					}
				}
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

		private static EnsureManagedPeerCallback ensure_managed_peer = new EnsureManagedPeerCallback(ApplicationLauncher.EnsureManagedPeer);
	}
}
