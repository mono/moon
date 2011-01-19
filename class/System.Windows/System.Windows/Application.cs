//
// Application.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2008, 2010 Novell, Inc.
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
using Mono.Xaml;
using System;
using System.IO;
using System.IO.IsolatedStorage;
using System.Linq;
using System.Reflection;
using System.Security;
using System.Windows.Controls;
using System.Windows.Resources;
using System.Windows.Interop;
using System.Windows.Media;
using System.Collections;
using System.Collections.Generic;
using System.Globalization;
using System.Resources;
using System.Runtime.InteropServices;
using System.Threading;
using System.Windows.Markup;
using System.ComponentModel;

namespace System.Windows {

	public partial class Application : INativeDependencyObjectWrapper, IRefContainer {
		//
		// Application instance fields
		//
		Mono.EventHandlerList event_list;
		UIElement root_visual;
		SilverlightHost host;

		GetDefaultStyleCallback get_default_style;
		ConvertSetterValuesCallback convert_setter_values;
		ConvertKeyframeValueCallback convert_keyframe_value;
		GetResourceCallback get_resource;
		ApplicationLifetimeObjectsCollection lifetime_objects;
		DependencyObjectHandle handle;

		static Application ()
		{
			ReinitializeStaticData ();
		}

		internal Application (IntPtr raw, bool dropref)
		{
			NativeHandle = raw;
			if (dropref)
				NativeMethods.event_object_unref (raw);

			strongRefs = new Dictionary<IntPtr,object> ();

			NativeDependencyObjectHelper.SetManagedPeerCallbacks (this);

			get_default_style = new GetDefaultStyleCallback (get_default_style_cb_safe);
			convert_setter_values = new ConvertSetterValuesCallback (convert_setter_values_cb_safe);
			convert_keyframe_value = new ConvertKeyframeValueCallback (convert_keyframe_value_cb_safe);
			get_resource = new GetResourceCallback (get_resource_cb_safe);

			NativeMethods.application_register_callbacks (NativeHandle, get_default_style, convert_setter_values, get_resource, convert_keyframe_value);

			if (Current == null) {
				Current = this;

				SynchronizationContext context = new System.Windows.Threading.DispatcherSynchronizationContext ();
				SynchronizationContext.SetSynchronizationContext (context);
			} else {
				root_visual = Current.root_visual;
			}

			lifetime_objects = new ApplicationLifetimeObjectsCollection ();

			// once installed the default quota for isolated storage is augmented to 25MB (instead of 1MB)
			// note: this applies only to the browser (not desktop) assemblies (it won't compile otherwise)
#if !NET_3_0
			if (IsRunningOutOfBrowser) {
				long OutOfBrowserQuota = 25 * IsolatedStorage.DefaultQuota;
				if (IsolatedStorage.Quota < OutOfBrowserQuota)
					IsolatedStorage.Quota = OutOfBrowserQuota;
			}
#endif
			var handler = UIANewApplication;
			if (handler != null)
				handler (this, EventArgs.Empty);
		}

		public Application () : this (SafeNativeMethods.application_new (), true)
		{
		}

		internal void Terminate ()
		{
			if (Deployment.Current.XapDir == null)
				return;

			for (int i = 0; i < ApplicationLifetimeObjects.Count; i++) {
				IApplicationLifetimeAware asvc = ApplicationLifetimeObjects[i] as IApplicationLifetimeAware;
				if (asvc != null)
					asvc.Exiting();
			}
			
			if (Exit != null)
				Exit (this, EventArgs.Empty);

			for (int i = 0; i < ApplicationLifetimeObjects.Count; i++) {
				IApplicationLifetimeAware asvc = ApplicationLifetimeObjects[i] as IApplicationLifetimeAware;
				if (asvc != null)
					asvc.Exited();
			}

			// note: this loop goes in reverse order
			for (int i = ApplicationLifetimeObjects.Count - 1; i >= 0; i--) {
				IApplicationService svc = ApplicationLifetimeObjects[i] as IApplicationService;
				if (svc != null)
					svc.StopService ();
			}
			
			try {
				Directory.Delete (Deployment.Current.XapDir, true);
				Deployment.Current.XapDir = null;
			} catch {
			}

			foreach (KeyValuePair<Assembly, ResourceDictionary> kv in assemblyToGenericXaml)
				kv.Value.Clear ();
			assemblyToGenericXaml.Clear ();
			root_visual = null;
			Application.Current = null;

			ReinitializeStaticData ();
		}

