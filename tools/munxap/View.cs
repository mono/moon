/*
 * View.cs.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

using GLib;
using Gtk;

using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Resources;
using System.Text;


abstract class View {
	public ZipContent Content { get; private set; }
	
	public View (ZipContent content)
	{
		Content = content;
	}
	public View ()
	{
	}
	
	public abstract Widget GetView ();
}

class ViewText : View {
	ScrolledWindow scrolled_window;
	TextView view;
	TextBuffer buffer;
	string text;
	
	public ViewText (ZipContent content) : base (content)
	{
	}
	
	public ViewText (string text) : base (null)
	{
		if (text == null)
			throw new ArgumentNullException ("text");
		this.text = text;
	}

	public override Widget GetView ()
	{
		if (scrolled_window != null)
			return scrolled_window;

		buffer = new TextBuffer (new TextTagTable ());
		buffer.Text = Text;
		view = new TextView (buffer);
		
		scrolled_window = new ScrolledWindow ();
		scrolled_window.Add (view);
		
		return scrolled_window;
	}

	public virtual string Text {
		get {
			if (text == null)
				text =  File.ReadAllText (Content.UnzippedFilename);
			return text;
		}
		set {
			text = value;
		}
	}
}

class ViewXaml : ViewText {
	public ViewXaml (ZipContent content) : base (content) {}
}

class ViewCode : ViewText {
	public ViewCode (string code) : base (code)	{ }
}

class ViewResource : View {
	VBox box;
	ListStore store;
	TreeView tree;
	ScrolledWindow scrolled_tree;
	
	string filename;
	
	public ViewResource (string filename) : base (null)
	{
		this.filename = filename;
	}

	public override Widget GetView ()
	{
		if (box != null)
			return box;

		// Create ui
		box = new VBox ();
		store = new ListStore (typeof (string), typeof (string), typeof (object));
		tree = new TreeView ();
		tree.Model = store;
		tree.AppendColumn ("Name", new CellRendererText (), "text", 0);
		tree.AppendColumn ("Type", new CellRendererText (), "text", 1);
		tree.HeadersVisible = true;
		tree.CursorChanged += HandleCursorChanged;
		scrolled_tree = new ScrolledWindow ();
		scrolled_tree.Add (tree);
		box.PackStart (scrolled_tree, true, true, 0);

		// Load resources
		using (ResourceReader reader = new ResourceReader (filename)) {
			foreach (DictionaryEntry obj in reader) {
				store.AppendValues (obj.Key.ToString (), obj.Value.GetType ().FullName, obj.Value);
			}
		}

		return box;
	}

	void HandleCursorChanged (object sender, EventArgs e)
	{
		TreeSelection selection = tree.Selection;
		TreeModel model;
		TreeIter iter;
		Value v;
		string name, type;
		object value;
		View view = null;
		
		if (!selection.GetSelected (out model, out iter))
			return;
		
		v = new Value ();
		store.GetValue (iter, 0, ref v);
		name = v.Val as string;
		v = new Value ();
		store.GetValue (iter, 1, ref v);
		type = v.Val as string;
		v = new Value ();
		store.GetValue (iter, 2, ref v);
		value = v.Val;

		// Create a view based on what we got
		
		switch (type) {
		case "System.IO.MemoryStream":
			MemoryStream stream = value as MemoryStream;
			if (name.EndsWith (".xaml")) {
				stream.Position = 0;
				using (MemoryStream clone = new MemoryStream (stream.ToArray ())) {
					using (StreamReader t = new StreamReader (clone)) {
						view = new ViewText (t.ReadToEnd ());
					}
				}
			} else {
				Console.WriteLine ("Don't know what to do with a {0} in a {1}", name, type);
			}
			break;
		case "System.String":
			if (value != null)
				view = new ViewText ((string) value);
			break;
		default:
			Console.WriteLine ("Unhandled case: {0}", type);
			break;
		}

		// Remove previous views and add the new one
		
		while (box.Children.Length > 1)
			box.Remove (box.Children [1]);
		
		if (view != null) {
			box.PackStart (view.GetView (), true, true, 0);
			box.ShowAll ();
		}
	}	
}

class ViewAssembly : View {
	HBox box;
	ListStore store;
	TreeView tree;
	ScrolledWindow scrolled_tree;
	
	public ViewAssembly (ZipContent content) : base (content) {}

	public override Widget GetView ()
	{
		string line;
		
		if (box != null)
			return box;

		// Create ui
		box = new HBox ();
		store = new ListStore (typeof (string), typeof (string), typeof (string));
		tree = new TreeView ();
		tree.Model = store;
		tree.AppendColumn ("Name", new CellRendererText (), "text", 0);
		tree.AppendColumn ("Type", new CellRendererText (), "text", 1);
		tree.AppendColumn ("Info", new CellRendererText (), "text", 2);
		tree.HeadersVisible = true;
		tree.CursorChanged += HandleCursorChanged;		
		scrolled_tree = new ScrolledWindow ();
		scrolled_tree.Add (tree);
		box.PackStart (scrolled_tree, true, true, 0);

		// Load resources
		// first get a list of resources in the assembly
		List<string> resources = new List<string> ();
		using (System.Diagnostics.Process p = new System.Diagnostics.Process ()) {
		 	p.StartInfo.FileName = "monodis";
			p.StartInfo.Arguments = Content.UnzippedFilename;
			p.StartInfo.RedirectStandardOutput = true;
			p.StartInfo.UseShellExecute = false;
			p.Start ();
			while ((line = p.StandardOutput.ReadLine ()) != null) {
				if (!line.StartsWith (".mresource"))
					continue;
				line = line.Replace (".mresource ", "").Replace ("public ", "");
				line = line.Trim (new char [] {' ', '\''});
				resources.Add (line);
			}
		}
		// write those resources to disk
		using (System.Diagnostics.Process p = new System.Diagnostics.Process ()) {
		 	p.StartInfo.FileName = "monodis";
			p.StartInfo.Arguments = "--mresources " + Content.UnzippedFilename;
			p.StartInfo.WorkingDirectory = Path.GetDirectoryName (Content.UnzippedFilename);
			p.Start ();
			p.WaitForExit ();
		}
		// add to treeview
		foreach (string res in resources) {
			string path = Path.Combine (Path.GetDirectoryName (Content.UnzippedFilename), res);
			Console.WriteLine ("Added: '{0}' '{1}' '{2}'", res, "Resource", path);
			store.AppendValues (res, "Resource", path);
		}
		
		// Load types
		using (System.Diagnostics.Process p = new System.Diagnostics.Process ()) {
		 	p.StartInfo.FileName = "monodis";
			p.StartInfo.Arguments = "--typedef " + Content.UnzippedFilename;
			p.StartInfo.RedirectStandardOutput = true;
			p.StartInfo.UseShellExecute = false;
			p.Start ();
			while ((line = p.StandardOutput.ReadLine ()) != null) {
				if (line.Trim () == string.Empty)
					continue;
				store.AppendValues (line, "Class", "");
			}
		}
		
		return box;
	}
	
	void HandleCursorChanged (object sender, EventArgs e)
	{
		TreeSelection selection = tree.Selection;
		TreeModel model;
		TreeIter iter;
		Value v;
		string type, info;
		View view = null;
		
		if (!selection.GetSelected (out model, out iter))
			return;

		// Create a view based on what we got
		
		v = new Value ();
		store.GetValue (iter, 2, ref v);
		info = v.Val as string;
		v = new Value ();
		store.GetValue (iter, 1, ref v);
		type = v.Val as string;

		switch (type) {
		case "Class":
			Console.WriteLine ("JB, We need your decompiler.");
			break;
		case "Resource":
			view = new ViewResource (info);
			break;
		default:
			Console.WriteLine ("Unhandled case: {0}", type);
			break;
		}

		// Remove old views and add the new one
		
		while (box.Children.Length > 1)
			box.Remove (box.Children [1]);
		
		if (view != null) {
			box.PackStart (view.GetView (), true, true, 0);
			box.ShowAll ();
		}
	}
}
