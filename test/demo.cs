//
// demo.cs: if no arguments are passed, loads some default stuff
// if arguments are passed it loads the XAML file
// if the XAML file contains a Storyboard with the id "animation"
// pressing the mouse button pauses the animation, releasing it resumes it
//
using System;
using Gtk;
using Gtk.Moonlight;
using System.Windows;
using System.Windows.Media;
using System.Windows.Controls;
using System.Windows.Shapes;
using System.Windows.Input;
using System.IO;
using System.Windows.Media.Animation;

class X {
	static GtkSilver silver;
	static Window w;
	static Label msg;
	
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
		Canvas canvas = (Canvas) d;
		silver.Attach (canvas);

		DependencyObject anim = d.FindName ("animation");
		if (anim != null && anim is Storyboard){
			msg.Text = "Hold the button down to pause the animation";
			canvas.MouseLeftButtonDown += delegate {
				((Storyboard) anim).Pause ();
			};

			canvas.MouseLeftButtonUp += delegate {
				((Storyboard) anim).Resume ();
				Console.WriteLine ("UP");
			};
		}  else
			msg.Text = "";

		Brush enter_fill = new SolidColorBrush (Color.FromRgb (0, 255, 0));
		Brush leave_fill = new SolidColorBrush (Color.FromRgb (30, 90, 90)); // r.Fill;

		Canvas caa = d.FindName ("canvas") as Canvas;
		if (caa != null){
			caa.MouseEnter += delegate {
				Console.WriteLine ("DEMOCS: CANVAS ENTERING");
				caa.Background = new SolidColorBrush (Color.FromRgb (128, 0, 255));
			};
			caa.MouseLeave += delegate {
				Console.WriteLine ("DEMOCS: CANVAS LEAVING");
				caa.Background = leave_fill;
			};
		}
		
		Shape obj = d.FindName ("xform") as Shape;
		Console.WriteLine ("Find Object xform: {0}", obj);
		if(obj != null){
			
			obj.MouseEnter += delegate {
				Console.WriteLine ("OB Enter");
				obj.Fill = enter_fill;
			};

			obj.MouseLeave += delegate {
				Console.WriteLine ("OB Leave");
				obj.Fill = leave_fill;
			};
		}

		if (obj != null && caa != null){
			int i  = 0;
			obj.MouseLeftButtonDown += delegate {
				Console.WriteLine ("---> rect click");
				i++;
			};

			caa.MouseLeftButtonDown += delegate {
				Console.WriteLine ("---> canvas click");
				i++;
			};
		} else {
			Console.WriteLine ("No tc/tr");
		}
	}

	static void OnWindowDelete (object o, DeleteEventArgs args)
	{
		Application.Quit ();
	}
	
	static void Main (string [] args)
	{
		int i = 0;
		Application.Init ();

		w = new Window ("Top");
		w.DeleteEvent += new DeleteEventHandler (OnWindowDelete);

		Box vb = new VBox ();
		msg = new Label ("");
		vb.Add (msg);
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
			c.SetValue (FrameworkElement.WidthProperty, 400);
			c.SetValue (FrameworkElement.HeightProperty, 400);
			silver.Attach (c);

			Rectangle r = new Rectangle ();
			//r.SetValue (Canvas.LeftProperty, 100);
			RotateTransform trans = new RotateTransform ();
			trans.Angle = 45;
			r.RenderTransform = trans; 

			c.MouseMove += delegate (object sender, MouseEventArgs e) {
				//Console.WriteLine ("Canvas Motion: {0}", e.GetPosition (c));
			};
			
			r.MouseMove += delegate (object sender, MouseEventArgs e) {
				Console.WriteLine ("Motion Rect={0} Canvas={1}", e.GetPosition (r), e.GetPosition (c));
				//Console.WriteLine ("          Canvas {0}", e.GetPosition (c));
			};

			Brush enter_fill = new SolidColorBrush (Color.FromRgb (0, 255, 0));
			//Brush leave_fill = new SolidColorBrush (Color.FromRgb (0, 0, 255)); // r.Fill;
			Brush leave_fill = r.Fill;
			
			r.MouseEnter += delegate {
				Console.WriteLine ("Enter");
				r.Fill = enter_fill;
			};

			r.MouseLeave += delegate {
				Console.WriteLine ("Leave");
				r.Fill = leave_fill;
			};
			
			c.MouseLeftButtonDown += delegate {
				Console.WriteLine ("Canvas: Button Pressed!");
			};

			c.MouseLeftButtonUp += delegate {
				Console.WriteLine ("Canvas Button Released!");
			};

			r.MouseLeftButtonDown += delegate {
				Console.WriteLine ("Rectangle: Button Pressed!");
			};

			r.MouseLeftButtonUp += delegate {
				Console.WriteLine ("Rectangle Button Released!");
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