		internal Mono.EventHandlerList EventList {
			get {
				if (event_list == null)
					event_list = new Mono.EventHandlerList (this);
				return event_list;
			}
		}
		
		static void ReinitializeStaticData ()
		{
			// reinitialize these static lists so we don't inherit things from the previous application
			xmlns_definitions = new Dictionary<XmlnsDefinitionAttribute, Assembly> ();

			imported_namespaces = new List<string> ();

			ImportXamlNamespace ("clr-namespace:System.Windows;assembly:System.Windows.dll");
			ImportXamlNamespace ("clr-namespace:System.Windows.Controls;assembly:System.Windows.dll");
		}				

		public bool IsRunningOutOfBrowser {
			get {
				if (!Helper.CheckAccess ())
					throw new UnauthorizedAccessException ("Must be called from the main thread");
				return NativeMethods.application_is_running_out_of_browser (NativeHandle);
			}
		}

		public InstallState InstallState {
			get {
				if (!Helper.CheckAccess ())
					throw new UnauthorizedAccessException ("Must be called from the main thread");
				return NativeMethods.application_get_install_state (NativeHandle);
			}
		}

		public bool Install ()
		{
			return Install (false);
		}

		bool Install (bool unattended)
		{
			// note: user-initiated check is done in unmanaged code
			return NativeMethods.application_install (NativeHandle, false, unattended);
		}

		public IList ApplicationLifetimeObjects {
			get {
				if (!Helper.CheckAccess ())
					throw new UnauthorizedAccessException ("Must be called from the main thread");
				return lifetime_objects;
			}
		}


		Dictionary<Assembly, ResourceDictionary> assemblyToGenericXaml = new Dictionary<Assembly, ResourceDictionary>();

		void convert_keyframe_value_cb_safe (Kind kind, IntPtr property_ptr, IntPtr original, out Value converted)
		{
			try {
				convert_keyframe_value_cb (kind, property_ptr, original, out converted);
			} catch (Exception ex) {
				converted = default (Value);
				try {
					Console.WriteLine ("Moonlight: Unhandled exception in apply_object_key_frame_cb_safe: {0}", ex);
				} catch {
				}
			}
		}
		
		void convert_keyframe_value_cb (Kind kind, IntPtr property_ptr, IntPtr original, out Value converted)
		{
			Type type = Deployment.Current.Types.KindToType (kind);
			if (type != null)
				Types.Ensure (type);
			
			DependencyProperty property = DependencyProperty.Lookup (property_ptr);
			if (property == null) {
				Console.WriteLine ("Moonlight Error: Property couldn't be looked up");
				converted = Value.Empty;
				return;
			}
			
			object o = Value.ToObject (null, original);
			if (o == null) {
				Console.WriteLine ("Moonlight Error: Object was null");
				converted = Value.Empty;
				return;
			}
			
			o = MoonlightTypeConverter.ConvertObject (property, o, null, true);
			
			if (o == null) {
				Console.WriteLine ("Moonlight Error: Converted to null");
				converted = Value.Empty;
				return;
			}

			// This is freed in native code
			converted = Value.FromObject (o);
		}
		
		void get_default_style_cb_safe (IntPtr fwe_ptr, out IntPtr styles_array)
		{
			styles_array = IntPtr.Zero;
			try {
				var fwe = NativeDependencyObjectHelper.FromIntPtr (fwe_ptr) as FrameworkElement;
				if (fwe != null)
					get_default_style_cb (fwe, out styles_array);
			} catch (Exception ex) {
				try {
					Console.WriteLine ("Moonlight: Unhandled exception in Application.get_default_style_cb_safe: {0}", ex);
				} catch {
				}
			}
		}

