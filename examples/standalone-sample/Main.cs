//
// Main.cs 
//
// A sample standalone program showcasing the xaml embedding/class resolving
// capabilities.
//
// Authors:
//   Michael Dominic K. (mdk@mdk.am)
//
// Copyright 2008 Novell, Inc. (http://www.novell.com)
//
// See LICENSE file in the Moonlight distribution for licensing details

using System;
using System.IO;
using System.Net;
using System.Xml;
using Gtk;
using Gtk.Moonlight;

namespace StandaloneSample {

	public static class StandaloneSample {

		public static int Main (string [] args)
		{
			Application.Init ();
			GtkSilver.Init ();
		
			Window window = new Window ("My Canvas");
			window.DeleteEvent += DeleteEventHandler;
	
			GtkSilver silver = new GtkSilver (400, 400);
			silver.LoadFile ("MyCanvas.xaml");
			
			window.Add (silver);
			window.ShowAll ();
			Application.Run ();

			return 0;
		}

		public static void DeleteEventHandler (object o, DeleteEventArgs args)
		{
			Application.Quit ();
		}

	}

}
