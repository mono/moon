/*
 * munxap.cs.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

using NDesk.Options;
using GLib;
using Gtk;

using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Resources;
using System.Text;

public class Munxap
{
	Window window;
	VBox main;
	VBox left;
	HBox xap;
	Button close_button;
	Label file_label;
	TreeView xap_file_view;
	ListStore xap_file_store;
	ScrolledWindow xap_file_scrollable;
	Zip zip;
	
	public bool Help { get; private set; }
	public string File { get; private set; }
	
	static void ShowHelp (OptionSet o)
	{
		Console.WriteLine ("Usage is: munxap [args] file.xap\n" +
				   "Arguments are:");
		o.WriteOptionDescriptions (Console.Out);
	}
	
	static int Main (string [] args)
	{
		try {
			Munxap unxap = new Munxap ();
			
			var p = new OptionSet () {
				{ "h|?|help", v => unxap.Help = v != null },
			};

			List<string> rest = new List<string> ();
			try {
				rest = p.Parse (args);
			} catch (OptionException){
				Console.Error.WriteLine ("Try `munxap --help' for more information.");
				return 1;
			}

			if (unxap.Help) {
				ShowHelp (p);
				return 0;
			}
			
			if (rest.Count != 1) {
				ShowHelp (p);
				return 1;
			}

			unxap.File = rest [0];
			
			unxap.Show ();
			
			return 0;
		} catch (Exception ex) {
			Console.WriteLine (ex);
			return 1;
		}
	}

	void Show ()
	{
		zip = new Zip (File);
 
		Application.Init ();

		// Main window
		window = new Window ("munxap: " + File);
		window.Resize (600, 600);
		window.DeleteEvent += delegate (object obj, DeleteEventArgs args)
		{
			Close ();
		};

		// Main vbox
		main = new VBox (false, 10);
		window.Add (main);

		// label with the filename of the xap file on top
		file_label = new Label (File);
		main.PackStart (file_label, false, true, 0);

		// the middle consists of a hbox, leftmost column a list of files in the zip file
		xap = new HBox (false, 10);
		main.PackStart (xap, true, true, 0);

		left = new VBox (false, 10);
		xap.PackStart (left, true, true, 0);
		
		// a list of files in the zip file
		xap_file_store = new ListStore (typeof (String), typeof (String), typeof (ZipContent));		
		xap_file_view = new TreeView ();
		xap_file_view.Model = xap_file_store;
		xap_file_view.HeadersVisible = true;
		xap_file_view.AppendColumn ("Name", new CellRendererText (), "text", 0);
		xap_file_view.AppendColumn ("Type", new CellRendererText (), "text", 1);
		xap_file_view.CursorChanged += HandleCursorChanged;
		xap_file_scrollable = new ScrolledWindow ();
		xap_file_scrollable.Add (xap_file_view);
		left.PackStart (xap_file_scrollable, true, true, 0);
		
		// close button at the bottom
		close_button = new Button ("Close");
		close_button.Clicked += delegate (object obj, EventArgs args)
		{
			Close ();
		};
		main.PackEnd (close_button, false, true, 0);

		// Load zip contents
		foreach (ZipContent f in zip.Files) {
			xap_file_store.AppendValues (f.Filename, f.Type, f);
		}
		
		
		window.ShowAll ();
		
		Application.Run ();
	}

	void Close ()
	{
		zip.Dispose ();
		Application.Quit ();
	}
 	
	void HandleCursorChanged (object sender, EventArgs e)
	{
		TreeSelection selection = xap_file_view.Selection;
		TreeModel model;
		TreeIter iter;
		Value v;
		ZipContent content;
		View view = null;

		// Find the selected column, and create a view for it
		
		if (!selection.GetSelected (out model, out iter))
			return;
		
		v = new Value ();
		xap_file_store.GetValue (iter, 2, ref v);
		content = v.Val as ZipContent;

		if (content == null) {
			Console.WriteLine ("No content here.");
			return;
		}
		
		switch (content.Type) {
		case ".xaml":
		case ".xml":
			view = new ViewXaml (content);
			break;
		case ".dll":
			view = new ViewAssembly (content);
			break;
		case ".mdb":
		case ".pdb":
			break; // Ignore
		default:
			Console.WriteLine ("Unknown type: {0}");
			break;
		}

		// Remove any previous views
		while (left.Children.Length > 1)
			left.Remove (left.Children [1]);

		// Add the current one, if any
		if (view != null) {
			left.PackStart (view.GetView (), true, true, 0);
			left.ShowAll ();
		}
	}
}