		void get_default_style_cb (FrameworkElement fwe, out IntPtr styles_array)
		{
			Type fwe_type = fwe.GetType();
			string fwe_type_string = fwe_type.ToString();
			List<IntPtr> implicit_styles = new List<IntPtr> ();

			Style generic_xaml_style = null;
			Style app_resources_style = null;
			Style visual_tree_style = null;

			// start with the lowest priority, the
			// generic.xaml style, only valid for
			// controls, as it requires DefaultStyleKey.
			Control control = fwe as Control;
			if (control != null) {
				Type style_key = control.DefaultStyleKey as Type;
				if (style_key != null) {
					generic_xaml_style = GetGenericXamlStyleFor (style_key);
				}
			}

			// next try the application's resources.
			// these affect all framework elements,
			// whether inside templates or out.
			if (Resources.Contains (fwe_type))
				app_resources_style = (Style)Resources[fwe_type];
			else if (Resources.Contains (fwe_type_string))
				app_resources_style = (Style)Resources[fwe_type_string];

			// highest priority, the visual tree
			FrameworkElement el = fwe;
			while (el != null) {
				// if the frameworkelement was defined outside of a template and we hit a containing
				// template (e.g. if the FWE is in the Content of a ContentControl) we need to skip
				// the intervening elements in the visual tree, until we hit more user elements.
				if (el.TemplateOwner != null && fwe.TemplateOwner == null) {
					el = VisualTreeHelper.GetParent (el) as FrameworkElement;
					continue;
				}

				// for non-controls, we limit the implicit style scope to that of the template.
				if (!(fwe is Control) && el == fwe.TemplateOwner)
					break;

				if (el.Resources.Contains (fwe_type)) {
					visual_tree_style = (Style)el.Resources[fwe_type];
					if (visual_tree_style != null)
						break;
				}
				else if (el.Resources.Contains (fwe_type_string)) {
					visual_tree_style = (Style)el.Resources[fwe_type_string];
					if (visual_tree_style != null)
						break;
				}

				el = VisualTreeHelper.GetParent (el) as FrameworkElement;
			}

			implicit_styles.Insert (0, IntPtr.Zero);
			if (generic_xaml_style != null)
				implicit_styles.Insert (0, generic_xaml_style.native);
			if (visual_tree_style != null)
				implicit_styles.Insert (0, visual_tree_style.native);
			else if (app_resources_style != null)
				implicit_styles.Insert (0, app_resources_style.native);
			
			styles_array = Marshal.AllocHGlobal (IntPtr.Size * implicit_styles.Count);
			for (int i = 0; i < implicit_styles.Count; i ++)
				Marshal.WriteIntPtr (styles_array, i * IntPtr.Size, implicit_styles[i]);
		}
		
		void convert_setter_values_cb_safe (IntPtr style_ptr)
		{
			try {
				convert_setter_values_cb (style_ptr);
			} catch (Exception ex) {
				try {
					Console.WriteLine ("Moonlight: Unhandled exception in Application.convert_setter_values_cb: {0}", ex);
				} catch {
				}
			}
		}
		
		void convert_setter_values_cb (IntPtr style_ptr)
		{
			Style style = NativeDependencyObjectHelper.FromIntPtr(style_ptr) as Style;
			style.ConvertSetterValues ();
		}

		public void CheckAndDownloadUpdateAsync ()
		{
			NativeMethods.application_check_and_download_update_async (NativeHandle);
		}

		internal Style GetGenericXamlStyleFor (Type type)
		{
			ResourceDictionary rd = null;

			if (assemblyToGenericXaml.ContainsKey (type.Assembly)) {
				rd = assemblyToGenericXaml[type.Assembly];
			}
			else {
				Console.WriteLine ("trying to load: /{0};component/themes/generic.xaml",
						   type.Assembly.GetName().Name);

				StreamResourceInfo info = null;

				try {
					info = GetResourceStream (new Uri (string.Format ("/{0};component/themes/generic.xaml",
											  type.Assembly.GetName().Name), UriKind.Relative));
				}
				catch {
					Console.WriteLine ("no generic.xaml for assembly {0}", type.Assembly.GetName().Name);
				}
				
				if (info != null) {
					using (StreamReader sr = new StreamReader (info.Stream)) {
						Uri resource_base = UriHelper.FromNativeUri (NativeMethods.dependency_object_get_resource_base (NativeHandle));
						XamlLoader loader = XamlLoaderFactory.CreateLoader (type.Assembly, resource_base, Deployment.Current.Surface.Native, PluginHost.Handle);

						try {
							rd = loader.CreateObjectFromReader (sr, false) as ResourceDictionary;
						}
						catch (Exception e) {
							Console.WriteLine ("failed generic.xaml parsing:");
							Console.WriteLine (e);
						}
					}
				}

				if (rd == null)
					// create an empty one so we don't fall into this block again for this assembly
					rd = new ResourceDictionary();

				assemblyToGenericXaml[type.Assembly] = rd;

			}

			return rd[type.FullName] as Style;
		}

