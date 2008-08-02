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
	}
	
	public static class Log {
		
		public static void Write (string text, params object [] args)
		{
			//Console.Write (text, args);
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
		
		public class TypeDefinition
		{
			private bool is_const;
			private string native;
			private string pinvoke;
					
			public string Native {
				get { return native; }
				set { native = value; }
			}
			
			public string Managed {
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
				get { return !Managed.Contains ("/* Unknown */"); }
			}
			
			public void Write (StringBuilder text, TypeDefinitionType which)
			{
				switch (which) {
				case TypeDefinitionType.Managed:
					text.Append (Managed); break;
				case TypeDefinitionType.Native:
					text.Append (native); break;
				case TypeDefinitionType.PInvoke:
					text.Append (string.IsNullOrEmpty (pinvoke) ? Managed : pinvoke); break;
				default:
					throw new ArgumentOutOfRangeException ("which");
				}
			}
			
			public static string GetManagedType (string native)
			{
				if (string.IsNullOrEmpty (native))
					throw new ArgumentNullException ();
				
				switch (native) {
				case "char*":
					return "string";
				case "bool":
				case "int":
					return native;
				case "Type::Kind":
					return "Kind";
				case "DependencyObject **o":
				case "DependencyProperty*":
				case "DependencyObject*":
				case "Surface*":
					return "IntPtr";
				case "NativePropertyChangedHandler*":
					return "Mono.NativePropertyChangedHandler";
				default:
					return string.Format ("/* Unknown */ IntPtr", native);
				}
			}
		}
		
		public class ParameterDefinition
		{
			public MethodDefinition method;
			public string name;
			public TypeDefinition type;
		
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
			public TypeDefinition returntype = new TypeDefinition ();
			public ParameterDefinitions parameters = new ParameterDefinitions ();
			public Dictionary <string, object> properties = new Dictionary<string,object> ();
			
			public MethodDefinition (object generator, string signature, string header)
			{
				this.generator = generator;
				this.signature = signature;
				Parse ();
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
				if (returntype.Managed == "string") {
					returntype.PInvoke = "IntPtr";
					return StringReturnTypeMarshaller;
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
					}
					
					returntype = ReadType (tokens);
					
					if (!tokens.Peek ().IsIdentifier)
						throw new ArgumentException ("Expected an identifier after the type.");
					
					name = tokens.Dequeue ().value;
					
					Log.WriteLine ("Parsed method name: '{0}' and return type: '{1}' (managed: '{2}')", name, returntype.Native, returntype.Managed);
					
					if (tokens.Dequeue () != "(")
						throw new ArgumentException ("Expected '(')");
					
					if (tokens.Peek () != ")") {
						do {
							parameter_type = ReadType (tokens);
							if (!tokens.Peek ().IsIdentifier)
							    throw new ArgumentException ("Expected identifier");
							parameter_name = tokens.Dequeue ().value;
							
							Log.WriteLine ("Parsed name: '{0}' type: '{1}'", parameter_name, parameter_type);
							
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
			
					if (tokens.Peek () == "{") {
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
					if (first_done)
						text.Append (", ");
					parameter.Write (text, which);
					first_done = true;
				}
				text.Append (")");
			}
			
			public void WriteWrapperCall (StringBuilder text)
			{
				WriteWrapperCall (text, false);
			}
			public void WriteWrapperCall (StringBuilder text, bool skip_first)
			{
				bool first_done = false;
				bool first_skipped = !skip_first;
				text.Append (" (");
				foreach (ParameterDefinition parameter in parameters) {
					if (!first_skipped) {
						first_skipped = true;
						continue;
					}
					if (first_done)
						text.Append (", ");
					text.Append (parameter.name);
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
				WriteWrapperCall (text);
				text.AppendLine (";");
				
				text.Append (tabs);
				text.AppendLine ("\treturn p == IntPtr.Zero ? null : Marshal.PtrToStringAnsi (p);");
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