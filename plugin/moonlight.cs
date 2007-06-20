//
// NativeMethods.cs
//
// Author:
//   Miguel de Icaza (miguel@ximian.com)
//   Jackson Harper  (jackson@ximian.com)
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

using System;
using System.Runtime.InteropServices;
using System.Reflection;
using System.Windows;
using System.Windows.Media;
using System.Windows.Controls;
using System.Collections;
using System.ComponentModel;

namespace Moonlight {

	internal delegate IntPtr CreateElementCallback (string xmlns, string name);
	internal delegate void SetAttributeCallback (IntPtr target, string name, string value);
	internal delegate void HookupEventCallback (IntPtr target, string name, string value);
	
	public class Hosting {

		[DllImport ("moon")]
		internal extern static IntPtr surface_attach (IntPtr surface, IntPtr toplevel);

		[DllImport ("moon")]
		internal extern static IntPtr xaml_create_from_file (string file, bool create_namescope,
				CreateElementCallback ce, SetAttributeCallback sa, HookupEventCallback hue, ref int kind_type);

		[DllImport ("moon")]
		internal extern static IntPtr xaml_create_from_str (string file, bool create_namescope,
				CreateElementCallback ce, SetAttributeCallback sa, HookupEventCallback hue, ref int kind_type);

		[DllImport("moon",EntryPoint="dependency_object_get_name")]
		internal extern static IntPtr _dependency_object_get_name (IntPtr obj);

		internal static string dependency_object_get_name (IntPtr obj)
		{
			IntPtr p = _dependency_object_get_name (obj);
			
			if (p == IntPtr.Zero){
				Console.WriteLine ("Got a null");
				return null;
			}
			return Marshal.PtrToStringAuto (p);
		}
		
                // [DONE] 1. Load XAML file 
                // 2. Make sure XAML file exposes a few new properites:
                //    a. Loaded  (this is the method to call)
                //    b. x:Class (it specifies the assembly to request from the server, and the class to load).
                // 3. Request the browser to download the files
                // 4. From the assembly, lookup the specified class
                // 5. From the class lookup the specified method
                // 6. Run the method
                // 7. If none of those properties are there, we can return

		public static Loader CreateXamlFileLoader (IntPtr plugin, IntPtr surface, string filename)
		{
			Loader loader = new Loader (plugin, surface, filename, "");

			return loader;
		}

		public static Loader CreateXamlStrLoader (IntPtr plugin, IntPtr surface, string contents)
		{
			Loader loader = new Loader (plugin, surface, "", contents);

			return loader;
		}
	}

	public class Loader {
		DomainInstance rl;
		
		public Loader (IntPtr plugin, IntPtr surface, string filename, string contents)
		{
			AppDomain a = AppDomain.CreateDomain ("moonlight-" + plugin);

			rl =  (DomainInstance) a.CreateInstanceAndUnwrap (
				Assembly.GetExecutingAssembly ().FullName,
				"Moonlight.DomainInstance");

			rl.Setup (plugin, surface, filename, contents);
		}

		public void InsertMapping (string key, string name)
		{
			rl.InsertMapping (key, name);
		}

		public string TryLoad (out int error)
		{
			return rl.TryLoad (out error);
		}
	}
	
	public class DomainInstance : MarshalByRefObject {
		static IntPtr plugin, surface;
		string filename;
		string contents;
		string missing;

		CreateElementCallback create_element_callback;
		SetAttributeCallback set_attribute_callback;
		HookupEventCallback hookup_event_callback;
		
		Hashtable h = new Hashtable ();

		static public IntPtr PluginHandle {
			get {
				return plugin;
			}
		}
		
		public DomainInstance ()
		{
		}
		
		public void Setup (IntPtr _plugin, IntPtr _surface, string filename, string contents)
		{
			plugin = _plugin;
			surface = _surface;
			this.filename = filename;
			this.contents = contents;

			create_element_callback = new CreateElementCallback (create_element);
			set_attribute_callback = new SetAttributeCallback (set_attribute);
			hookup_event_callback = new HookupEventCallback (hookup_event);
			
			Type t = typeof (System.Windows.Interop.BrowserHost);
			MethodInfo m = t.GetMethod ("SetPluginHandle", BindingFlags.Static | BindingFlags.NonPublic);
			if (m == null){
				Console.WriteLine ("Having problems");
			} else {
				Console.WriteLine ("The plugin handle is {0}", _plugin);
				m.Invoke (null, new object [] { _plugin });
			} 
		}

		public void InsertMapping (string key, string name)
		{
			Console.WriteLine ("Inserting mapping {0} and {1}", key, name);
			if (h.Contains (key)){
				Console.WriteLine ("Inserting a duplicate key? {0}-{1}", key, name);
				return ;
			}
			
			h [key] = name;
		}
		
