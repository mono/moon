//
// desklets.cs: basic functionality for moonlight desklets
//
// Authors:
//   Miguel de Icaza (miguel@novell.com)
//   Marek Habersack (mhabersack@novell.com)
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
using System.Windows;
using Gtk;

using Application = Gtk.Application;

namespace Moonlight.Gtk {

	/// <summary>
	///   Provides utility functions for Desklet developers
	/// </summary>
        /// <remarks>
	/// </remarks>
	public class Desklet {
		static bool _allElementsFound = true;

		/// <summary>
		///   Returns whether or not all the controls were loaded properly
		/// </summary>
		/// <remarks>
		///   Returns true if all the elements loaded with the <see cref="M:Mono.Desklets.Desklet.FindElement"/>
		///   method were found and of proper types. Otherwise returns false. Note that if you do not use the
		///   method, the return value of this property will be true.
		/// </remarks>
		/// <returns>
		///   True if all elements were found and of proper types, false otherwise.
		/// </returns>
		public static bool AllElementsFound {
			get { return _allElementsFound; }
		}
		
		/// <summary>
		///    Invokes the given EventHandler on the GUI thread.
		/// </summary>
		/// <param name="handler">The handler to invoke on the main thread</param>
		/// <remarks>
		///    This routine should be used when your application needs
		///    to invoke methods or access properties and events from Moonlight
		///    or Gtk# from a thread that is not the main thread. 
		/// </remarks>
		public static void Invoke (EventHandler handler)
		{
			Application.Invoke (handler);
		}

		/// <summary>
		///    Invokes the given EventHandler on the GUI thread.
		/// </summary>
		/// <param name="sender">The sender parameter to pass to the handler</param>
		/// <param name="args">Arguments to pass to the handler</param>
		/// <param name="handler">The handler to invoke on the main thread</param>
		/// <remarks>
		///    This routine should be used when your application needs
		///    to invoke methods or access properties and events from Moonlight
		///    or Gtk# from a thread that is not the main thread. 
		/// </remarks>
                public static void Invoke (object sender, EventArgs args, EventHandler handler)
		{
			Application.Invoke (sender, args, handler);
		}

		static void SetupOnClick (UIElement element, EventHandler handler)
		{
			bool down = false, inside = false;
			
			element.MouseLeftButtonDown += delegate {
				down = true;
			};

			element.MouseEnter += delegate {
				inside = true;
			};

			element.MouseLeave += delegate {
				inside = false;
			};

			element.MouseLeftButtonUp += delegate {
				if (inside && down){
					handler (element, EventArgs.Empty);
				}
				down = false;
			};
		}
		
		/// <summary>
		///   Configures various elements found on the given UIElement
		///   to become control points. 
		/// </summary>
		/// <param name="root">
		///    The UIElement that might contain the specially named objects to
		///    be hooked up.
		/// </param>
		/// <remarks>
		///   This looks up elements named "desklet-close" and "desklet-drag"
		///   elements from the given root element and hooks up mouse events
		///   so that they trigger a close action and a drag action on the
		///   desklet.
		/// </remarks>
		public static void SetupToolbox (FrameworkElement root)
		{
			UIElement close = root.FindName ("desklet-close") as UIElement;
			if (close != null){
				SetupOnClick (close, delegate {

					// When we move to multiple desklets on an
					// appdomain we will need to change this to
					// use some sort of global reference count and
					// only shut Gtk when the count reaches zero.
					Application.Quit ();
				});
			}

			UIElement drag = root.FindName ("desklet-drag") as UIElement;
			if (drag != null){
				//
				// TODO: we need to pass the Gtk.Window, sort out
				// a way of getting this information from mopen or
				// from the surface
				//
				SetupOnClick (drag, delegate {
					Console.WriteLine ("On drag not enabled, as we do not know our Gtk.Window yet");
				} );
			}
		}

		/// <summary>
		///   Find an element with the specified name, from the indicated root UI element and, optionally,
		///   check whether it is a descendant of the specified type.
		/// </summary>
		/// <param name="root">The FrameworkElement which may contain the named element</param>
		/// <param name="name">Name of the element to look for</param>
		/// <param name="type">If not null, specifies the type the element must be descendant from, in order
		/// for the lookup to succeed</param>
		/// <remarks>
		///   If all elements looked up with this method were found and matched the specified types, the
		///   <see cref="M:AllElementsFound"/> property will return true.
		/// </remarks>
		/// <returns>
		///   The element looked for or null, if not found or if its type doesn't match the one specified in the type
		///   parameter.
		/// </returns>
		public static object FindElement (FrameworkElement root, string name, Type type)
		{
			DependencyObject ret = root.FindName (name) as DependencyObject;
			if (ret == null)
				_allElementsFound = false;
			if (type != null && !type.IsInstanceOfType (ret))
				_allElementsFound = false;
			
			return ret;
		}
	}
}