		public static void LoadComponent (object component, Uri resourceLocator)
		{			
			if (component == null)
				throw new ArgumentNullException ("component");

			if (resourceLocator == null)
				throw new ArgumentNullException ("resourceLocator");

			StreamResourceInfo sr = GetResourceStream (resourceLocator);

			if (sr == null) {
				// throws for case like a resource outside the XAP (e.g. DRT924) i.e. no assembly specified
				if (resourceLocator.ToString ().IndexOf (';') == -1)
					throw new XamlParseException ();
				// otherwise simply return
				return;
			}

			Assembly loading_asm = component.GetType ().Assembly;
	
			XamlLoader loader = XamlLoaderFactory.CreateLoader (loading_asm, resourceLocator, Deployment.Current.Surface.Native, PluginHost.Handle);
			loader.Hydrate (component, sr.Stream);
		}

		private static Dictionary<string,byte[]> local_xap_resources = new Dictionary<string,byte[]> ();

		unsafe static StreamResourceInfo GetXapResource (string resource)
		{
			try {
				string canon = Helper.CanonicalizeResourceName (resource);
				string res_file;

				if (string.IsNullOrEmpty (Deployment.Current.XapDir))
					return null;

				res_file = Path.GetFullPath (Path.Combine (Deployment.Current.XapDir, canon));
				// ensure the file path is rooted against the XAP directory and that it exists
				if (!res_file.StartsWith (Deployment.Current.XapDir) || !File.Exists (res_file))
					return null;

				byte[] data = null;
				// we don't want to run out of file handles (see bug #535709) so we cache the data in memory
				if (!local_xap_resources.TryGetValue (res_file, out data)) {
					data = File.ReadAllBytes (res_file);
					local_xap_resources.Add (res_file, data);
				}
				fixed (byte* ptr = &data [0]) {
					Stream ums = new UnmanagedMemoryStream (ptr, data.Length, data.Length, FileAccess.Read);
					return new StreamResourceInfo (ums, null);
				}
			}
			catch {
				return null;
			}
		}

		/*
		 * Resources take the following format:
		 * 	[/[AssemblyName;component/]]resourcename
		 * They will always be resolved in the following order:
		 * 	1. Application manifest resources
		 * 	2. XAP content
		 */
		public static StreamResourceInfo GetResourceStream (Uri uriResource)
		{
			if (uriResource == null)
				throw new ArgumentNullException ("uriResource");

			if (uriResource.IsAbsoluteUri && uriResource.Scheme != Uri.UriSchemeFile) {
				throw new ArgumentException ("Absolute uriResource");
			}

			// FIXME: URI must point to
			// - the application assembly (embedded resources)
			// - an assembly part of the application package (embedded resources)
			// - something included in the package

			Assembly assembly;
			string assembly_name;
			string resource;
			string loc = uriResource.ToString ();
			int p = loc.IndexOf (';');

			/* We have a resource of the format /assembly;component/resourcename */
			/* It looks like the / is optional tho.  *SIGH* */
			if (p > 0) {
				int l = loc [0] == '/' ? 1 : 0;
				assembly_name = loc.Substring (l, p - l);
				assembly = GetAssembly (assembly_name);
				if (assembly == null)
					return null;

				resource = loc.Substring (p + 11);
			} else {
				assembly = Deployment.Current.EntryAssembly;
				// Deployment.Current.EntryPointAssembly is not usable outside the main thread
				assembly_name = assembly.GetName ().Name;
				resource = loc;
			}

			resource = resource [0] == '/' ? resource : string.Format ("/{0}", resource);
			resource = Path.GetFullPath (resource);
			resource = resource [0] == '/' ? resource.Substring (1) : resource;

			try {
				var manager = new ResourceManager (assembly_name + ".g", assembly) { IgnoreCase = true };
				var stream = manager.GetStream (Uri.EscapeUriString (resource));
				if (stream != null)
					return new StreamResourceInfo (stream, string.Empty);
			} catch {}

			return GetXapResource (resource);
		}

