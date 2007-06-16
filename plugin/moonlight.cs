using System;
using System.Runtime.InteropServices;
using System.Reflection;
using System.Windows.Controls;
using System.Collections;

namespace Moonlight {

	public class Hosting {

		[DllImport ("moon")]
		internal extern static IntPtr surface_attach (IntPtr surface, IntPtr toplevel);

		[DllImport ("moon")]
		internal extern static IntPtr xaml_create_from_file (string file, ref int kind_type);

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
		
		Hashtable h = new Hashtable ();
		
		public Loader (IntPtr plugin, IntPtr surface, string s)
		{
			this.plugin = plugin;
			this.surface = surface;
			this.s = s;
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
			error = 0;

#if false
			//
			// The follwoing two lines emulate the XAML parsing
			// barfing and reporting that it needs a new DLL
			//
			if (!h.Contains ("test-custom-element.dll"))
				return "test-custom-element.dll";
#endif
			
			int kind = 0;
			string missing = null;

			error = -1;
			IntPtr x = Hosting.xaml_create_from_file (s, ref kind);
			
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
			if (kind != 23){
				Console.WriteLine ("return value is not 23 (Canvas)");
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
