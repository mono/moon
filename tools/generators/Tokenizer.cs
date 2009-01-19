/*
 * Tokenizer.cs.
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

public enum Token2Type {
	Identifier,
	Literal,
	Punctuation,
	CommentProperty
}

public class Token2 {
	public Token2Type type;
	public string value;
	
	public Token2 (Token2Type type, string value)
	{
		//Console.WriteLine ("Token2: {0}, '{1}'", type, value);
		this.type = type;
		this.value = value;
	}
	public Token2 (Token2Type type, char value)
	{
		//Console.WriteLine ("Token2: {0}, '{1}'", type, value);
		this.type = type;
		this.value = value.ToString ();
	}
	
	public override string ToString ()
	{
		return string.Format ("{0} '{1}'", type, value);
	}

}

public class Tokenizer {
	private Queue <string> files;
	private StreamReader current_stream;
	private Token2 current_token;
	private string current_file;
	private List <char> chars = new List <char> ();
	private int current_line;
	
	public int CurrentLine {
		get { return current_line; }
	}
	
	public Token2 CurrentToken {
		get { return current_token; }
	}
	
	public string CurrentFile {
		get { return current_file; }
	}
	
	public Tokenizer (string [] files)
	{
		this.files = new Queue <string> (files);
	}
	
	public char ReadNextChar ()
	{
		StringBuilder line = new StringBuilder ();
		
		do {
			line.Length = 0;
			
			while (current_stream == null || current_stream.EndOfStream) {
				if (current_stream != null) {
					current_stream.Close ();
					current_stream = null;
				}
				if (files.Count == 0)
					return char.MinValue;;
				current_file = files.Dequeue ();
				current_stream = new StreamReader (current_file);
				current_line = 0;
				//Console.WriteLine ("Parsing {0}...", current_file);
			}
			
			do {
				line.Append (current_stream.ReadLine ());
				current_line++;
				
				if (line == null || line.Length == 0)
					break;
				
				if (line [line.Length - 1] != '\\')
					break;
				
				line.Length--;
			} while (true);
			
			//Console.WriteLine ("ReadNextChar: Read line: '{0}'", line);
			
			if (line.Length == 0) {
				//Console.WriteLine ("ReadNextChar: Empty line");
				continue;
			}
			
			if (line [0] == '#') {
				//Console.WriteLine ("ReadNextChar: Skipped preprocessor line: '{0}'", line);
				continue;
			}
			break;
		} while (true);
		
		for (int i = 0; i < line.Length; i++)
			chars.Add (line [i]);
		chars.Add ('\n');
		return '\n';
	}
	
	public char GetNextChar ()
	{
		char result;
		
		if (chars.Count != 0) {
			result = chars [0];
			chars.RemoveAt (0);
			//Console.WriteLine ("GetNextChar (): popped '{0}'", result);
			return result;
		}
		
		result = ReadNextChar ();
		//Console.WriteLine ("GetNextChar (): read '{0}'", result);
		
		// All newlines are passed as only one '\n' to the rest of the code
		if (result == '\n') {
			if (PeekChar (1) == '\r')
				ReadNextChar ();
		} else if (result == '\r') {
			if (PeekChar (1) == '\n')
				result = ReadNextChar ();
			else
				result = '\n';
		}
		return result;
	}
	
	public void PutBackChar (char v)
	{
		chars.Add (v);
	}
	
	public char PeekChar (int positions)
	{
		while (chars.Count < positions) {
			char c = ReadNextChar ();
			if (c == char.MinValue)
				return c;
			PutBackChar (c);
		}
	
		//Console.WriteLine ("PeekChar ({0}): peeked '{1}'", positions, chars [positions - 1]);
		return chars [positions - 1];
	}
	
	public bool Advance (bool throw_on_end)
	{
		bool result;
		try {
			do {
				result = AdvanceInternal (throw_on_end);
				if (CurrentToken.value == "G_BEGIN_DECLS") {
					continue;
				} else if (CurrentToken.value == "G_END_DECLS") {
					continue;
				}
				break;
			} while (true);
			return result;
		} catch (Exception ex) {
			throw new Exception (string.Format ("{0}({1}): {2}", current_file, current_line, ex.Message), ex);
		}
	}
	
	private bool AdvanceInternal (bool throw_on_end)
	{
		char current;
		StringBuilder builder = new StringBuilder ();
		
	 startagain:
		
		builder.Length = 0;
		
		do {
			current = GetNextChar ();
		} while (IsWhiteSpace (current));
		
		if (current == '/') {
			current = GetNextChar ();
			if (current == '*') { // Found a comment
				// Skip any whitespace
				do {
					current = GetNextChar ();
				} while (IsWhiteSpace (current));
				
				// Check for a comment property
				if (current == '@') {
					current = GetNextChar ();
					while (true) {
						if (current == char.MinValue)
							throw new Exception ("Unexpected end of code.");
						if (current == '*' && PeekChar (1) == '/')
							break;
						builder.Append (current);
						current = GetNextChar ();
					}
					while (builder.Length > 0 && builder [builder.Length - 1] == ' ')
						builder.Length--;
					
					if (builder.Length == 0)
						throw new Exception ("Empty comment property.");
				}
				
				while (true) {
					if (current == char.MinValue)
						throw new Exception ("Unexpected end of code.");
					if (current == '*' && PeekChar (1) == '/')
						break;
				
					current = GetNextChar ();
				}
				
				if (current != '*')
					throw new Exception (string.Format ("Expected '*', got '{0}'", current));
				
				current = GetNextChar ();
				if (current != '/')
					throw new Exception (string.Format ("Expected '/', got '{0}'", current));
				
				if (builder.Length != 0) {
					current_token = new Token2 (Token2Type.CommentProperty, builder.ToString ());
					return true;
				}
				// We've skipped the comment, start again
				goto startagain;
			} else if (current == '/') { // Found a comment
				do {
					current = GetNextChar ();
				} while (current != '\r' && current != '\n' && current != char.MinValue);
				if (current == '\r') {
				} else if (current == '\n') {
				} else {
					throw new Exception ("Expected end of line.");
				}
				// We've skipped the comment, start again
				goto startagain;
			} else {
				PutBackChar (current);
				current_token = new Token2 (Token2Type.Punctuation, "/");
				return true;
			}
		}
		
		if (IsPunctuation (current)) {
			current_token = new Token2 (Token2Type.Punctuation, current);
			return true;
		}

		if (current == '\'') {
			current_token = new Token2 (Token2Type.Punctuation, current);
			return true;
		}
		
		if (current == '"') {
			do {
				current = GetNextChar ();
				if (current == '\\') {
					current = GetNextChar ();
					builder.Append (current); // We don't care much about special characters like \n, \t, etc.
				} else if (current != '"') {
					builder.Append (current);
				} else if (current == char.MinValue) {
					throw new Exception ("Unexpected end of code in string literal.");
				} else if (current == '"') {
					GetNextChar (); // Skip the "
					break;
				} else {
					throw new Exception (string.Format ("Got unexpected character: {0}", current));
				}
			} while (true);
			current_token = new Token2 (Token2Type.Literal, builder.ToString ());
			return true;
		}
		
		if (IsPunctuation (current)) {
			current_token = new Token2 (Token2Type.Literal, current);
			return true;
		}
		
		if (IsIdentifier (current)) {
			builder.Append (current);
			while (IsIdentifier (PeekChar (1))) {
				builder.Append (GetNextChar ());
			}
			current_token = new Token2 (Token2Type.Identifier, builder.ToString ());
			return true;
		}
		
		if (current == char.MinValue) {
			if (throw_on_end)
				throw new Exception ("Unexpected end of code");
			return false;
		}
		
		throw new Exception (string.Format ("Unexpected character: {0}", current));
	}
	
	public void FindStartBrace ()
	{
		while (CurrentToken.value != "{") {
			Advance (true);
		}
	}
	
	public void SyncWithEndBrace ()
	{
		int braces = 0;
		
		AcceptOrThrow (Token2Type.Punctuation, "{");
		
		do {
			if (Accept (Token2Type.Punctuation, "{")) {
				braces++;
			} else if (Accept (Token2Type.Punctuation, "}")) {
				if (braces == 0)
					return;
				braces--;
			} else {
				Advance (true);
			}
		} while (true);
	}
	
	public string GetIdentifier ()
	{
		string result;
		VerifyIdentifier ();
		result = CurrentToken.value;
		Advance (true);
		return result;
	}
	
	public void VerifyIdentifier ()
	{
		VerifyType (Token2Type.Identifier);
	}
	
	public void VerifyType (Token2Type type)
	{
		if (CurrentToken.type != type)
			throw new Exception (string.Format ("Expected {0}, got {1}", type, CurrentToken));
	}
	
	public bool Accept (Token2Type type, string value)
	{
		if (CurrentToken.type != type)
			return false;
		
		if (CurrentToken.value != value)
			return false;
		
		Advance (true);
		return true;
	}
	
	public void AcceptOrThrow (Token2Type type, string value)
	{
		if (CurrentToken.type != type)
			throw new Exception (string.Format ("Expected {0} '{2}', got {1}", type, CurrentToken, value));
		
		if (CurrentToken.value != value)
			throw new Exception (string.Format ("Expected '{0}', not '{1}'", value, CurrentToken.value));
		
		Advance (true);
	}

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
		switch (c) {
		case '~':
		case '=':
		case '<':
		case '>':
		case ':':
		case ';':
		case '!':
		case '.':
		case ',':
		case '|':
		case '^':
		case '&':
		case '{':
		case '}':
		case '(':
		case ')':
		case '[':
		case ']':
		case '*':
		case '-':
		case '+':
		case '?':
		case '\\':
		case '/':
			return true;
		default:
			return false;
		}
	}
	
	public static bool IsWhiteSpace (char c)
	{
		return char.IsWhiteSpace (c);
	}
}