		internal static bool IsAbsoluteResourceStreamLocator (Uri locator)
		{
			if (locator.IsAbsoluteUri)
				return true;

			string str_value = locator.ToString ();

			if (Path.IsPathRooted (str_value))
				return true;

			if (str_value.Contains (";component/"))
				return true;

			return false;
		}

		internal static Uri MergeResourceStreamLocators (Uri resource_base, Uri relative)
		{
			UriKind uri_kind;
			string base_str;
			string relative_str = relative.ToString ();

			if (resource_base.IsAbsoluteUri) {
				base_str = Path.GetDirectoryName (resource_base.AbsolutePath);
				uri_kind = UriKind.Absolute;
			} else {
				base_str = Path.GetDirectoryName (resource_base.ToString ());
				uri_kind = UriKind.Relative;
			}

			string full_path = Path.Combine (base_str, relative_str);
			return new Uri (full_path, uri_kind);
		}

		internal static ManagedStreamCallbacks get_resource_cb_safe (IntPtr resourceBase, IntPtr name)
		{
			try {
				return get_resource_cb (UriHelper.FromNativeUri (resourceBase), UriHelper.FromNativeUri (name));
			} catch (Exception ex) {
				try {
					Console.WriteLine ("Moonlight: Unhandled exception in Application.get_resource_cb: {0}", ex);
				} catch {
				}
			}
			return new ManagedStreamCallbacks ();
		}

		internal static ManagedStreamCallbacks get_resource_cb (Uri resourceBase, Uri name)
		{
			StreamResourceInfo info = null;

			if (resourceBase != null
				&& !resourceBase.IsAbsoluteUri
				&& resourceBase.OriginalString.IndexOf (";component/", StringComparison.OrdinalIgnoreCase) != -1
				&& resourceBase.OriginalString [0] == '/') {
				// DRT: #788
				// resource base is like: /assembly;component/path/to/file.xaml
				// we make an absolute uri of resourceBase by faking a file:// uri,
				// combine with the name, and then strip off the file:// when 
				// passing it to GetResourceStream. The file:// hack is to avoid
				// parsing strings manually.
				// #243 checks that the ";component/" comparison must ignore case.
				try {
					Uri absolute_rb = new Uri ("file://" + resourceBase.OriginalString, UriKind.Absolute);
					Uri absolute_uri = new Uri (absolute_rb, name);
					info = GetResourceStream (new Uri (absolute_uri.AbsolutePath, UriKind.Relative));
				} catch {}
			}

			if (resourceBase != null && info == null) {
				try {
					if (resourceBase.IsAbsoluteUri && resourceBase.Scheme == Uri.UriSchemeFile) {
						Uri file_stream_locator = new Uri (resourceBase, name);
						info = GetResourceStream (file_stream_locator);
					} else if (!resourceBase.IsAbsoluteUri) {
						// DRT #243 has a resource base like this: dir/file.xaml, and requests subdir/file.wmv,
						// which is in dir/subdir/file.wmv. Since we can't combine relative urls, create a
						// fake absolute uri and then take the relative path of it.
						Uri absolute_resbase = new Uri (new Uri ("http://mono-project.com"), resourceBase);
						Uri relative_stream_locator = new Uri (new Uri (absolute_resbase, name).AbsolutePath.Substring (1), UriKind.Relative);
						info = GetResourceStream (relative_stream_locator);
					}
				} catch {}
			}

			if (info == null) {
				Uri resource_uri = name;
				if (!resource_uri.IsAbsoluteUri) {
					// 'name' might be escaped. The following is to unescape it.
					// DRT #GB18030* (GB18030_double1 for instance) run into this.
					Uri absolute = new Uri ("http://www.mono-project.com/", UriKind.Absolute);
					Uri absolute_uri = new Uri (absolute, resource_uri);
					Uri unescaped = new Uri (absolute_uri.GetComponents (UriComponents.Path, UriFormat.SafeUnescaped), UriKind.Relative);
					resource_uri = unescaped;
					
				}
				info = GetResourceStream (resource_uri);
			}


			if (info == null)
				return new ManagedStreamCallbacks ();

			return new StreamWrapper (info.Stream).GetCallbacks ();
		}

