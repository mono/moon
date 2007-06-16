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
using System.Windows.Controls;
using System.Collections;

namespace Moonlight {

	internal delegate IntPtr CreateElementCallback (string xmlns, string name);

	public class Hosting {

		[DllImport ("moon")]
		internal extern static IntPtr surface_attach (IntPtr surface, IntPtr toplevel);

		[DllImport ("moon")]
		internal extern static IntPtr xaml_create_from_file (string file, bool create_namescope, CreateElementCallback ce, ref int kind_type);

		[DllImport("moon")]
		internal extern static string dependency_object_get_name (IntPtr obj);
		
                // [DONE] 1. Load XAML file 
                // 2. Make sure XAML file exposes a few new properites:
                //    a. Loaded  (this is the method to call)
                //    b. x:Class (it specifies the assembly to request from the server, and the class to load).
                // 3. Request the browser to download the files
                // 4. From the assembly, lookup the specified class
                // 5. From the class lookup the specified method
                // 6. Run the method
                // 7. If none of those properties are there, we can return

		public static Loader CreateXamlLoader (IntPtr plugin, IntPtr surface, string s)
		{
			Loader l = new Loader (plugin, surface, s);

			return l;
		}
	}

	

	public class Loader {
		IntPtr plugin, surface;
		string s;
		string missing;

		CreateElementCallback create_element_callback;
		
		Hashtable h = new Hashtable ();
		
		public Loader (IntPtr plugin, IntPtr surface, string s)
		{
			this.plugin = plugin;
			this.surface = surface;
			this.s = s;

			create_element_callback = new CreateElementCallback (create_element);
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
			Console.WriteLine ("Loader.TryLoad: {0} {1}", surface, s);
			int kind = 0;

			missing = null;
			error = -1;
			IntPtr x = Hosting.xaml_create_from_file (s, true, create_element, ref kind);

			Console.WriteLine ("Got: {0}", x);
			// HERE:
			//     Insert code to check the output of from_file
			//     to see which assembly we are missing
			if (missing != null){
				error = 0;
				return missing;
			}
			
			if (x == IntPtr.Zero){
				Console.WriteLine ("Could not load xaml file");
				return null;
			}

			string xname = Hosting.dependency_object_get_name (x);
			if (xname != "Canvas"){
				Console.WriteLine ("return value is not a Canvas, its a {0}", xname);
				return null;
			}

			MethodInfo m = typeof (Canvas).GetMethod ("FromPtr", BindingFlags.Static | BindingFlags.NonPublic);
			Canvas c = (Canvas) m.Invoke (null, new object [] { x });
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
			string asm_name;
			string asm_path;
			string fullname;

			ParseXmlns (xmlns, out ns, out asm_name);

			if (ns == null || asm_name == null)
				return IntPtr.Zero;

			asm_path = h [asm_name] as string;

			if (asm_path == null) {
				missing = asm_name;
				Console.WriteLine ("unable to parse xmlns string: '{0}'", xmlns);
				return IntPtr.Zero;
			}
				
			Assembly clientlib = Assembly.LoadFile (asm_path);

			if (clientlib == null) {
				Console.WriteLine ("could not load client library: '{0}'", asm_path);
				return IntPtr.Zero;
			}

			fullname = String.Concat (ns, ".", name);
			DependencyObject res = (DependencyObject) clientlib.CreateInstance (fullname);

			if (res == null) {
				Console.WriteLine ("unable to create object instance:  '{0}'", fullname);
				return IntPtr.Zero;
			}

			MethodInfo m = typeof (Canvas).Assembly.GetType ("Mono.Hosting").
				GetMethod ("GetNativeObject", BindingFlags.Static | BindingFlags.NonPublic);
			
			IntPtr p = (IntPtr) m.Invoke (null, new object [] { res });
			return p;
		}

		internal static void ParseXmlns (string xmlns, out string ns, out string asm)
		{
			ns = null;
			asm = null;

			string [] decls = xmlns.Split (';');
			foreach (string decl in decls) {
				if (decl.StartsWith ("clr-namespace:")) {
					ns = decl.Substring (14, decl.Length - 14);
				}
				if (decl.StartsWith ("assembly=")) {
					asm = decl.Substring (9, decl.Length - 9);
				}
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
