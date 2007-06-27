//
// mopen.cs: Mono Open Application
//
// Opens a XAML file or an application
//
// Author:
//   Miguel de Icaza (miguel@novell.com)
//
//
// See LICENSE file in the Moonlight distribution for licensing details
//
// TODO:
//    * Implement --host
//    * Implement loading of exes
//    * Implement --fixed
//    * Make it so we can open directories that contain DIR/default.xaml
//      and setup the managed loader to default to this location to load
//      files from.
//
// The idea is that we can write XAML-based applications and launch them with
// this thing, typing:
//
//    $ mopen calendar
//
// Would check if calendar/default.xaml exists, and if it does, configure the
// downloader to default to the directory calendar for obtaining images,
// resources and so on.
//
// If no default.xaml exists, we could check in order: a file with a .dll
// extension, if found, load it;   a file with any of the scripting languages
// extensions that the DLR knows about ".py", ".rb", ".js", ".vb" and load
// the proper compiler to run the code.
//
// The idea behind --host is to load multiple programs in a single instance of
// mono/moonlight using separate appdomains, so multiple applications (or
// desklets) can be loaded into the same process, saving resources.
//
// We could either use DBus# for this, or a simpler mechanism might be to use
// the named features from System.Threading to do this
//

using System;
using Gtk;
using Cairo;
using Gtk.Moonlight;
using System.Windows;
using System.Windows.Media;
using System.Windows.Controls;
using System.Windows.Input;
using System.IO;
using System.Collections;

class MonoOpen {
	static bool fixedwindow = false;
	static int width = -1;
	static int height = -1;
	static bool transparent = false;	
	static bool desklet = false;

	static void Help ()
	{
		Console.WriteLine ("Usage is: mopen [args] [file.xaml|dirname]\n\n" +
				   "Arguments are:\n" +
				   "   --fixed         Disable window resizing\n"  +
				   "   --geometry WxH  Overrides the geometry to be W x H pixels\n" +
				   "   --host NAME     Specifies that this file should be loaded in host NAME\n" +
				   "   --transparent   Transparent toplevel\n" +
				   "   --desklet       Remove window decoration for desklets use\n"
				   );
	}

	[GLib.ConnectBefore]
	static void HandleExposeEvent (object sender, ExposeEventArgs expose_args)
	{
		 Widget w = (Widget)sender;
	    
		 Cairo.Context ctx = CompositeHelper.Create (w.GdkWindow);
		 ctx.Operator = Cairo.Operator.Source;
		 ctx.Color = new Cairo.Color (1.0, 1.0, 1.0, 0.0);
		 CompositeHelper.Region (ctx, expose_args.Event.Region);
		 ctx.Fill ();
	}

	static void HandleButtonPressEvent (object sender, ButtonPressEventArgs args)
	{
		if (!(sender is Gtk.Window))
			return;

		if (args.Event.Button == args.Event.Button) {
			Gtk.Window w = (Gtk.Window)sender;

			if (args.Event.Button == 1) {
				w.BeginMoveDrag ((int) args.Event.Button, 
								(int) args.Event.XRoot, 
								(int) args.Event.YRoot, 
								(uint) args.Event.Time);
			}
		}
	}

	static void ConfigureDeskletWindow (Gtk.Window window)
	{
		window.Decorated = false;
		window.SkipPagerHint = true;
		window.SkipTaskbarHint = true;

		//window.ButtonPressEvent += HandleButtonPressEvent;
	}

