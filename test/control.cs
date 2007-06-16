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

public class MyControl : Control {
	public MyControl ()
	{
		InitializeFromXaml ("<Line X1='0' Y1='0' X2='100' Y2='100' Stroke='#80808080'/>");
	}
}

class ControlSample {

	static void Main ()
	{
		Application.Init ();

		Window w = new Window ("Top");
		GtkSilver silver = new GtkSilver (400, 400);
		w.Add (silver);
		w.ShowAll ();

		Canvas c = new Canvas ();
		silver.Attach (c);
		
		MyControl control = new MyControl ();
		c.Children.Add (control);

		Line l = new Line ();
		l.X1 = 100;
		l.X2 = 100;
		l.Y1 = 100;
		l.Y2 = 300;
		l.Stroke = new SolidColorBrush (Color.FromRgb (255, 255, 0));
		c.Children.Add (l);
		
		Application.Run ();
		
	}
}
