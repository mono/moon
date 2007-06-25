/*
 * Panel.cs: Moonlight Desklets Panel, main application window.
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
using Gtk;
using Gdk;

public partial class PanelWindow: Gtk.Window
{
	StatusIcon statusIcon;
	
	public PanelWindow (): base (Gtk.WindowType.Toplevel)
	{
		Build ();

		Pixbuf icon = Pixbuf.LoadFromResource ("mono_monkey_icon.png");
		statusIcon = new StatusIcon (icon);
		statusIcon = statusIcon;
		statusIcon.PopupMenu += new PopupMenuHandler (IconPopupHandler);
	}

	void IconPopupHandler (object o, PopupMenuArgs args)
	{
		Menu popup = new Menu ();
		
		MenuItem item = new MenuItem ("About");
		popup.Append (item);
		// broken
// 		popup.Popup (null, null, new MenuPositionFunc (StatusIcon.PositionMenu),
// 			     null, args.Event.Button, Gtk.Global.CurrentEventTime);
	}
	
	protected void OnDeleteEvent (object sender, DeleteEventArgs a)
	{
		Application.Quit ();
		a.RetVal = true;
	}

	protected virtual void OnCloseButtonClicked (object sender, System.EventArgs e)
	{
		// For now, just close app, when we haev trayicon we can just hide.
		// Hide ();
		Application.Quit ();
	}

	protected virtual void OnDeskletsButtonClicked (object sender, System.EventArgs e)
	{
		// Add here code to open a desklet page (on fucture).
	}

	protected virtual void OnHelpButtonClicked (object sender, System.EventArgs e)
	{
		// We will have some help?
	}
}