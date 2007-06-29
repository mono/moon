using System;
using Gtk;
using Gtk.Moonlight;
using System.Windows;
using System.Windows.Media;
using System.Windows.Controls;
using System.Windows.Shapes;
using System.Windows.Input;
using System.IO;
using System.Collections.Generic;

class X {
	static GtkSilver silver;
	static Window w;
	static List<TestBase> tests;
	static int i=0;
	
	static void Load (TestBase test)
	{
		w.Title = test.GetType().Name;
		silver.Attach (test.Canvas);
	}
	
	static void Main (string [] args)
	{
		Application.Init ();

		w = new Window ("Top");
		Box vb = new VBox ();
		Box hb = new HBox ();
		vb.Add (hb);

		
		Button quit = new Button ("Quit");
		quit.Clicked += delegate { Application.Quit (); };
		hb.Add (quit);
		Button next = new Button ("Next");
		Button previous = new Button ("Previous");
		previous.Sensitive = false;
			hb.Add (previous);
			previous.Clicked += delegate {
				Load (tests [--i]);
				previous.Sensitive = (i != 0);
				next.Sensitive = true;
			};

		
			hb.Add (next);
			next.Clicked += delegate {
				Load (tests [++i]);
				next.Sensitive = (i < (tests.Count-1));
				previous.Sensitive = true;
			};
		silver = new GtkSilver (400, 400);
		vb.Add (silver);
		w.Add (vb);
		w.ShowAll ();



		//
		// Now, the SL API
		//
		tests = new List<TestBase>();

		tests.Add(new LineColorTest());
		tests.Add(new LinearBrushTest());
		tests.Add(new RadialBrushTest());
		tests.Add(new AnimationLineTest());
		tests.Add(new ZIndexChangeTest());

		Load (tests[0]);		
		Application.Run ();
	}
}
