/*
 * Panel.cs: Moonlight Desklets Panel.
 *
 * Author:
 *   Everaldo Canuto (ecanuto@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

using System;
using System.IO;
using Gtk;
using Gdk;

namespace DeskletsPanel
{
	class MainClass
	{
		static StatusIcon statusIcon;
		
		static StatusIcon SetUpStatusIcon ()
		{
			Pixbuf icon = Pixbuf.LoadFromResource ("mono_monkey_icon.png");
			return new StatusIcon (icon);
		}
		
		public static void Main (string[] args)
		{
			Application.Init ();
			PanelWindow win = new PanelWindow (SetUpStatusIcon ());
			win.Show ();
			Application.Run ();
		}
	}
}