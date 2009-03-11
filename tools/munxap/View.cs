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
using Mono.Cecil;

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

class ViewImage : View {
	ScrolledWindow scrolled_window;
	Gdk.Pixbuf pixbuf;
	Gtk.Image image;

	public ViewImage (ZipContent content) : base (content)
	{
	}

	public ViewImage (System.IO.Stream stream) : base (null)
	{
		pixbuf = new Gdk.Pixbuf (stream);
	}

	public override Widget GetView ()
	{
		if (scrolled_window != null)
			return scrolled_window;

		image = new Gtk.Image (pixbuf);
		scrolled_window = new ScrolledWindow ();
		scrolled_window.AddWithViewport (image);

		return scrolled_window;
	}
}

class ViewCode : View {
	ListStore store;
	TreeView tree;
	ScrolledWindow scrolled_tree;
	MethodDefinition method;
	public ViewCode (MethodDefinition method)
	{
		this.method = method;
	}

	public override Widget GetView ()
	{
		if (scrolled_tree != null)
			return scrolled_tree;

		// Create ui
		store = new ListStore (typeof (string), typeof (string), typeof (string));
		tree = new TreeView ();
		tree.Model = store;
		tree.AppendColumn ("Offset", new CellRendererText (), "text", 0);
		tree.AppendColumn ("Instruction", new CellRendererText (), "text", 1);
		tree.AppendColumn ("Operand", new CellRendererText (), "text", 2);
		tree.HeadersVisible = true;
		scrolled_tree = new ScrolledWindow ();
		scrolled_tree.Add (tree);

		Mono.Cecil.Cil.MethodBody body = method.Body;
		Mono.Cecil.Cil.Instruction instr;
		if (body != null) {
			for (int i = 0; i < body.Instructions.Count; i++) {
				instr = body.Instructions [i];
				
				store.AppendValues ("IL_" + instr.Offset.ToString ("0000"), instr.OpCode.ToString (), FormatOperand (instr.Operand));
			}
		}
		return scrolled_tree;
	}

	string FormatOperand (object operand)
	{
		Mono.Cecil.Cil.Instruction instr;

		if (operand == null)
			return string.Empty;
		
		instr = operand as Mono.Cecil.Cil.Instruction;
		if (instr != null)
			return "IL_" + instr.Offset.ToString ("0000");

		return operand.ToString ();
	}
	
	
}

class ViewResource : View {
	VBox box;
	ListStore store;
	TreeView tree;
	ScrolledWindow scrolled_tree;
	
	Resource resource;
	
	public ViewResource (Resource resource) : base ()
	{
		this.resource = resource;
	}

