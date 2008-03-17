//
// MyCanvas.cs
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

using Gtk;
using Gtk.Moonlight;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Controls;
using System.Windows.Input;
using System;

namespace StandaloneSample {

	public partial class MyCanvas : Canvas {

		public MyCanvas ()
		{
			InitializeComponent ();
		}

	}

}
