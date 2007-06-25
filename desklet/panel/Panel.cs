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

namespace DeskletsPanel
{
	class MainClass
	{
		public static void Main (string[] args)
		{
			Application.Init ();
			PanelWindow win = new PanelWindow ();
			win.Show ();
			Application.Run ();
		}
	}
}