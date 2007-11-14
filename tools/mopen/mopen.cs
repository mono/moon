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
//      images from. 
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
using System.Collections.Generic;
using Gtk;
using Cairo;
using Gtk.Moonlight;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Controls;
using System.Windows.Input;
using System.IO;
using System.Collections;
using Mono;
using System.Reflection;
using System.Xml;

class MonoOpen {
	static Window window;
	static bool fixedwindow = false;
	static int width = -1;
	static int height = -1;
	static bool transparent = false;	
	static bool desklet = false;
	static bool parse_only = false;
	static List<string> story_names = null;
	static List<Storyboard> storyboards = null;
	static int current_storyboard = 0;
	static bool all_stories = false;
	
	static void Help ()
	{
		Console.WriteLine ("Usage is: mopen [args] [file.xaml|dirname]\n\n" +
				   "Arguments are:\n" +
				   "   --desklet       Remove window decoration for desklets use\n" +
				   "   --fixed         Disable window resizing\n"  +
				   "   --geometry WxH  Overrides the geometry to be W x H pixels\n" +
				   "   --host NAME     Specifies that this file should be loaded in host NAME\n" +
				   "   --parseonly     Only parse (don't display) the XAML input\n" + 
				   "   --story N1[,Nx] Plays the storyboard name N1, N2, .. Nx when the clicked\n" +
				   "   -s,--stories    Automatically prepare to play all stories on click\n" + 
				   "   --transparent   Transparent toplevel\n" 
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

	static bool can_dragging = false;

	static void HandleMouseLeftButtonDown (object sender, MouseEventArgs e)
	{
		can_dragging = true;
	}

	static void HandleMouseLeftButtonUp (object sender, MouseEventArgs e)
	{
		can_dragging = false;
	}

	static void HandleMouseMove (object sender, MouseEventArgs e)
	{
		if (!can_dragging)
			return;

		int x = 0, y = 0;
		window.GetPosition (out x, out y);
		System.Windows.Point p = e.GetPosition (sender as UIElement);
		window.BeginMoveDrag (1, (int) p.X + x,	(int) p.Y + y, 0);

		can_dragging = false;
	}

	static void ConfigureDeskletWindow (Gtk.Window window)
	{
		window.Decorated = false;
		window.SkipPagerHint = true;
		window.SkipTaskbarHint = true;
	}

	static int LoadXaml (string file, ArrayList args)
	{
		Application.Init ();
		GtkSilver.Init ();
		window = new Window (file);

		string full = System.IO.Path.GetFullPath (file);
		string dir = System.IO.Path.GetDirectoryName (full);
		
		Moonlight.RegisterLoader (delegate (string asm_file) {
			if (System.IO.Path.IsPathRooted (asm_file)){
				return Assembly.LoadFile (asm_file);
			} else {
				return Assembly.LoadFile (System.IO.Path.Combine (dir, asm_file));
			}
		}, delegate (string path) {
			if (System.IO.Path.IsPathRooted (path)){
				return new FileStream (path, FileMode.Open, FileAccess.Read);
			} else {
				return new FileStream (System.IO.Path.Combine (dir, path), FileMode.Open, FileAccess.Read);
			}
		});

		if (transparent) {
			CompositeHelper.SetRgbaColormap (window);
			window.AppPaintable = true;
			window.ExposeEvent += HandleExposeEvent;
		}    	

		if (desklet) {
			ConfigureDeskletWindow (window);
		}

		window.DeleteEvent += delegate {
			Application.Quit ();
		};

		GtkSilver silver = new GtkSilver (400, 400);
		Canvas canvas;
		
		if (!silver.LoadFile (file, out canvas)) {
			Console.Error.WriteLine ("mopen: Could not load xaml");
			return 1;
		}

		if (parse_only)
			return 0;

		if (width == -1)
			width = (int) canvas.Width;
		if (height == -1)
			height = (int) canvas.Height;

		if (width > 0 && height > 0) {
			silver.Resize (width, height);
			window.Resize (width, height);
		} 

		if (transparent){
			silver.AppPaintable = true;
			silver.Transparent = true;
		}

		if (desklet) {
			canvas.MouseLeftButtonDown += new MouseEventHandler (HandleMouseLeftButtonDown);
			canvas.MouseLeftButtonUp += new MouseEventHandler (HandleMouseLeftButtonUp);
			canvas.MouseMove += new MouseEventHandler (HandleMouseMove);
		}

		window.Add (silver);

		window.ShowAll ();

		if (story_names != null){
			storyboards = new List<Storyboard> ();
			
			foreach (string story in story_names){
				object o = canvas.FindName (story);
				Storyboard sb = o as Storyboard;

				if (sb == null){
					Console.Error.WriteLine ("mopen: there is no Storyboard object named {0} in the XAML file", story);
					return 1;
				}
				sb.Completed += delegate {
					window.Title = String.Format ("Storyboard {0} completed", current_storyboard-1);
				};
				
				storyboards.Add (sb);
			};

			canvas.MouseLeftButtonUp += delegate {
				if (current_storyboard == storyboards.Count)
					current_storyboard = 0;
				if (current_storyboard == storyboards.Count)
					return;
				
				window.Title = String.Format ("Storyboard {0} running", current_storyboard);
				storyboards [current_storyboard++].Begin ();
			};
		}
		
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
		string [] names;
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

			case "--story":
				if (i+1 == args.Length){
					Console.Error.WriteLine ("mopen: --story flag takes the name of the storyboard to run");
					return 1;
				}
				i++;
				if (story_names == null)
					story_names = new List<string> ();
				names = args [i].Split (new Char [] { ','});
				foreach (string s in names)
					story_names.Add (s);
				break;

			case "--stories":
			case "-s":
				if (story_names == null)
					story_names = new List<string> ();
				all_stories = true;
				break;
					
			case "--host":
				if (i+1 == args.Length){
					Console.WriteLine ("mopen: host flag `{0}' takes an argument", args [i]);
					return 1;
				}
				// FIXME: implement this
				Console.WriteLine ("--host not implemented");
				break;
				
			case "--parseonly": case "--parse-only":
				parse_only = true;
				break;

			default:
				if (file == null)
					file = args [i];
				else
					cmdargs.Add (args [i]);
				break;
			}
		}

		if (File.Exists (file)){
			if (all_stories){
				string xns = "http://schemas.microsoft.com/winfx/2006/xaml";
				XmlDocument d = new XmlDocument ();
				d.Load (file);
				
				XmlNamespaceManager mgr = new XmlNamespaceManager (d.NameTable);
				mgr.AddNamespace ("xaml", "http://schemas.microsoft.com/winfx/2006/xaml/presentation");
				mgr.AddNamespace ("x", xns);
				XmlNodeList nodes = d.SelectNodes ("//xaml:Storyboard[@x:Name]", mgr);
				foreach (XmlNode n in nodes){
					XmlAttribute a = n.Attributes ["Name", xns];
					if (a != null)
						story_names.Add (a.Value);
				}
			}
			
			return DoLoad (file, cmdargs);
		}

		if (Directory.Exists (file)){
			string combine = System.IO.Path.Combine (file, "default.xaml");
			if (File.Exists (combine))
				return DoLoad (combine, cmdargs);
		}

		string path = Environment.GetEnvironmentVariable ("PATH");
		string [] dirs = path.Split (new char [] {':'});
		foreach (string dir in dirs){
			string combine = System.IO.Path.Combine (dir, "default.xaml");

			if (File.Exists (combine))
				return DoLoad (combine, cmdargs);
		}
		
		Console.Error.WriteLine ("mopen: Nothing to do");
		return 1;
	}
}