	public override Widget GetView ()
	{
		if (box != null)
			return box;

		// Create ui
		box = new VBox (false, 10);
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
		EmbeddedResource embed = resource as EmbeddedResource;
		if (embed != null) {
			using (MemoryStream stream = new MemoryStream (embed.Data)) {
				using (ResourceReader reader = new ResourceReader (stream)) {
					foreach (DictionaryEntry obj in reader) {
						store.AppendValues (obj.Key.ToString (), obj.Value.GetType ().FullName, obj.Value);
					}
				}
			}
		} else {
			Console.WriteLine ("Don't know how to handle a resource whose type is {0}", resource.GetType ().FullName);
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
			} else if (name.EndsWith (".png") || name.EndsWith (".jpg")) { 
				using (MemoryStream clone = new MemoryStream (stream.ToArray ())) {
						view = new ViewImage (clone);
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
	TreeStore store;
	TreeView tree;
	ScrolledWindow scrolled_tree;
	AssemblyDefinition assembly;
	
	static Gdk.Pixbuf classPixbuf, delegatePixbuf, enumPixbuf;
	static Gdk.Pixbuf eventPixbuf, fieldPixbuf, interfacePixbuf;
	static Gdk.Pixbuf methodPixbuf, namespacePixbuf, propertyPixbuf;
	static Gdk.Pixbuf attributePixbuf, structPixbuf, assemblyPixbuf;

	static ViewAssembly ()
	{
		System.Reflection.Assembly ta = typeof (ViewAssembly).Assembly;
		classPixbuf = new Gdk.Pixbuf (ta, "c.gif");
		delegatePixbuf = new Gdk.Pixbuf (ta, "d.gif");
		enumPixbuf = new Gdk.Pixbuf (ta, "en.gif");
		eventPixbuf = new Gdk.Pixbuf (ta, "e.gif");
		fieldPixbuf = new Gdk.Pixbuf (ta, "f.gif");
		interfacePixbuf = new Gdk.Pixbuf (ta, "i.gif");
		methodPixbuf = new Gdk.Pixbuf (ta, "m.gif");
		namespacePixbuf = new Gdk.Pixbuf (ta, "n.gif");
		propertyPixbuf = new Gdk.Pixbuf (ta, "p.gif");
		attributePixbuf = new Gdk.Pixbuf (ta, "r.gif");
		structPixbuf = new Gdk.Pixbuf (ta, "s.gif");
		assemblyPixbuf = new Gdk.Pixbuf (ta, "y.gif");
	}
	
	public ViewAssembly (ZipContent content) : base (content) {}

	public override Widget GetView ()
	{
		Gtk.TreeIter iter;
		string line;
		
		if (box != null)
			return box;

		// Create ui
		box = new HBox ();
		store = new TreeStore (typeof (string), typeof (string), typeof (string), typeof (object), typeof (Gdk.Pixbuf));
		tree = new TreeView ();
		tree.Model = store;
		tree.AppendColumn ("Icon", new CellRendererPixbuf (), "pixbuf", 4);
		tree.AppendColumn ("Name", new CellRendererText (), "text", 0);
		tree.AppendColumn ("Type", new CellRendererText (), "text", 1);
		tree.AppendColumn ("Info", new CellRendererText (), "text", 2);
		tree.HeadersVisible = true;
		tree.CursorChanged += HandleCursorChanged;		
		scrolled_tree = new ScrolledWindow ();
		scrolled_tree.Add (tree);
		box.PackStart (scrolled_tree, true, true, 0);

		if (assembly == null)
			assembly = AssemblyFactory.GetAssembly (Content.UnzippedFilename);
				
		// Load resources
		// first get a list of resources in the assembly
		iter = store.AppendValues ("Resources");
		foreach (Mono.Cecil.Resource res in assembly.MainModule.Resources) {
			store.AppendValues (iter, res.Name, "Resource", "", res);
		}

		// Load types
		Gtk.TreeIter classes = store.AppendValues ("Classes");
		foreach (TypeDefinition type in assembly.MainModule.Types) {
			Gtk.TreeIter cl = store.AppendValues (classes, type.FullName, "Type", "", type, classPixbuf);
			List<MethodDefinition> shown_methods = new List<MethodDefinition> ();
			foreach (PropertyDefinition property in type.Properties) {
				Gtk.TreeIter prop = store.AppendValues (cl, property.Name, "Property", "", property, propertyPixbuf);
				if (property.GetMethod != null) {
					store.AppendValues (prop, FormatMethod (property.GetMethod), "Getter", "", property.GetMethod, methodPixbuf);
					shown_methods.Add (property.GetMethod);
				}
				if (property.SetMethod != null) {
					store.AppendValues (prop, FormatMethod (property.SetMethod), "Setter", "", property.SetMethod, methodPixbuf);
					shown_methods.Add (property.SetMethod);
				}
			}
			foreach (EventDefinition ev in type.Events) {
				Gtk.TreeIter evIter = store.AppendValues (cl, ev.Name, "Event", "", ev, eventPixbuf);
				if (ev.AddMethod != null) {
					store.AppendValues (evIter, FormatMethod (ev.AddMethod), "Adder", "", ev.AddMethod, methodPixbuf);
					shown_methods.Add (ev.AddMethod);
				}
				if (ev.RemoveMethod != null) {
					store.AppendValues (evIter, FormatMethod (ev.RemoveMethod), "Remover", "", ev.RemoveMethod, methodPixbuf);
					shown_methods.Add (ev.RemoveMethod);
				}
				if (ev.InvokeMethod != null) {
					store.AppendValues (evIter, FormatMethod (ev.InvokeMethod), "Invoker", "", ev.InvokeMethod, methodPixbuf);
					shown_methods.Add (ev.InvokeMethod);
				}
			}
			foreach (MethodDefinition method in type.Methods) {
				if (shown_methods.Contains (method))
					continue;
				store.AppendValues (cl, FormatMethod (method), "Method", "", method, methodPixbuf);
			}
			foreach (MethodDefinition method in type.Constructors) {
				store.AppendValues (cl, FormatMethod (method), "Constructor", "", method, methodPixbuf);
			}
			foreach (FieldDefinition field in type.Fields) {
				store.AppendValues (cl, field.FieldType.Name + " " + field.Name, "Field", "", field, fieldPixbuf);
			}
		}
		return box;
	}

	string FormatMethod (MethodDefinition method)
	{
		StringBuilder result = new StringBuilder ();

		result.Append (method.ReturnType.ReturnType.Name);
		result.Append (" ");
		result.Append (method.Name);
		result.Append ("(");

		for (int i = 0; i < method.Parameters.Count - 1; i++) {
			if (i > 0)
				result.Append (", ");
			result.Append (method.Parameters [i].ParameterType.Name);
			result.Append (" ");
			result.Append (method.Parameters [i].Name);
		}
		result.Append (")");

		return result.ToString ();
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
			Resource res;
			v = new Value ();
			store.GetValue (iter, 3, ref v);
			res = v.Val as Resource;
			if (res != null) {
				view = new ViewResource (res);
			} else {
				Console.WriteLine ("{0} didn't have a Resource, it had a {1}.", type, v.Val == null ? "null" : v.Val.GetType ().ToString ());
			}
			break;
		case "Method":
		case "Getter":
		case "Setter":
		case "Remover":
		case "Adder":
		case "Invoker":
		case "Constructor":
			MethodDefinition def;
			v = new Value ();
			store.GetValue (iter, 3, ref v);
			def = v.Val as MethodDefinition;
			if (def != null) {
				view = new ViewCode (def);
			} else {
				Console.WriteLine ("{0} didn't have a MethodDefinition.", type);
			}
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
