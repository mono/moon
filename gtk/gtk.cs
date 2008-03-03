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
using Mono.Xaml;
using System.Reflection;

namespace Gtk.Moonlight {
	
	/// <summary>
	///    A Gtk# widget that can be used to embed Moonlight/Silverlight(tm)
	///    content in a Gtk application
	/// </summary>
	/// <remarks>
        ///    See the namespace documentation for a sample on how to use this
        ///    widget with your Gtk# code.
	/// </remarks>
public class GtkSilver : EventBox {
	IntPtr surface;

	//
	// The downloader callbacks
	//
	internal delegate IntPtr downloader_create_state_func  (IntPtr native);
	internal delegate void   downloader_destroy_state_func (IntPtr state);
	internal delegate void   downloader_open_func  (string verb, string uri, IntPtr state);
	internal delegate void   downloader_send_func  (IntPtr state);
	internal delegate void   downloader_abort_func (IntPtr state);
	
	[DllImport ("moon")]
        internal extern static void downloader_set_functions (
		downloader_create_state_func create_state,
		downloader_destroy_state_func destroy_state,
		downloader_open_func open,
		downloader_send_func send,
		downloader_abort_func abort);
	
	static GtkSilver ()
	{
		downloader_set_functions (
			ManagedDownloader.CreateDownloader,
			ManagedDownloader.DestroyDownloader,
			ManagedDownloader.Open,
			ManagedDownloader.Send,
			ManagedDownloader.Abort);

		// Just touch something in DependencyObject to trigger
		// its initialization
		object o = DependencyObject.NameProperty;
	}

	/// <summary>
	///    Initializes the GtkSilver widget, this must be called
        ///    before any calls are done to System.Windows.
	/// </summary>
	/// <remarks>
        ///    <para>The System.Windows namespace for Silverlight requires a
        ///    downloader engine to be registered before it can be used to
        ///    satisfy assembly dependencies and images.    If your application will
	///    for some reason call into System.Windows before they create an instance
	///    of GtkSilver, they should call this method to ensure that the proper
	///    downloader has been registered with the agclr runtime.</para>
	///
	///    <para>Failure to call this method typically result in errors from the XAML
	///    parsing code when it tries to resolve assembly references, external
	///    classes or loading of external media.</para>
	/// </remarks>
	static public void Init ()
	{
		// Just to execute the constructor.
	}
	
	/// <summary>
	///    Public constructor, creates a widget with the specified width and height
	/// </summary>
	/// <param name="width">The initial width for the widget</param>
	/// <param name="height">The initial height for the widget</param>
	/// <remarks>
	///    <para>The initial width and height of the GtkSilver control are given by the
	///    parameters.   The size of the widget can later be changed by using the
	///    standard Gtk# APIs (SizeAllocate).</para>
	///
	///    <para>The widget is initially empty, you must call the <see cref="Attach"/>
	///    method with a System.Windows.Controls.Canvas instance (you can create
	///    those programatically, using XAML, or using the <see cref="LoadFile(System.String)"/> method).</para>
	/// </remarks>
	public GtkSilver (int width, int height)
	{
		surface = NativeMethods.surface_new (width, height);
		Mono.Xaml.XamlLoader.SurfaceInDomain = surface;
		Raw = NativeMethods.surface_get_widget (surface);
	}

	/// <summary>
	///    The transparent state for the widget.   Used to drive the compositing of unpainted regions against the background.
	/// </summary>
	/// <remarks>
        ///    By default the value is false which will produce a solid white background,
	///    otherwise the background is cleared with black and composited with the
	///    background.
	/// </remarks>
	public bool Transparent {
		get {
			return NativeMethods.surface_get_trans (surface);
		}

		set {
			NativeMethods.surface_set_trans (surface, value);
		}
	}
	
	// This is a quick hack for f-spot code it will be cleaned up soon
	public void Print (IntPtr ctx, Gdk.Rectangle area)
	{
		NativeMethods.surface_paint (surface, ctx, area.X, area.Y, area.Width, area.Height);
	}

	/// <summary>
	///    Makes the specifies System.Windows.Control.Canvas the content to be displayed on this widget
	/// </summary>
	/// <param name="canvas">The System.Windows.Control.Canvas to attach.</param>
	/// <remarks>
	///    This will make the instance of canvas be the content displayed by the widget.
	///    Calling this method with a new canvas replaces the currently attached canvas
	///    with the new one.
	/// </remarks>
	public void Attach (Canvas canvas)
	{
		if (canvas == null)
			throw new ArgumentNullException ("canvas");

		// Dynamically invoke our get-the-handle-code
		MethodInfo m = typeof (Canvas).Assembly.GetType ("Mono.Hosting").
			GetMethod ("SurfaceAttach", BindingFlags.Static | BindingFlags.NonPublic);
		m.Invoke (null, new object [] { surface, canvas });
		c = canvas;
	}

	Canvas c;
	
	/// <summary>
	///    The currently attached Canvas.
	/// </summary>
	/// <remarks>
	///   This returns the instance of the currently attached Canvas.
	/// </remarks>
	public Canvas Canvas {
		get {
			return c;
		}
	}

	/// <summary>
	///    Resizes the surface to the specified size.
	/// </summary>
	/// <param name="width">The new width.</param>
	/// <param name="height">The new height.</param>
	public void Resize (int width, int height)
	{
		NativeMethods.surface_resize (surface, width, height);
	}

	/// <summary>
	///    Initializes the GtkSilver widget from the XAML contents in a string
	/// </summary>
	/// <param name="xaml">The contents of the string.</param>
	/// <param name="canvas">The created canvas, if the creation of the xaml string was successful.</param>
	/// <remarks>
	///   This uses the XAML parser to load the given string and display it on 
	///   the GtkSilver widget.
	/// </remarks>
	public bool LoadXaml (string xaml, out Canvas canvas)
	{
		canvas = null;

		if (xaml == null)
			throw new ArgumentNullException ("xaml");

		XamlLoader.AllowMultipleSurfacesPerDomain = true;

		XamlLoader loader = XamlLoader.CreateManagedXamlLoader (surface, IntPtr.Zero);
		object top = loader.CreateDependencyObjectFromString (xaml, true);

		if (top != null)
			return false;

		canvas = top as Canvas;
		if (canvas == null)
			return false;
	
		Attach (canvas);

		return true;
	}

	/// <summary>
	///    Initializes the GtkSilver widget from the XAML contents in a file
	/// </summary>
	/// <param name="file">The name of a file in your file system.</param>
	/// <param name="canvas">The created canvas, if the creation of the xaml string was successful.</param>
	/// <remarks>
	///   This uses the XAML parser to load the given file and display it on 
	///   the GtkSilver widget.
	/// </remarks>
	public bool LoadFile (string file, out Canvas canvas)
	{
		if (file == null)
			throw new ArgumentNullException ("file");

		return LoadXaml (System.IO.File.ReadAllText (file), out canvas);
	}

	/// <summary>
	///    Initializes the GtkSilver widget from the XAML contents in a file
	/// </summary>
	/// <param name="file">The name of a file in your file system.</param>
	/// <remarks>
	///   This uses the XAML parser to load the given file and display it on 
	///   the GtkSilver widget.
	/// </remarks>
	public bool LoadFile (string file)
	{
		Canvas canvas;
		return LoadFile (file, out canvas);
	}
}
}


