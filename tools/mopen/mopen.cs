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
using GLib;
using Gtk;
using Cairo;
using Moonlight.Gtk;
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
using System.Net;
using Application = Gtk.Application;
using NDesk.Options;

class MonoOpen {
	static Window window;
	static MoonlightHost moon_host;
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
	static bool sync = false;
	static int timeout = -1;
	
	static void ShowHelp (OptionSet o)
	{
		Console.WriteLine ("Usage is: mopen [args] [file.xaml|file.xap|dirname]\n\n" +
				   "Arguments are:\n");
		o.WriteOptionDescriptions (Console.Out);
	}

	[GLib.ConnectBefore]
	static void HandleExposeEvent (object sender, ExposeEventArgs expose_args)
	{
		 Widget w = (Widget)sender;
	    
		 using (Cairo.Context ctx = CompositeHelper.Create (w.GdkWindow)){
			 double opacity = 0;
			 ctx.Operator = Cairo.Operator.Source;
			 if (moon_host.Content is Moon.Windows.Desktop.Window)
				 opacity = ((Moon.Windows.Desktop.Window)moon_host.Content).WindowOpacity;
			 ctx.Color = new Cairo.Color (1.0, 1.0, 1.0, opacity);
			 CompositeHelper.Region (ctx, expose_args.Event.Region);
			 ctx.Fill ();
		 }
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

	static int LoadXaml (string file, List<string> args)
	{
		string [] test = { "" };

		if (sync)
			test [0] = "--sync";

		Application.Init ("mopen", ref test);
		MoonlightRuntime.Init ();
		window = new Window (file);
		window.SetDefaultSize (400, 400);

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

		moon_host = new MoonlightHost ();

		try {
			moon_host.LoadXamlFromFile (file);
		} catch (Exception e) {
			Console.Error.WriteLine ("mopen: Could not load xaml: {0}", e.Message);
			return 1;
		}

		FrameworkElement top = moon_host.Content;

		if (parse_only)
			return 0;

		if (width == -1)
			width = (int) top.Width;
		if (height == -1)
			height = (int) top.Height;

		if (width > 0 && height > 0) {
			moon_host.SetSizeRequest (width, height);
			window.Resize (width, height);
		} 

		if (transparent){
			moon_host.AppPaintable = true;
			moon_host.Transparent = true;
		}

		if (desklet) {
			top.MouseLeftButtonDown += new MouseButtonEventHandler (HandleMouseLeftButtonDown);
			top.MouseLeftButtonUp += new MouseButtonEventHandler (HandleMouseLeftButtonUp);
			top.MouseMove += new MouseEventHandler (HandleMouseMove);
		}

		window.Add (moon_host);

		window.ShowAll ();

		if (story_names != null){
			storyboards = new List<Storyboard> ();
			
			foreach (string story in story_names){
				object o = top.FindName (story);
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

			top.MouseLeftButtonUp += delegate {
				if (current_storyboard == storyboards.Count)
					current_storyboard = 0;
				if (current_storyboard == storyboards.Count)
					return;
				
				window.Title = String.Format ("Storyboard {0} running", current_storyboard);
				storyboards [current_storyboard++].Begin ();
			};
		}

		if (timeout > 0)
			GLib.Timeout.Add ((uint)(timeout * 1000), new TimeoutHandler (Quit));

		Application.Run ();
		return 0;
	}

	static int LoadXap (string file, List<string> args)
	{
		string [] test = { "" };

		if (sync)
			test [0] = "--sync";

		Application.Init ("mopen", ref test);
		MoonlightRuntime.Init ();
		window = new Window (file);
		window.SetDefaultSize (400, 400);

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

		moon_host = new MoonlightHost ();

		try {
			moon_host.LoadXap (file);
		} catch (Exception e) {
			Console.Error.WriteLine ("mopen: Could not load xaml: {0}", e.Message);
			return 1;
		}

		System.Windows.Application app = moon_host.Application;
		FrameworkElement top = (FrameworkElement)app.RootVisual;

		if (top is Moon.Windows.Desktop.Window) {
			var moonwindow = ((Moon.Windows.Desktop.Window)top);
			/* special window handling mode */
			moonwindow.IsActive = window.IsActive;

			Wnck.Screen.Default.ActiveWindowChanged += delegate {
				moonwindow.IsActive = window.IsActive;
			};

			moonwindow.PositionChanged += delegate {
				window.Move ((int)moonwindow.Position.X, (int)moonwindow.Position.Y);
			};

			moonwindow.SizeChanged += delegate {
				window.Resize ((int)moonwindow.Size.Width, (int)moonwindow.Size.Height);
			};

			moonwindow.WindowOpacityChanged += delegate {
				moon_host.QueueDraw ();
			};

			if (!transparent) {
				CompositeHelper.SetRgbaColormap (window);
				window.AppPaintable = true;
				window.ExposeEvent += HandleExposeEvent;

				moon_host.AppPaintable = true;
				moon_host.Transparent = true;
			}
		}

		if (parse_only)
			return 0;

		if (width == -1)
			width = (int) top.Width;
		if (height == -1)
			height = (int) top.Height;

		if (width > 0 && height > 0) {
			moon_host.SetSizeRequest (width, height);
			window.Resize (width, height);
		} 

		if (transparent){
			moon_host.AppPaintable = true;
			moon_host.Transparent = true;
		}

		if (desklet) {
			top.MouseLeftButtonDown += new MouseButtonEventHandler (HandleMouseLeftButtonDown);
			top.MouseLeftButtonUp += new MouseButtonEventHandler (HandleMouseLeftButtonUp);
			top.MouseMove += new MouseEventHandler (HandleMouseMove);
		}

		window.Add (moon_host);

		window.ShowAll ();

		if (story_names != null){
			storyboards = new List<Storyboard> ();
			
			foreach (string story in story_names){
				object o = top.FindName (story);
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

			top.MouseLeftButtonUp += delegate {
				if (current_storyboard == storyboards.Count)
					current_storyboard = 0;
				if (current_storyboard == storyboards.Count)
					return;
				
				window.Title = String.Format ("Storyboard {0} running", current_storyboard);
				storyboards [current_storyboard++].Begin ();
			};
		}

		if (timeout > 0)
			GLib.Timeout.Add ((uint)(timeout * 1000), new TimeoutHandler (Quit));

		Application.Run ();
		return 0;
	}

	static bool Quit ()
	{
		Application.Quit ();
		return true;
	}

	static bool ParseGeometry (string geometry, out int width, out int height)
	{
		width = 100;
		height = 100;
		
		int p = geometry.IndexOf ('x');
		if (p == -1){
			Console.Error.WriteLine ("Invalid geometry passed: {0}", geometry);
		}
		try {
			width = Int32.Parse (geometry.Substring (0, p));
			height = Int32.Parse (geometry.Substring (p+1));
		} catch {
		}
		
		return true;
	}
	
	//
	// TODO, if a host is specified, look for it, if not,
	// create the domain ourselves and listen to requests.
	//
	static int DoLoad (string file, List<string> args)
	{
		//
		// Here:
		//    implement loading the DLL or executanle, search in path perhaps?
		//
		if (file.EndsWith (".dll"))
			return 1;
		else if (file.EndsWith (".xaml"))
			return LoadXaml (file, args);
		else if (file.EndsWith (".xap"))
			return LoadXap (file, args);

		else
			return 1;
	}
	
	static int Main (string [] args)
	{
		List<string> cmdargs = new List<string> ();
		string [] names;
		string file = null;
		bool help = false;
		
		var p = new OptionSet () {
			{ "h|?|help", v => help = v != null },
			{ "f|fixed", "Disable window resizing", v => fixedwindow = true },
			{ "sync", "Make the gdk connection synchronous", v => sync = true },
			{ "d|desklet", "Remove window decoration for desklets use", v => { desklet = true;  transparent = true; } },
			{ "t|transparent", "Transparent toplevel", v => transparent = true },
			{ "g|geometry=", "Overrides the geometry to be W x H pixels", v => ParseGeometry (v, out width, out height) },
			{ "story=", "Plays the storyboard name N1[,N2,N3], .. Nx when the clicked", v => {
				if (story_names == null)
					story_names = new List<string> ();
				names = v.Split (new Char [] { ','});
				foreach (string s in names)
					story_names.Add (s);
				}
			},
			{ "s|stories", "Automatically prepare to play all stories on click", v => { story_names = new List<string> (); all_stories = true; } },
			{ "parseonly", "Only parse (don't display) the XAML input", v => parse_only = true },
			{ "timeout=", "Time, in seconds, before closing the window", v => {
				if (!Int32.TryParse (v, out timeout)) {
					Console.Error.WriteLine ("mopen: invalid timeout value `{0}'", v);
					Environment.Exit (1);
				}
				}
			},
		};

		List<string> rest;
		try {
			rest = p.Parse (args);
		} catch (OptionException){
			Console.Error.WriteLine ("Try `mxap --help' for more information.");
			return 1;
		}
		
		if (help || rest.Count == 0){
			ShowHelp (p);
			return 1;
		}

		if (rest.Count > 1){
			file = rest [0];
			rest.RemoveAt (0);
			cmdargs = rest;
		} else {
			file = rest [0];
		}

		if (File.Exists (file)) {
			if (all_stories && file.EndsWith (".xaml")) {
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
		
		Uri uri = new Uri (file, UriKind.RelativeOrAbsolute);

		if (uri.IsAbsoluteUri) {
			WebClient client = new WebClient ();
			string extension = file.Substring (file.LastIndexOf ("."));
			string tmpfile = System.IO.Path.Combine (System.IO.Path.GetTempPath (), string.Format ("{0}{1}", Guid.NewGuid ().ToString (), extension));
			int ret = 0;

			try {
				client.DownloadFile (uri, tmpfile);
				ret = DoLoad (tmpfile, cmdargs);
			} catch {
				ret = 1;
			} finally {
				File.Delete (tmpfile);
			}

			return ret;
		}
		
		Console.Error.WriteLine ("mopen: Nothing to do");
		return 1;
	}
}
