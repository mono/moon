//
// gtk.cs
//
// Author:
//   Miguel de Icaza (miguel@novell.com)
//
// Copyright 2007 Novell, Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

using System;
using System.Runtime.InteropServices;
using Gtk;
using System.Windows;
using System.Windows.Controls;
using Mono;
using System.Reflection;

namespace Gtk.Moonlight {
	
public class GtkSilver : EventBox {
	[DllImport ("moon")]
	extern static IntPtr surface_new (int w, int h);

	[DllImport ("moon")]
	extern static IntPtr surface_get_drawing_area (IntPtr surface);

	[DllImport ("moon")]
	extern static IntPtr xaml_create_from_file (string file, ref int kind_type);
	
	[DllImport ("moon")]
	extern static void surface_paint (IntPtr surface, IntPtr ctx, int x, int y, int width, int height);

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

	// This is a quick hack for f-spot code it will be cleaned up soon
	public void Print (IntPtr ctx, Gdk.Rectangle area)
	{
		surface_paint (surface, ctx, area.X, area.Y, area.Width, area.Height);
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


