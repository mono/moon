// 
// MoonlightHost.cs
// 
// Author:
//   Aaron Bockover <abockover@novell.com>
// 
// Copyright 2009 Aaron Bockover
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
using System.Windows;
using Mono;
using Mono.Xaml;
using Gtk;

namespace Moonlight.Gtk
{
	/// <summary>
	///    A Gtk# widget that can be used to embed
	///    Moonlight/Silverlight(tm) content in a Gtk application
	/// </summary>
	/// <remarks>
	///    See the namespace documentation for a sample on how to
	///    use this widget with your Gtk# code.
	/// </remarks>
	public class MoonlightHost : EventBox 
	{
		private IntPtr surface;
		private IntPtr window;
		
		/// <summary>
		///    Public constructor
		/// </summary>
		/// <remarks>
		///    <para>The size of the internal root canvas is determined by 
		//     the size of the Surface widget, which can later be changed
		///    by using the standard Gtk# APIs (SizeAllocate).</para>
		///
		///    <para>The widget is initially empty, you must set the
		///    <see cref="Content"/> method with a
		///    System.Windows.FrameworkElement instance (you can
		///    create those programatically, or use 
		///    <see cref="LoadXaml(System.String)"/>,
		///    <see cref="LoadXamlFromFile(System.String)"/>, or
		///    <see cref="LoadXap(System.String)"/>).</para>
		/// </remarks>
		public MoonlightHost ()
		{
			Mono.Xaml.XamlLoader.AllowMultipleSurfacesPerDomain = true;
			window = NativeMethods.moon_window_gtk_new (false, 0, 0, IntPtr.Zero, IntPtr.Zero);
			surface = NativeMethods.surface_new (window);
			Raw = NativeMethods.moon_window_gtk_get_native_widget (window);

			SizeAllocated += OnSizeAllocated;
		}

		#region Public API

			/// <summary>
			///    Gets or sets the current top level FrameworkElement
			/// </summary>
			/// <remarks>
			///    When setting, this will make the instance of canvas be the content
			///    displayed by the widget. Using this setter with a new canvas replaces
			///    the currently set content with the new content.
			/// </remarks>
		public FrameworkElement Content {
			get { return (FrameworkElement)System.Windows.Application.Current.RootVisual; }
			set {
				Deployment.Current.InitializeDeployment (null, null);

				System.Windows.Application.Current.RootVisual = value;
			}
		}

		/// <summary>
		///    Helper property to get the current Application controlling the current Content
		/// </summary>
		/// <remarks>
		///    This property simply proxies the value of System.Windows.Application.Current
		/// </remarks>
		public System.Windows.Application Application {
			get { return System.Windows.Application.Current; }
		}

		/// <summary>
		///    The transparent state for the widget.  Used to drive
		///    the compositing of unpainted regions against the
		///    background.
		/// </summary>
		/// <remarks>
		///    By default the value is false which will produce a
		///    solid white background, otherwise the background is
		///    cleared with black and composited with the background.
		/// </remarks>
		public bool Transparent {
			get { return NativeMethods.moon_window_get_transparent (window); }
			set { NativeMethods.moon_window_set_transparent (window, value); }
		}

		/// <summary>
		///    Initializes the Surface widget from the XAML contents in a string
		/// </summary>
		/// <param name="xaml">The contents of the string.</param>
		/// <remarks>
		///   This uses the XAML parser to load the given string and
		///   display it on the Surface widget.
		/// </remarks>
		public void LoadXaml (string xaml)
		{
			if (xaml == null) {
				throw new ArgumentNullException ("xaml");
			}
            
			Deployment.Current.InitializeDeployment (null, null);
            
			DependencyObject toplevel = CreateElementFromString (xaml, true);
    
			if (toplevel == null) {
				throw new ArgumentException ("xaml");
			}
            
			Content = (FrameworkElement)toplevel;
		}

		/// <summary>
		///    Initializes the GtkSilver widget from the XAML contents in a file
		/// </summary>
		/// <param name="file">The name of a file in your file system.</param>
		/// <remarks>
		///   This uses the XAML parser to load the given file and
		///   display it on the Surface widget.
		/// </remarks>
		public void LoadXamlFromFile (string file)
		{
			if (file == null) {
				throw new ArgumentNullException ("file");
			}
            
			LoadXaml (System.IO.File.ReadAllText (file));
		}

		/// <summary>
		///    Initializes the Surface widget from a XAP file
		/// </summary>
		/// <param name="xapPath">Path to the XAP file</param>
		/// <remarks>
		///   This uses the XAP loader to load the given XAP
		///   display it on the Surface widget.
		/// </remarks>
		public void LoadXap (string xapPath)
		{
			if (xapPath == null) {
				throw new ArgumentNullException ("xapPath");
			}
            
			Deployment.Current.InitializeDeployment (IntPtr.Zero, xapPath, null, null);

			// we don't Attach() here because CreateFromXap has already done that for us.
		}

		/// <summary>
		///    Loads XAML within the context of the current GtkSilver widget
		/// </summary>
		/// <param name="xaml">The contents of the string.</param>
		/// <param name="createNamescope"></param>
		public DependencyObject CreateElementFromString (string xaml, bool createNamescope)
		{
			if (xaml == null) {
				throw new ArgumentNullException ("xaml");
			}
            
			XamlLoader.AllowMultipleSurfacesPerDomain = true;
            
			return (DependencyObject)XamlLoader
				.CreateManagedXamlLoader (null, surface, IntPtr.Zero)
				.CreateObjectFromString (xaml, true) as DependencyObject;
		}

		#endregion

			private void OnSizeAllocated (object o, SizeAllocatedArgs args)
		{
			NativeMethods.surface_resize (surface, Allocation.Width, Allocation.Height);
		}
	}
}
