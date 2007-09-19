/*
 * Panel.cs: Moonlight Desklets Panel, about box.
 *
 * Author:
 *   Marek Habersack (mhabersack@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

using System;
using Gtk;
using Gdk;

public partial class AboutBox: Gtk.Window
{
	public AboutBox (): base (Gtk.WindowType.Toplevel)
	{
		Build ();
	}
}