		//
		// On error it sets the @error ref to 1
		// Returns the filename that we are missing
		//
		public string TryLoad (out int error)
		{
			Console.WriteLine ("Loader.TryLoad: {0} {1}", surface, filename);
			int kind = 0;

			missing = null;
			error = -1;
			
			IntPtr element;
			if (filename != String.Empty)
				element = Hosting.xaml_create_from_file (filename, true, create_element_callback, set_attribute_callback, hookup_event_callback, ref kind);
			else
				element = Hosting.xaml_create_from_str (contents, true, create_element_callback, set_attribute_callback, hookup_event_callback, ref kind);

			// HERE:
			//     Insert code to check the output of from_file
			//     to see which assembly we are missing
			if (missing != null){
				error = 0;
				return missing;
			}
			
			if (element == IntPtr.Zero){
				Console.WriteLine ("Could not load xaml file");
				return null;
			}

			string xname = Hosting.dependency_object_get_name (element);
			if (xname != "Canvas"){
				Console.WriteLine ("return value is not a Canvas, its a {0}", xname);
				return null;
			}

			MethodInfo m = typeof (Canvas).GetMethod ("FromPtr", BindingFlags.Static | BindingFlags.NonPublic);
			Canvas c = (Canvas) m.Invoke (null, new object [] { element });
			if (c == null){
				Console.WriteLine ("Could not invoke Canvas.FromPtr");
				return null;
			}

			m = typeof (Canvas).Assembly.GetType ("Mono.Hosting").
				GetMethod ("GetNativeObject", BindingFlags.Static | BindingFlags.NonPublic);
			
			IntPtr p = (IntPtr) m.Invoke (null, new object [] { c });

			error = 0;
			Hosting.surface_attach (surface, p);
			return null;
		}

		private IntPtr create_element (string xmlns, string name)
		{
			string ns;
			string type_name;
			string asm_name;
			string asm_path;

			ParseXmlns (xmlns, out type_name, out ns, out asm_name);

			if (asm_name == null) {
				Console.WriteLine ("unable to parse xmlns string: '{0}'", xmlns);
				return IntPtr.Zero;
			}

			asm_path = h [asm_name] as string;

			if (asm_path == null) {
				missing = asm_name;
				Console.WriteLine ("Assembly not available:  {0}", asm_name);
				return IntPtr.Zero;
			}

			Assembly clientlib = Assembly.LoadFile (asm_path);
			if (clientlib == null) {
				Console.WriteLine ("could not load client library: '{0}'", asm_path);
				return IntPtr.Zero;
			}

			if (type_name != null)
				name = type_name;

			if (ns != null)
				name = String.Concat (ns, ".", name);

			DependencyObject res = (DependencyObject) clientlib.CreateInstance (name);

			if (res == null) {
				Console.WriteLine ("unable to create object instance:  '{0}'", name);
				return IntPtr.Zero;
			}

			MethodInfo m = typeof (Canvas).Assembly.GetType ("Mono.Hosting").GetMethod ("GetNativeObject",
					BindingFlags.Static | BindingFlags.NonPublic);
			IntPtr p = (IntPtr) m.Invoke (null, new object [] { res });
			return p;
		}

		private void set_attribute (IntPtr target_ptr, string name, string value)
		{
			MethodInfo m = typeof (DependencyObject).GetMethod ("Lookup",
					BindingFlags.Static | BindingFlags.NonPublic, null, new Type [] { typeof (IntPtr) }, null);
			DependencyObject target = (DependencyObject) m.Invoke (null, new object [] { target_ptr });

			if (target == null) {
				Console.WriteLine ("unable to create target object from: 0x{0}", target_ptr);
				return;
			}

			PropertyDescriptor pd = TypeDescriptor.GetProperties (target).Find (name, true);

			if (pd == null) {
				Console.WriteLine ("unable to set property ({0}) no property descriptor found", name);
				return;
			}

			if (!pd.Converter.CanConvertFrom (typeof (string))) {
				//
				// MS does not seem to handle this yet either, but I think a logical improvement
				// here is to call back into unmanaged code something like xaml_create_object_from_str
				// with the attribute string, and see if the managed code can parse it, this would
				// allow you to stick things like Colors and KeySplines on your object and still have
				// custom setters
				//
				Console.WriteLine ("unable to convert property '{0}' from a string", name);
				return;
			}

			pd.SetValue (target, pd.Converter.ConvertFrom (value));
		}

		private void hookup_event (IntPtr target_ptr, string name, string value)
		{
			MethodInfo m = typeof (DependencyObject).GetMethod ("Lookup",
					BindingFlags.Static | BindingFlags.NonPublic, null, new Type [] { typeof (IntPtr) }, null);
			DependencyObject target = (DependencyObject) m.Invoke (null, new object [] { target_ptr });

			if (target == null) {
				Console.WriteLine ("hookup event unable to create target object from: 0x{0}", target_ptr);
				return;
			}

			EventInfo src = target.GetType ().GetEvent (name);
			if (src == null) {
				Console.WriteLine ("hookup event unable to find event to hook to: '{0}'.", name);
				return;
			}

			Delegate d = Delegate.CreateDelegate (src.EventHandlerType, target, value);
			if (d == null) {
				Console.WriteLine ("hookup event unable to create delegate.");
				return;
			}

			src.AddEventHandler (target, d);
		}

		internal static void ParseXmlns (string xmlns, out string type_name, out string ns, out string asm)
		{
			type_name = null;
			ns = null;
			asm = null;

			string [] decls = xmlns.Split (';');
			foreach (string decl in decls) {
				if (decl.StartsWith ("clr-namespace:")) {
					ns = decl.Substring (14, decl.Length - 14);
					continue;
				}
				if (decl.StartsWith ("assembly=")) {
					asm = decl.Substring (9, decl.Length - 9);
					continue;
				}
				type_name = decl;
			}
		}
	}
		
	class Moonlight {
		
		static int count;
		
		static void CreateInstance ()
		{
			// 1. Create a new AppDomain.
			// 2. Invoke a method in the new AppDomain to load the XAML file
		}
		
		static void Main ()
		{
			Console.WriteLine ("Running Moonlight.cs {0}", count++);
		}
	}
}