	static int LoadXaml (string file, ArrayList args)
	{
		Application.Init ();
		GtkSilver.Init ();
		Window w = new Window (file);

		if (transparent) {
			CompositeHelper.SetRgbaColormap (w);
			w.AppPaintable = true;
			w.ExposeEvent += HandleExposeEvent;
		}    	

		if (desklet) {
			ConfigureDeskletWindow (w);
		}

		w.DeleteEvent += delegate {
			Application.Quit ();
		};

		string xaml = "";
		
		try {
			using (FileStream fs = File.OpenRead (file)){
				using (StreamReader sr = new StreamReader (fs)){
					xaml = sr.ReadToEnd ();
				}
			}
		} catch (Exception e) {
			Console.Error.WriteLine ("mopen: Error loading XAML file {0}: {1}", file, e.GetType());
			return 1;
		}
		
		if (xaml == null){
			Console.Error.WriteLine ("mopen: Error loading XAML file {0}", file);
			return 1;
		}

		DependencyObject d = XamlReader.Load (xaml);
		if (d == null){
			Console.Error.WriteLine ("mopen: No dependency object returned from XamlReader");
			return 1;
		}
		
		if (!(d is Canvas)){
			Console.Error.WriteLine ("mopen: No Canvas as root in the specified file");
			return 1;
		}

		GtkSilver silver;
		Canvas canvas = d as Canvas;

		if (width != -1 && height != -1){
			silver = new GtkSilver (width, height);
			w.Resize (width, height);
		} else if (canvas.Width > 0 && canvas.Height > 0) {
			w.Resize ((int) canvas.Width, (int) canvas.Height);
			silver = new GtkSilver ((int) canvas.Width, (int) canvas.Height);
		} else {
			silver = new GtkSilver (400, 400);
		}

		w.Add (silver);
		silver.Attach (canvas);

		w.ShowAll ();
		Application.Run ();
		return 0;
	}

	static bool ParseGeometry (string geometry, out int width, out int height)
	{
		// FIXME: implement this
		Console.WriteLine ("ParseGeometry not implemented");
		
		width = 100;
		height = 100;

		return true;
	}
	
	//
	// TODO, if a host is specified, look for it, if not,
	// create the domain ourselves and listen to requests.
	//
	static int DoLoad (string file, ArrayList args)
	{
		if (file.EndsWith (".xaml"))
			return LoadXaml (file, args);

		//
		// Here:
		//    implement loading the DLL or executanle, search in path perhaps?
		//
		if (file.EndsWith (".dll")){
		}

		return 1;
	}
	
	static int Main (string [] args)
	{
		ArrayList cmdargs = new ArrayList ();
		string file = null;
		
		if (args.Length < 1){
			Help ();
			return 1;
		}

		for (int i = 0; i < args.Length; i++){
			switch (args [i]){
			case "-h": case "-help": case "--help":
				Help ();
				return 0;

			case "-f": case "--fixed":
				fixedwindow = true;
				break;

			case "--desklet": case "-d":
				desklet = true;
				transparent = true;
				break;
			
			case "--transparent": case "-t":
				transparent = true;
				break;

			case "--geometry": case "-g":
				if (i+1 == args.Length){
					Console.Error.WriteLine ("mopen: geometry flag `{0}' takes an argument", args [i]);
					return 1;
				}
				i++;
				if (ParseGeometry (args [i], out width, out height))
					break;
				else
					return 1;

			case "--host":
				if (i+1 == args.Length){
					Console.WriteLine ("mopen: host flag `{0}' takes an argument", args [i]);
					return 1;
				}
				// FIXME: implement this
				Console.WriteLine ("--host not implemented");
				break;
				
			default:
				if (file == null)
					file = args [i];
				else
					cmdargs.Add (args [i]);
				break;
			}
		}

		if (File.Exists (file))
			return DoLoad (file, cmdargs);

		if (Directory.Exists (file)){
			string combine = Path.Combine (file, "default.xaml");
			if (File.Exists (combine))
				return DoLoad (combine, cmdargs);
		}

		string path = Environment.GetEnvironmentVariable ("PATH");
		string [] dirs = path.Split (new char [] {':'});
		foreach (string dir in dirs){
			string combine = Path.Combine (dir, "default.xaml");

			if (File.Exists (combine))
				return DoLoad (combine, cmdargs);
		}
		
		Console.Error.WriteLine ("mopen: Nothing to do");
		return 1;
	}
}