		internal static Assembly GetAssembly (string name)
		{
			foreach (var assembly in Deployment.Current.Assemblies)
				if (assembly.GetName ().Name == name)
					return assembly;

			return null;
		}

		public static StreamResourceInfo GetResourceStream (StreamResourceInfo zipPackageStreamResourceInfo, Uri uriResource)
		{
			if (zipPackageStreamResourceInfo == null)
				throw new ArgumentNullException ("zipPackageStreamResourceInfo");
			if (uriResource == null)
				throw new ArgumentNullException ("resourceUri");
			
			MemoryStream ms = new MemoryStream ();
			ManagedStreamCallbacks source_cb;
			ManagedStreamCallbacks dest_cb;
			StreamWrapper source_wrapper;
			StreamWrapper dest_wrapper;
			Stream source;

			source = zipPackageStreamResourceInfo.Stream;

			source_wrapper = new StreamWrapper (source);
			dest_wrapper = new StreamWrapper (ms);

			source_cb = source_wrapper.GetCallbacks ();
			dest_cb = dest_wrapper.GetCallbacks ();

			if (NativeMethods.managed_unzip_stream_to_stream (ref source_cb, ref dest_cb, uriResource.ToString ())) {
				if (source.CanSeek)
					source.Seek (0, SeekOrigin.Begin);
				ms.Seek (0, SeekOrigin.Begin);
				return new StreamResourceInfo (ms, null);
			}

			return null;
		}

		public static Application Current {
			get {
				IntPtr app = NativeMethods.application_get_current ();
				return NativeDependencyObjectHelper.Lookup (app) as Application;
			}

			private set {
				NativeMethods.application_set_current (value == null ? IntPtr.Zero : value.NativeHandle);
			}
		}

		internal static bool IsCurrentSet {
			get {
				IntPtr app = NativeMethods.application_get_current ();
				return NativeDependencyObjectHelper.Lookup (app) != null;
			}
		}
#if NET_3_0
		// desktop assemblies always have (more than) elevated permissions
		public bool HasElevatedPermissions {
			get { return true; }
			set { ; }
		}
#else
		public bool HasElevatedPermissions {
			get { return SecurityManager.HasElevatedPermissions; }
			[EditorBrowsable (EditorBrowsableState.Never)]
			set { SecurityManager.HasElevatedPermissions = value; }
		}
#endif
		public Window MainWindow {
			get {
				if (!IsRunningOutOfBrowser)
					throw new NotSupportedException ("OoB-only feature");

				Window window = new Window ();
				IntPtr moon_window = NativeMethods.surface_get_normal_window (Deployment.Current.Surface.Native);

				NativeMethods.window_set_moon_window (window.native, moon_window);

				Console.WriteLine ("returning window {0}", window.GetHashCode());

				return window;
			}
		}
		
		public ResourceDictionary Resources {
			get {
				if (!Helper.CheckAccess ())
					throw new UnauthorizedAccessException ("Must be called from the main thread");
				return (ResourceDictionary) ((INativeDependencyObjectWrapper)this).GetValue (ResourcesProperty);
			}
			set {
				if (!Helper.CheckAccess ())
					throw new UnauthorizedAccessException ("Must be called from the main thread");
				((INativeDependencyObjectWrapper) this).SetValue (ResourcesProperty, value);
			}
		}

