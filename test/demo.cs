using System;
using System.Runtime.InteropServices;
using Gtk;

class X {
	[DllImport ("moon")]
	extern static IntPtr surface_new (int w, int h);

	[DllImport ("moon")]
	extern static IntPtr surface_get_drawing_area (IntPtr surface);
	
	static void Main ()
	{
		Application.Init ();

		IntPtr surface = surface_new (400, 400);
		
		Window w = new Window ("Top");
		DrawingArea da = new DrawingArea (surface_get_drawing_area (surface));

		w.Add (da);
		w.ShowAll ();

		Application.Run ();
	}
}
