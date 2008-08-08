/*
 * Helper.cs.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

using System;
using System.Text;
using System.Collections.Generic;
using System.IO;

public static class Helper {
	/// <summary>
	/// Generates a c style name from a c++ method
	/// - All capitalized letters are lowered
	/// - If the capital letter is not the first one, neither preceded by another capital letter,
	///   an underscore is prepended.
	/// - If the type name equals the member name (a C++ ctor), then 'new' is appended instead of the member name
	/// - If the type name starts with ~ (a C++ dtor), then 'free' is appended instead of the member name
	/// </summary>
	/// <param name="type">
	/// A <see cref="System.String"/>
	/// </param>
	/// <param name="member">
	/// A <see cref="System.String"/>
	/// </param>
	/// <returns>
	/// A <see cref="System.String"/>
	/// </returns>
	public static string CppToCName (string type, string member)
	{
		StringBuilder result = new StringBuilder (member.Length + (type != null ? type.Length : 0) + 5);
		
		if (!string.IsNullOrEmpty (type)) {
			CppToCName (type, result);
			result.Append ("_");
		}
		if (type == member) {
			result.Append ("new");
		} else if (member.StartsWith ("~")) {
			result.Append ("free");
		} else {
			CppToCName (member, result);
		}
		
		return result.ToString ();
	}
	
	/// <summary>
	/// Generates a c style name from a c++ name
	/// - All capitalized letters are lowered
	/// - If the capital letter is not the first one, neither preceded by another capital letter,
	///   an underscore is prepended.
	/// A few samples
	/// - DependencyObject -> dependency_object
	/// - UIElementCollection -> uielement_collection
	/// </summary>
	/// <param name="name">
	/// A <see cref="System.String"/>
	/// </param>
	/// <param name="result">
	/// A <see cref="StringBuilder"/>. The result is added to this parameter.
	/// </param>
	public static void CppToCName (string name, StringBuilder result)
	{
		for (int i = 0; i < name.Length; i++) {
			if (char.IsUpper (name [i])) {
				if (i > 0 && !char.IsUpper (name [i-1]))
					result.Append ("_");
				result.Append (char.ToLower (name [i]));
			} else {
				result.Append (name [i]);
			}
		}
	}
	
	public static void InitializeCurrentDirectory ()
	{
		string path = System.Reflection.Assembly.GetExecutingAssembly ().Location;

		while (!Directory.Exists (Path.Combine (path, "src")) && !Directory.Exists (Path.Combine (path, "plugin"))) {
			path = Path.GetDirectoryName (path);
			if (path == Path.GetDirectoryName (path))
			    throw new Exception ("Could not find the base directory of the moon module.");
		}

		Environment.CurrentDirectory = path;
	}
	public static void WriteAllText (string filename, string contents)
	{
		filename = filename.Replace ('/', Path.DirectorySeparatorChar).Replace ('\\', Path.DirectorySeparatorChar);
		if (File.ReadAllText (filename) != contents) {
			File.WriteAllText (filename, contents);
			Console.WriteLine ("Wrote {0}.", filename);
		} else {
			Console.WriteLine ("Skipped writing {0}, no changes.", filename);
		}
	}
	
	static int Main (string [] args)
	{
		Generator generator;
		
		try {
			foreach (string arg in args) {
				if (arg == "--log")
					Log.LogEnabled = true;
				else
					throw new Exception ("Invalid argument: " + arg);
			}
			InitializeCurrentDirectory ();
			generator = new Generator ();
			generator.Generate ();
			return 0;
		} catch (Exception ex) {
			Console.WriteLine (ex.ToString ());
			return 1;
		}
	}
}