		public UIElement RootVisual {
			get {
				if (!Helper.CheckAccess ())
					throw new UnauthorizedAccessException ("Must be called from the main thread");
				return root_visual;
			}

			set {
				if (!Helper.CheckAccess ())
					throw new UnauthorizedAccessException ("Must be called from the main thread");
				if (value == null)
					throw new InvalidOperationException ();

				// Can only be set once according to the docs.
				if (root_visual != null)
					return;
				
				root_visual = value;

				NativeMethods.surface_attach (Deployment.Current.Surface.Native, root_visual.native);

				var handler = UIARootVisualSet;
				if (handler != null)
					handler (this, EventArgs.Empty);
			}
		}

		public SilverlightHost Host {
			get {
				if (!Helper.CheckAccess ())
					throw new UnauthorizedAccessException ("Must be called from the main thread");
				return host ?? (host = new SilverlightHost ());
			}
		}

		//used by A11Y infrastructure
		internal event EventHandler UIARootVisualSet;
		internal static event EventHandler UIANewApplication;

		public event EventHandler Exit;
		public event StartupEventHandler Startup;
		public event EventHandler<ApplicationUnhandledExceptionEventArgs> UnhandledException;
		
		public event CheckAndDownloadUpdateCompletedEventHandler CheckAndDownloadUpdateCompleted {
			add {
				EventList.RegisterEvent (this, EventIds.Application_CheckAndDownloadUpdateCompletedEvent, value,
					       Events.CreateCheckAndDownloadUpdateCompletedEventHandlerDispatcher (this, value));
			}
			remove {
				EventList.UnregisterEvent (this, EventIds.Application_CheckAndDownloadUpdateCompletedEvent, value);
			}
		}
		
		public event EventHandler InstallStateChanged {
			add {
				EventList.RegisterEvent (this, EventIds.Application_InstallStateChangedEvent, value,
					       Events.CreateEventHandlerDispatcher (value));
			}
			remove {
				EventList.UnregisterEvent (this, EventIds.Application_InstallStateChangedEvent, value);
			}
		}
		
		internal void OnStartup (StartupEventArgs e) {
			// FIXME: should we be sharing the
			// Dictionary<string,string> for each call to
			// the lifetime objects?  or should we be
			// creating a new one for each call?
			ApplicationServiceContext ctx = new ApplicationServiceContext (e.InitParams as Dictionary<string,string>);

			lifetime_objects.Close (); // it's now too late to add items to this collection

			for (int i = 0; i < ApplicationLifetimeObjects.Count; i++) {
				IApplicationService svc = ApplicationLifetimeObjects[i] as IApplicationService;
				if (svc != null)
					svc.StartService (ctx);

			}

			for (int i = 0; i < ApplicationLifetimeObjects.Count; i++) {
				IApplicationLifetimeAware asvc = ApplicationLifetimeObjects[i] as IApplicationLifetimeAware;
				if (asvc != null)
					asvc.Starting();
			}

			if (Startup != null)
				Startup (this, e);

			for (int i = 0; i < ApplicationLifetimeObjects.Count; i++) {
				IApplicationLifetimeAware asvc = ApplicationLifetimeObjects[i] as IApplicationLifetimeAware;
				if (asvc != null)
					asvc.Started();
			}
		}

		// initialized in ReinitializeStaticData
		internal static Dictionary<XmlnsDefinitionAttribute,Assembly> xmlns_definitions;
		internal static List<string> imported_namespaces;
		
		internal static void LoadXmlnsDefinitionMappings (Assembly a)
		{
			object [] xmlns_defs = a.GetCustomAttributes (typeof (XmlnsDefinitionAttribute), false);

			foreach (XmlnsDefinitionAttribute ns_mapping in xmlns_defs){
				xmlns_definitions [ns_mapping] = a;
			}
		}

		internal static void ImportXamlNamespace (string xmlns)
		{
			imported_namespaces.Add (xmlns);
		}

		static Type GetType (Assembly assembly, string ns, string name)
		{
			var fullname = string.IsNullOrEmpty (ns) ? name : ns + "." + name;
			return assembly.GetType (fullname);
		}

