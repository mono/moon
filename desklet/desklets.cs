//
// desklets.cs: basic functionality for moonlight desklets
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
using System.Windows;
using Gtk;

namespace Mono.Desklets {

	/// <summary>
	///   Provides utility functions for Desklet developers
	/// </summary>
        /// <remarks>
	/// </remarks>
	public class Desklet {

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
		public static void SetupToolbox (UIElement root)
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
	}
}
