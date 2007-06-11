using System;
using System.Runtime.InteropServices;
using System.Reflection;
using System.Windows.Controls;

namespace Moonlight {

	public class Hosting {

		[DllImport ("moon")]
		extern static IntPtr surface_attach (IntPtr surface, IntPtr toplevel);

		[DllImport ("moon")]
		extern static IntPtr xaml_create_from_file (string file, ref int kind_type);

                // [DONE] 1. Load XAML file 
                // 2. Make sure XAML file exposes a few new properites:
                //    a. Loaded  (this is the method to call)
                //    b. x:Class (it specifies the assembly to request from the server, and the class to load).
                // 3. Request the browser to download the files
                // 4. From the assembly, lookup the specified class
                // 5. From the class lookup the specified method
                // 6. Run the method
                // 7. If none of those properties are there, we can return

		public static void FromXaml (IntPtr surface, string s)
		{
			Console.WriteLine ("LoadFromXaml: {0} {1}", surface, s);

			int kind = 0;
			
			IntPtr x = xaml_create_from_file (s, ref kind);
			if (x == IntPtr.Zero){
				Console.WriteLine ("Could not load xaml file");
				return;
			}
			if (kind != 18){
				Console.WriteLine ("return value is not 18 (Canvas)");
				return;
			}
			
			MethodInfo m = typeof (Canvas).GetMethod ("FromPtr", BindingFlags.Static | BindingFlags.NonPublic);
			Canvas c = (Canvas) m.Invoke (null, new object [] { x });
			if (c == null){
				Console.WriteLine ("Could not invoke Canvas.FromPtr");
				return;
			}

			m = typeof (Canvas).Assembly.GetType ("Mono.Hosting").
				GetMethod ("GetNativeObject", BindingFlags.Static | BindingFlags.NonPublic);
			IntPtr p = (IntPtr) m.Invoke (null, new object [] { c });
			
			surface_attach (surface, p);
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
