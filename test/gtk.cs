using System;
using System.Runtime.InteropServices;
using Gtk;
using System.Windows;
using System.Windows.Controls;
using Mono;
using System.Reflection;

namespace Gtk.Moonlight {
	
public class GtkSilver : DrawingArea {
	[DllImport ("moon")]
	extern static IntPtr surface_new (int w, int h);

	[DllImport ("moon")]
	extern static IntPtr surface_get_drawing_area (IntPtr surface);

	[DllImport ("moon")]
	extern static IntPtr xaml_create_from_file (string file, ref int kind_type);
	
	IntPtr surface;

	//
	// The downloader callbacks
	//
	internal delegate IntPtr downloader_create_state_func  (IntPtr native);
	internal delegate void   downloader_destroy_state_func (IntPtr state);
	internal delegate void   downloader_open_func  (string verb, string uri, bool async, IntPtr state);
	internal delegate void   downloader_send_func  (IntPtr state);
	internal delegate void   downloader_abort_func (IntPtr state);
	internal delegate string downloader_get_response_text_func (string part, IntPtr state);
	
	[DllImport ("moon")]
        internal extern static void downloader_set_functions (
		downloader_create_state_func create_state,
		downloader_destroy_state_func destroy_state,
		downloader_open_func open,
		downloader_send_func send,
		downloader_abort_func abort,
		downloader_get_response_text_func get_response);

	
	static GtkSilver ()
	{
		downloader_set_functions (
			ManagedDownloader.CreateDownloader,
			ManagedDownloader.DestroyDownloader,
			ManagedDownloader.Open,
			ManagedDownloader.Send,
			ManagedDownloader.Abort,
			ManagedDownloader.GetResponseText);
	}

	public GtkSilver (int w, int h)
	{
		surface = surface_new (w, h);
		Raw = surface_get_drawing_area (surface);
	}

	public void Attach (Canvas canvas)
	{
		if (canvas == null)
			throw new ArgumentNullException ("canvas");

		// Dynamically invoke our get-the-handle-code
		MethodInfo m = typeof (Canvas).Assembly.GetType ("Mono.Hosting").
			GetMethod ("SurfaceAttach", BindingFlags.Static | BindingFlags.NonPublic);
		m.Invoke (null, new object [] { surface, canvas });
	}

	public bool LoadFile (string file)
	{
		if (file == null)
			throw new ArgumentNullException ("file");
		int k = 1;
		
		IntPtr x = xaml_create_from_file (file, ref k);
		if (x == IntPtr.Zero)
			return false;

		// TODO: Check that x is a Canvas

		MethodInfo m = typeof (Canvas).GetMethod ("FromPtr", BindingFlags.Static | BindingFlags.NonPublic);
		Canvas c = (Canvas) m.Invoke (null, new object [] { x });
		if (c == null)
			return false;
		
		Attach (c);
		
		return true;
	}
}
}


