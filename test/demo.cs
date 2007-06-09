using System;
using Gtk;
using System.Windows;
using System.Windows.Media;
using System.Windows.Controls;
using System.Windows.Shapes;
using System.IO;

class X {
	static void Main (string [] args)
	{
		Application.Init ();

		Window w = new Window ("Top");
		GtkSilver silver = new GtkSilver (400, 400);
		w.Add (silver);
		w.ShowAll ();

		//
		// Now, the SL API
		//
		if (args.Length == 0){
			Canvas c = new Canvas ();
			silver.Attach (c);
			Rectangle r = new Rectangle ();

			r.Width = 100;
			r.Height = 100;
			r.Stroke = new SolidColorBrush (Color.FromRgb (255, 0, 0));
			c.Children.Add (r);

			r.SetValue (Canvas.LeftProperty, 100.0);
			r.SetValue (Canvas.TopProperty, 100.0);

		} else {
			string xaml = null;
			
			try {
				using (FileStream fs = File.OpenRead (args [0])){
					using (StreamReader sr = new StreamReader (fs)){
						xaml = sr.ReadToEnd ();
					}
				}
			} catch (Exception e) {
				Console.Error.WriteLine ("Error loading XAML file {0}: {1}", args [0], e.GetType());
				return;
			}
			
			if (xaml == null){
				Console.Error.WriteLine ("Error loading XAML file {0}", args [0]);
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
			silver.Attach ((Canvas) d);
		}
		
		Application.Run ();
	}
}
