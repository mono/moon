using System;
using Gtk;
using System.Windows;
using System.Windows.Media;
using System.Windows.Controls;
using System.Windows.Shapes;
using System.IO;

class X {
	static GtkSilver silver;
	static Window w;
	
	static void Load (string file)
	{
		string xaml = null;

		Console.WriteLine ("Loading: {0}", file);
		try {
			using (FileStream fs = File.OpenRead (file)){
				using (StreamReader sr = new StreamReader (fs)){
					xaml = sr.ReadToEnd ();
				}
			}
		} catch (Exception e) {
			Console.Error.WriteLine ("Error loading XAML file {0}: {1}", file, e.GetType());
			return;
		}
		
		if (xaml == null){
			Console.Error.WriteLine ("Error loading XAML file {0}", file);
			return;
		}
		DependencyObject d = XamlReader.Load (xaml);
		if (d == null){
			Console.Error.WriteLine ("No dependency object returned from XamlReader");
			return;
		}
		
		if (!(d is Canvas)){
			Console.Error.WriteLine ("No Canvas as root");
			return;
		}
		w.Title = file;
		silver.Attach ((Canvas) d);
	}
	
	static void Main (string [] args)
	{
		int i = 0;
		Application.Init ();

		w = new Window ("Top");
		Box vb = new VBox ();
		Box hb = new HBox ();
		vb.Add (hb);

		if (args.Length > 1){
			Button next = new Button ("Load next file");
			hb.Add (next);
			next.Clicked += delegate {
				Load (args [++i]);
				if (i == args.Length-1)
				next.Sensitive = false;
			};
		}
		
		Button quit = new Button ("Quit");
		quit.Clicked += delegate { Application.Quit (); };
		hb.Add (quit);
		silver = new GtkSilver (400, 400);
		vb.Add (silver);
		w.Add (vb);
		w.ShowAll ();

		//
		// Now, the SL API
		//
		if (args.Length == 0){
			Canvas c = new Canvas ();
			silver.Attach (c);
			Rectangle r = new Rectangle ();

			r.MouseMove += delegate {
				Console.WriteLine ("Demo is moving");
			};

			r.Width = 100;
			r.Height = 100;
			r.Stroke = new SolidColorBrush (Color.FromRgb (255, 0, 0));
			c.Children.Add (r);

			Console.WriteLine (((SolidColorBrush)r.Stroke).Color);
			
			r.SetValue (Canvas.LeftProperty, 100);
			r.SetValue (Canvas.TopProperty, 100);

			Line l = new Line ();
			l.X1 = 100;
			l.Y1 = 200;
			l.Y1 = 30;
			l.Y2 = 90;
			l.Stroke = new SolidColorBrush (Color.FromRgb (0, 255, 0));
			l.StrokeDashArray = new double [] { 1, 4, 10 };

			
			c.Children.Add (l);
		} else {
			Load (args [i]);
		}
		
		Application.Run ();
	}
}
