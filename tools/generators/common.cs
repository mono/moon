/*
 * common.cs: General routines for code generators
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
using Generation;
using System.Collections.Generic;
using System.IO;

namespace Generation {
	public static class Helper {
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
	
	public static class Log {
		public static bool LogEnabled;
		
		public static void Write (string text, params object [] args)
		{
			if (LogEnabled)
				Console.Write (text, args);
		}
		public static void WriteLine (string text, params object [] args)
		{
			Write (text + Environment.NewLine, args);
		}
		public static void WriteLine ()
		{
			WriteLine ("");
		}
	}

	namespace C {
		
		public class Token {
			bool is_identifier;
			public string value;
			
			public bool IsIdentifier {
				get { return is_identifier; }
			}
			
			public bool IsPunctuation {
				get { return !is_identifier; }
			}
			
			public Token (string value, bool is_identifier) 
			{
				this.value = value;
				this.is_identifier = is_identifier;
			}
			
			public static bool operator == (Token a, Token b)
			{
				return a.value == b.value;
			}
			public static bool operator != (Token a, Token b)
			{
				return !(a == b);
			}
			public static bool operator == (Token a, string b)
			{
				return a.value == b;
			}
			public static bool operator != (Token a, string b)
			{
				return !(a == b);
			}
			public static bool operator == (string a, Token b)
			{
				return a == b.value;
			}
			public static bool operator != (string a, Token b)
			{
				return !(a == b);
			}
			
			public override bool Equals (object obj)
			{
				if (obj == null)
					return false;
				if (obj is string)
					return (string) obj == value;
				if (obj is Token)
					return (Token) obj == this;
				return false;
			}
			
			public override int GetHashCode ()
			{
				return value.GetHashCode ();
			}
		}
		
		public class Tokens : Queue <Token>
		{
			public void Tokenize (string code)
			{
				StringBuilder current = new StringBuilder ();
				
				foreach (char c in code) {
					if (!Parser.IsIdentifier (c) && current.Length > 0) {
						Enqueue (new Token (current.ToString (), true));
						current.Length = 0;
					}
					
					if (Parser.IsIdentifier (c)) {
						current.Append (c);
					} else if (Parser.IsWhiteSpace (c)) {
						// do nothing
					} else if (Parser.IsPunctuation (c)) {
						Enqueue (new Token (c.ToString (), false));
					} else {
						throw new ArgumentException (string.Format ("Unknown character: '{0}' (#{1})", c, (int) c));
					}
				}
				if (current.Length > 0)
					Enqueue (new Token (current.ToString (), true));
			}
			
			public void Dump ()
			{
				Log.Write ("Tokens: ");
				foreach (Token token in this)
					Log.Write ("'{0}' ", token.value);
			}
		}
		
		public enum TypeDefinitionType
		{
			Native,
			Managed,
			PInvoke
		}
		
		public class ManagedTypeDefinition
		{
			public bool is_ref;
			public bool is_out;
			public string type;
			public ManagedTypeDefinition (string type)
			{
				this.type = type;
			}
			public ManagedTypeDefinition (string type, bool is_ref, bool is_out)
			{
				this.type = type;
				this.is_ref = is_ref;
				this.is_out = is_out;
			}
		}
		
		public class TypeDefinition
		{
			private bool is_const;
			private string native;
			private string pinvoke;
					
			public string Native {
				get { return native; }
				set { native = value; }
			}
			
			public ManagedTypeDefinition Managed {
				get { return GetManagedType (native); }
			}
			
			public string PInvoke {
				get { return pinvoke; }
				set { pinvoke = value; }
			}
			
			public bool IsConst {
				get { return is_const; }
				set { is_const = value; }
			}
			
			public bool IsKnownType {
				get { return !Managed.type.Contains ("/* Unknown */"); }
			}
			
			public void Write (StringBuilder text, TypeDefinitionType which)
			{
				switch (which) {
				case TypeDefinitionType.Managed:
					ManagedTypeDefinition managed = Managed;
					if (managed.is_out)
						text.Append ("out ");
					if (managed.is_ref)
						text.Append ("ref ");
					text.Append (managed.type);
					break;
				case TypeDefinitionType.Native:
					if (is_const)
						text.Append ("const ");
					text.Append (native); break;
				case TypeDefinitionType.PInvoke:
					if (string.IsNullOrEmpty (pinvoke))
						Write (text, TypeDefinitionType.Managed);
					else
						text.Append (pinvoke);
					break;
				default:
					throw new ArgumentOutOfRangeException ("which");
				}
			}
			
			public static ManagedTypeDefinition GetManagedType (string native)
			{
				if (string.IsNullOrEmpty (native))
					throw new ArgumentNullException ();
				
				switch (native) {
				case "char*":
					return new ManagedTypeDefinition ("string");
				case "bool":
				case "int":
					return new ManagedTypeDefinition (native);
				case "Type::Kind":
					return new ManagedTypeDefinition ("Kind");
				case "MoonError*":
					return new ManagedTypeDefinition ("MoonError", false, true);
				case "Value*":
				case "DependencyObject **o":
				case "DependencyProperty*":
				case "DependencyObject*":
				case "Surface*":
					return new ManagedTypeDefinition ("IntPtr");
				case "NativePropertyChangedHandler*":
					return new ManagedTypeDefinition ("Mono.NativePropertyChangedHandler");
				default:
					return new ManagedTypeDefinition (string.Format ("/* Unknown */ IntPtr", native));
				}
			}
		}
		
		public class ParameterDefinition
		{
			public MethodDefinition method;
			public string name;
			public TypeDefinition type;
			public bool not_managed;
			public bool disabled_once;
			public string managed_wrapper_code; // The code to put into the call to the call to the real pinvoke, defaults to the name of the parameter (if this field is null)
			
			public ParameterDefinition (MethodDefinition method, string name, TypeDefinition type)
			{
				this.method = method;
				this.name = name;
				this.type = type;
			}
			
			public void Write (StringBuilder text, TypeDefinitionType which)
			{
				type.Write (text, which);
				text.Append (" ");
				text.Append (name);
			}
		}
		
		public class ParameterDefinitions : List <ParameterDefinition>
		{
			public bool ContainsUnknownTypes {
				get {
					foreach (ParameterDefinition parameter in this)
						if (!parameter.type.IsKnownType)
							return true;
					return false;
				}
			}
		}

		public delegate void WrapperGenerator (StringBuilder text, string tabs);

		public class MethodDefinition
		{
			public string header;
			public object generator;
			public string signature; // The entire native signature.
			public string name;
			public string declaringtype;
			public bool is_static;
			public bool is_virtual;
			public TypeDefinition returntype = new TypeDefinition ();
			public ParameterDefinitions parameters = new ParameterDefinitions ();
			public Dictionary <string, object> properties = new Dictionary<string,object> ();
			
			public MethodDefinition (object generator, string signature, string header)
			{
				this.generator = generator;
				this.signature = signature;
				Parse ();
			}
			
			public void Dump ()
			{
				Log.Write ("Method declaringtype: '{0}' name='{1}', is_static: {2}, is_virtual: {3}, returntype: '{3}'",
				           declaringtype, name, is_static, is_virtual, returntype.Native);
				foreach (KeyValuePair <string, object> pair in properties) {
					Log.Write (" '{0}' = '{1}' ", pair.Key, pair.Value);
				}
				Log.WriteLine ();
			}
			
			public bool ContainsUnknownTypes {
				get {
					if (!returntype.IsKnownType)
						return true;
					if (parameters.ContainsUnknownTypes)
						return true;
					return false;
				}
			}
			
			public WrapperGenerator GetMarshaller ()
			{
				if (returntype.Managed.type == "string") {
					returntype.PInvoke = "IntPtr";
					return StringReturnTypeMarshaller;
				}
				
				if (parameters.Count > 0 && parameters [parameters.Count - 1].type.Native == "MoonError*") {
					parameters [parameters.Count - 1].not_managed = true;
					return MoonErrorMarshaller;
				}
				
				return null;
			}
			
			void Parse ()
			{
				string parameter_name;
				TypeDefinition parameter_type;
				Tokens tokens = new Tokens ();
				
				try {
					if (signature.Contains ("{"))
						signature = signature.Substring (0, signature.IndexOf ("{")) + ";";
					
					tokens.Tokenize (signature);
				
					if (tokens.Peek () == "static") {
						tokens.Dequeue ();
						is_static = true;
					} else if (tokens.Peek () == "virtual") {
						tokens.Dequeue ();
						is_virtual = true;
					}
					
					returntype = ReadType (tokens);
					
					if (!tokens.Peek ().IsIdentifier)
						throw new ArgumentException ("Expected an identifier after the type.");
					
					name = tokens.Dequeue ().value;
					
					//Log.WriteLine ("Parsed method name: '{0}' and return type: '{1}' (managed: '{2}') ", name, returntype.Native, returntype.Managed);
					
					if (tokens.Dequeue () != "(")
						throw new ArgumentException ("Expected '(')");
					
					if (tokens.Peek () != ")") {
						do {
							parameter_type = ReadType (tokens);
							if (!tokens.Peek ().IsIdentifier)
							    throw new ArgumentException ("Expected identifier");
							parameter_name = tokens.Dequeue ().value;
							
							//Log.WriteLine ("Parsed name: '{0}' type: '{1}'", parameter_name, parameter_type);
							
							parameters.Add (new ParameterDefinition (this, parameter_name, parameter_type));
							
							if (tokens.Peek () != ",") {
								break;
							} else {
								tokens.Dequeue ();
								continue;
							}
						} while (true);
					}
					
					if (tokens.Dequeue () != ")")
						throw new ArgumentException ("Expected ')'");
			
					if (tokens.Count == 0) {
						// we're done, the code is code starting on the next line
					} else if (tokens.Peek () == "{") {
						// we're done, the rest is code
					} else if (tokens.Dequeue () != ";")
						throw new ArgumentException ("Expected ';'");
					
				} catch (Exception ex) {
					Console.WriteLine ("MethodDefinition: parsing signature: '{0}'", signature);
					Console.WriteLine ("Exception: " + ex.Message);
					tokens.Dump ();
					Console.WriteLine ("");
					Console.WriteLine (ex);
					throw;
				}
			}
			
			public static TypeDefinition ReadType (Tokens tokens)
			{
				TypeDefinition type = new TypeDefinition ();
				string result = "";
				
				if (tokens.Peek () == "const") {
					tokens.Dequeue ();
					type.IsConst = true;
				}
		
				if (!tokens.Peek ().IsIdentifier)
					throw new ArgumentException (string.Format ("ReadType ({0}): Expected first character of a type to be an identifier.", tokens.Peek ()));
				
				result += tokens.Dequeue ().value;
				if (tokens.Peek () == ":") {
					result += tokens.Dequeue ().value;
					if (!(tokens.Peek () == ":"))
						throw new ArgumentException (string.Format ("ReadType ({0}): Expected another : after a first :.", tokens.Peek ()));
					result += tokens.Dequeue ().value;
					if (!tokens.Peek ().IsIdentifier)
						throw new ArgumentException (string.Format ("ReadTyep ({0}): Expected identifier after ::.", tokens.Peek ()));
					result += tokens.Dequeue ().value;
				}
				while (tokens.Peek () == "*")
					result += tokens.Dequeue ().value;
				
				type.Native = result;
				return type;
			}
				
			
			public void WriteParameters (StringBuilder text, TypeDefinitionType which)
			{
				bool first_done = false;
				text.Append (" (");
				foreach (ParameterDefinition parameter in parameters) {
					if (parameter.disabled_once) {
						parameter.disabled_once = false;
						continue;
					}
					if (parameter.not_managed && which == TypeDefinitionType.Managed)
						continue;
					
					if (first_done)
						text.Append (", ");
					parameter.Write (text, which);
					first_done = true;
				}
				text.Append (")");
			}
			
			public void WriteWrapperCall (StringBuilder text, TypeDefinitionType which)
			{
				bool first_done = false;
				ManagedTypeDefinition type;
				
				text.Append (" (");
				foreach (ParameterDefinition parameter in parameters) {
					if (parameter.disabled_once) {
						parameter.disabled_once = false;
						continue;
					}
					
					if (first_done)
						text.Append (", ");
					type = parameter.type.Managed;
					if (parameter.managed_wrapper_code != null) {
						text.Append (parameter.managed_wrapper_code);
					} else {
						if (which == TypeDefinitionType.Managed) {
							if (type.is_out)
								text.Append ("out ");
							if (type.is_ref)
								text.Append ("ref ");
						}
						text.Append (parameter.name);
					}
					first_done = true;
				}
				text.Append (")");
			}
			
			public void StringReturnTypeMarshaller (StringBuilder text, string tabs)
			{
				text.Append (tabs);
				text.Append ("\tIntPtr p = ");
				text.Append (name);
				text.Append ("_ ");
				WriteWrapperCall (text, TypeDefinitionType.Managed);
				text.AppendLine (";");
				
				text.Append (tabs);
				text.AppendLine ("\treturn p == IntPtr.Zero ? null : Marshal.PtrToStringAnsi (p);");
			}
			
			public void MoonErrorMarshaller (StringBuilder text, string tabs)
			{
				bool is_void = returntype.Native == "void";
				text.Append (tabs);
				text.AppendLine ("\tMoonError error;");
				
				text.Append (tabs);
				text.Append ("\t");
				if (!is_void) {
					returntype.Write (text, TypeDefinitionType.Managed);
					text.Append (" result = ");
				}
				text.Append (name);
				text.Append ("_");
				WriteWrapperCall (text, TypeDefinitionType.Managed);
				text.AppendLine (";");
				
				text.Append (tabs);
				text.AppendLine ("\tif (error.Number != 0)");
				
				text.Append (tabs);
				text.AppendLine ("\t\tthrow CreateManagedException (error);");
				
				if (!is_void) {
					text.Append (tabs);
					text.AppendLine ("\treturn result;");
				}
			}
		}
			
		public static class Parser
		{
			public static bool IsIdentifier(string str)
			{
				return IsIdentifier (str [0]);
			}
			
			public static bool IsIdentifier (char c)
			{
				return char.IsLetterOrDigit (c) || c == '_';
			}
			
			public static bool IsPunctuation (char c)
			{
				return char.IsPunctuation (c);
			}
			
			public static bool IsWhiteSpace (char c)
			{
				return char.IsWhiteSpace (c);
			}
		}

	}
}