		internal static Type GetComponentTypeFromName (string name)
		{
			// If it has a namespace its definitely a fullname
			if (name.Contains ('.'))
				return GetComponentTypeFromFullName (name);

			foreach (var pair in xmlns_definitions) {
				if (!imported_namespaces.Contains (pair.Key.XmlNamespace))
					continue;

				var type = GetType (pair.Value, pair.Key.ClrNamespace, name);
				if (type != null)
					return type;
			}

			// Maybe its a fullname without a namespace
			if (!name.Contains ('.'))
				return GetComponentTypeFromFullName (name);

			return null;
		}

		internal static Type GetComponentTypeFromFullName (string name)
		{
			foreach (Assembly asm in Deployment.Current.Assemblies) {
				Type t = asm.GetType (name);
				if (t != null)
					return t;
			}

			return null;
		}

		//
		// Creates the proper component by looking the namespace and name
		// in the various assemblies loaded
		//
		internal static object CreateComponentFromName (string name)
		{
			Type t = GetComponentTypeFromName (name);

			if (t == null) {
				Console.Error.WriteLine ("Application.CreateComponentFromName - could not find type: {0}", name);
				return null;
			}

			return Activator.CreateInstance (t);
		}

		internal static void OnUnhandledException (object sender, Exception ex)
		{
			try {
				Application app = Application.Current;
				string unmanaged_report = ex.ToString();

				if (app != null && app.UnhandledException != null) {
					ApplicationUnhandledExceptionEventArgs args = new ApplicationUnhandledExceptionEventArgs (ex, false);
					try {
						app.UnhandledException (sender, args);
						if (args.Handled)
							unmanaged_report = null;
					} catch (Exception ex2) {
						Console.WriteLine ("Exception caught in Application UnhandledException handler: " + ex2);
						unmanaged_report = ex2.Message;
					}
				} else {
					Console.WriteLine ("Unhandled Exception: " + ex);
				}

				if (unmanaged_report != null) {
					Deployment.Current.EmitError (4004, unmanaged_report);
				}
			} catch {
				// Make this completely safe.
			}
		}

		private static readonly DependencyProperty ResourcesProperty =
			DependencyProperty.Lookup (Kind.APPLICATION, "Resources", typeof (ResourceDictionary));

		internal IntPtr NativeHandle {
			get { return handle.Handle; }
			set {
				if (handle != null) {
					throw new InvalidOperationException ("Application.native is already set");
				}

				NativeDependencyObjectHelper.AddNativeMapping (value, this);
				handle = new DependencyObjectHandle (value, this);
			}
		}

		IntPtr INativeEventObjectWrapper.NativeHandle {
			get { return NativeHandle; }
			set { NativeHandle = value; }
		}

		Dictionary<IntPtr,object> strongRefs;

		void IRefContainer.AddStrongRef (IntPtr referent, string name)
		{
			if (strongRefs.ContainsKey (referent))
				return;

			var o = Value.ToObject (referent);
			if (o != null) {
#if DEBUG_REF
				Console.WriteLine ("Adding ref from {0}/{1} to {2}/{3}", GetHashCode(), this, o.GetHashCode(), o);
#endif
				strongRefs.Add (referent, o);
			}
		}

		void IRefContainer.ClearStrongRef (IntPtr referent, string name)
		{
#if DEBUG_REF
			var o = Value.ToObject (referent);
			Console.WriteLine ("Clearing ref from {0}/{1} to {2}/{3}", GetHashCode(), this, o.GetHashCode(), o);
			Console.WriteLine (Environment.StackTrace);
#endif
			strongRefs.Remove (referent);
		}

#if HEAPVIZ
		ICollection IRefContainer.GetManagedRefs ()
		{
			List<HeapRef> refs = new List<HeapRef> ();
			foreach (IntPtr nativeref in strongRefs.Keys)
				if (strongRefs[nativeref] is INativeEventObjectWrapper)
					refs.Add (new HeapRef ((INativeEventObjectWrapper)strongRefs[nativeref]));
				
			return refs;
		}
#endif

		void INativeEventObjectWrapper.MentorChanged (IntPtr mentor_ptr)
		{
		}

		void INativeEventObjectWrapper.OnAttached ()
		{
		}

		void INativeEventObjectWrapper.OnDetached ()
		{
		}

		Kind INativeEventObjectWrapper.GetKind ()
		{
			return Deployment.Current.Types.TypeToKind (GetType ());
		}
	}
}
