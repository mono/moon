using System;
using System.Collections;
using System.Collections.Generic;
using System.Globalization;
using System.Reflection;
using System.Text;
using System.Security.Permissions;

using Mono;

using IJSONObject = System.Collections.Generic.IDictionary<string,object>;

namespace System.Windows.Browser.Serialization
{
	public class JavaScriptSerializer
	{
		JavaScriptTypeResolver resolver;
		int max_json_length, recursion_limit = 100;
		List<JavaScriptConverter> converters = new List<JavaScriptConverter> ();

		public JavaScriptSerializer ()
		{
			// with null type resolver.
		}
		
		public JavaScriptSerializer (JavaScriptTypeResolver resolver)
		{
			if (resolver == null)
				throw new ArgumentNullException ("resolver");
			this.resolver = resolver;
		}

		public T ConvertToType<T> (object obj)
		{
			if (obj is T)
				return (T) obj;
			if (obj is IJSONObject)
				return (T) ConvertTo ((IJSONObject) obj, typeof (T));

			try {
				// I'm not sure if System.Convert is used here.
				// (unlike ASP.NET Ajax; since there is no
				//  TypeDescriptor in silverlight System.dll)
				return (T) Convert.ChangeType (obj, typeof (T), CultureInfo.InvariantCulture);
			} catch (InvalidCastException ex) {
				throw new InvalidOperationException (String.Format ("Cannot convert object of type {0} to {1}", obj != null ? obj.GetType ().FullName : "(null)", typeof (T)), ex);
			}
		}
		
		public T Deserialize<T> (string input)
		{
			return ConvertToType<T> (DeserializeObject (input));
		}

		[MonoLimitation ("no max-length check is done (is it required?)")]
		public object DeserializeObject (string input)
		{
			if (input == null)
				throw new ArgumentNullException ("input");
			int i = 0;
			object o = DeserializeCore (input, ref i);
			if (i != input.Length)
				throw new ArgumentException (String.Format ("Invalid JSON primitive at {0}", i));
			return o;
		}

		public int MaxJsonLength {
			get { return max_json_length; }
			set {
				if (value <= 0)
					throw new ArgumentException ("value cannot be non-positive value.");
				max_json_length = value;
			}
		}
		
		public int RecursionLimit {
			get { return recursion_limit; }
			set {
				if (value <= 0)
					throw new ArgumentException ("value cannot be non-positive value.");
				recursion_limit = value;
			}
		}
		
		public void RegisterConverters (IEnumerable<JavaScriptConverter> converters)
		{
			foreach (JavaScriptConverter c in converters)
				if (!this.converters.Contains (c))
					this.converters.Add (c);
		}
		
		public string Serialize (object obj)
		{
			StringBuilder sb = new StringBuilder ();
			Serialize (obj, sb);
			return sb.ToString ();
		}

		[MonoLimitation ("No max-length check is done yet")]
		public void Serialize (object obj, StringBuilder output)
		{
			Serialize (obj, output, 0);
		}

		#region Deserialization impl.

		object DeserializeCore (string input, ref int i)
		{
			i = SkipSpaces (input, i);
			if (i == input.Length)
				return null;
			char c = input [i];
			switch (c) {
			case '[':
				i++;
				List<object> list = new List<object> ();
				i = SkipSpaces (input, i);
				while (true) {
					list.Add (DeserializeCore (input, ref i));
					i = SkipSpaces (input, i);
					if (i == input.Length || input [i] != ',')
						break;
					i++;
				}
				if (i == input.Length || input [i] != ']')
					throw new ArgumentException (String.Format ("Invalid JSON array format; ']' is expected, at {0}", i));
				i++;
				return list.ToArray ();
			case '{':
				i++;
				IJSONObject dic = new Dictionary<string,object> ();
				i = SkipSpaces (input, i);
				bool repeat = false;
				while (repeat || i < input.Length && input [i] != '}') {
					string name = ReadStringLiteral (input, ref i);
					i = SkipSpaces (input, i);
					if (i == input.Length || input [i] != ':')
						throw new ArgumentException (String.Format ("Invalid JSON object format; ':' is expected after a name, at {0}", i));
					i++;
					dic.Add (name, DeserializeCore (input, ref i));
					i = SkipSpaces (input, i);
					if (i == input.Length || input [i] != ',')
						break;
					repeat = true; // after comma, an item is mandatory
					i++;
					i = SkipSpaces (input, i);
				}
				if (i == input.Length || input [i] != '}')
					throw new ArgumentException (String.Format ("Invalid JSON object format; '}}' is expected, at {0}", i));
				i++;

				// creates a resolved instance if required.
				if (resolver == null || !dic.ContainsKey ("__type"))
					return dic;
				Type type = resolver.ResolveType (dic ["__type"] as string);
				if (type == null)
					// FIXME: could be different kind of exception?
					throw new InvalidOperationException (String.Format ("Type '{0}' cannot be resolved", dic ["__type"]));

				return ConvertTo (dic, type);
			default:
				if ('0' <= c && c <= '9' || c == '-')
					return ReadNumericLiteral (input, ref i);
				if (c == '"')
					return ReadStringLiteral (input, ref i);
				break;
			}
			if (String.CompareOrdinal (input, i, "null", 0, 4) == 0) {
				i += 4;
				return null;
			}

			throw new ArgumentException (String.Format ("Invalid JSON format; extra token at {0}", i));
		}

		object ConvertTo (IJSONObject dic, Type type)
		{
			// try custom converters, and then default deserialization.
			foreach (JavaScriptConverter conv in converters)
				foreach (Type t in conv.SupportedTypes)
					if (t == type)
						return conv.Deserialize (dic, type, this);

			object o = Helper.CreateInstance (type, false);
			foreach (KeyValuePair<string,object> p in dic) {
				if (p.Key == "__type")
					continue;
				PropertyInfo prop = type.GetProperty (p.Key);
				if (prop != null)
					// ChangeType() is needed for example setting double value to int property.
					prop.SetValue (o, Convert.ChangeType (p.Value, prop.PropertyType, CultureInfo.InvariantCulture), empty_args);
			}
			return o;
		}

		static int SkipSpaces (string s, int i)
		{
			while (i < s.Length) {
				switch (s [i]) {
				case ' ': case '\t': case '\r': case '\n':
					i++;
					continue;
				}
				return i;
			}
			return i;
		}

		static decimal ReadNumericLiteral (string input, ref int i)
		{
			bool negative = false;
			if (input [i] == '-') {
				negative = true;
				i++;
				if (i == input.Length)
					throw new ArgumentException (String.Format ("Invalid JSON numeric literal; extra negation at {0}", i - 1));
			}
			return (negative ? -1 : 1) * ReadPositiveNumericLiteral (input, ref i);
		}

		static decimal ReadPositiveNumericLiteral (string input, ref int i)
		{
			int val = 0;
			int start = i;
			int leadingZeroAt = i < input.Length && input [i] == '0' ? i : -1;
			for (;i < input.Length; i++) {
				if (input [i] < '0' || '9' < input [i])
					break;
				val = val * 10 + (input [i] - '0');
			}
			if (leadingZeroAt >= 0 && i > leadingZeroAt + 1)
				throw new ArgumentException (String.Format ("Invalid JSON numeric literal; leading multiple zeros are not allowed at {0}", leadingZeroAt));
			if (i == input.Length)
				return val;
			decimal frac = 0;
			int fdigits = 0;
			if (input [i] == '.') {
				i++;
				if (i == input.Length)
					throw new ArgumentException (String.Format ("Invalid JSON numeric literal; extra dot at {0}", i - 1));
				for (decimal d = 10; i < input.Length; i++) {
					if (input [i] < '0' || '9' < input [i])
						break;
					frac += (input [i] - '0') / d;
					d *= 10;
					fdigits++;
				}
				if (fdigits == 0)
					throw new ArgumentException (String.Format ("Invalid JSON numeric literal; extra dot at {0}", i - 1));
			}
			frac = Decimal.Round (frac, fdigits);
			if (i == input.Length || input [i] != 'e' && input [i] != 'E')
				return val + frac;
			i++;
			int exp = 0;
			if (i == input.Length)
				throw new ArgumentException (String.Format ("Invalid JSON numeric literal; extra exponent at {0}", i - 1));
			bool negexp = false;
			if (input [i] == '-') {
				i++;
				negexp = true;
			}
			else if (input [i] == '+')
				i++;
			if (i == input.Length)
				throw new ArgumentException (String.Format ("Invalid JSON numeric literal; extra exponent at {0}", i - 1));
			for (;i < input.Length; i++) {
				if (input [i] < '0' || '9' < input [i])
					break;
				exp = exp * 10 + (input [i] - '0');
			}
			// it is messy to handle exponent, so I just use Decimal.Parse() with assured JSON format.
			return Decimal.Parse (input.Substring (start, i - start), NumberStyles.Any, CultureInfo.InvariantCulture);
		}

		static string ReadStringLiteral (string input, ref int i)
		{
			if (input [i] != '"')
				throw new ArgumentException (String.Format ("Invalid JSON string literal format at {0}", i));

			i++;
			StringBuilder sb = null;
			for (int start = i; i < input.Length; i++) {
				if (input [i] == '"') {
					string remaining = input.Substring (start, i - start);
					i++;
					if (sb != null)
						return sb.Append (remaining).ToString ();
					else
						return remaining;
				}
				else if (input [i] != '\\')
					continue;

				// escaped expression
				if (sb == null)
					sb = new StringBuilder ();
				if (i != start)
					sb.Append (input, start, i - start);
				i++;
				if (i == input.Length)
					throw new ArgumentException (String.Format ("Invalid JSON string literal; incomplete escape sequence at {0}", i - 1));
				switch (input [i]) {
				case '"':
				case '\\':
				case '/':
					sb.Append (input [i]);
					break;
				case 'b':
					sb.Append ('\x8');
					break;
				case 'f':
					sb.Append ('\f');
					break;
				case 'n':
					sb.Append ('\n');
					break;
				case 'r':
					sb.Append ('\r');
					break;
				case 't':
					sb.Append ('\t');
					break;
				case 'u':
					ushort cp;
					try {
						cp = ushort.Parse (input.Substring (i + 1, 4), NumberStyles.HexNumber, CultureInfo.InvariantCulture);
					} catch (Exception e) {
						throw new ArgumentException (String.Format ("Invalid JSON string literal; \\u format expects four digits, at {0}", i), e);
					}

					i += 4; // another 1 is added later.
Console.WriteLine ("[{0}]", sb);
					sb.Append ((char) cp);
Console.WriteLine ("[{0}]", sb);
					break;
				default:
					throw new ArgumentException (String.Format ("Invalid JSON string literal; unexpected escape character at {0}", i));
				}
				start = i + 1;
			}
			throw new ArgumentException ("Invalid JSON string literal; missing closing quotation");
		}

		#endregion

		#region Serialization impl.

		static readonly object [] empty_args = new object [0];

		void Serialize (object obj, StringBuilder output, int recursion)
		{
			if (recursion == RecursionLimit)
				throw new InvalidOperationException ("The serialization process reached the recursion limit");

			if (obj == null) {
				output.Append ("null");
				return;
			}

			Type type = obj.GetType ();
			foreach (JavaScriptConverter c in converters)
				foreach (Type t in c.SupportedTypes)
					if (t == type) {
						Serialize (c.Serialize (obj, this), output, recursion + 1);
						return;
					}

			switch (Type.GetTypeCode (type)) {
			case TypeCode.Boolean:
				output.Append (((bool) obj) ? "true" : "false");
				break;
			case TypeCode.Char:
			case TypeCode.String:
				output.Append ('"').Append (EscapeStringLiteral (obj.ToString ())).Append ('"');
				break;
			case TypeCode.Byte:
			case TypeCode.Decimal:
			case TypeCode.Single:
			case TypeCode.Double:
			case TypeCode.Int16:
			case TypeCode.Int32:
			case TypeCode.Int64:
			case TypeCode.UInt16:
			case TypeCode.UInt32:
			case TypeCode.UInt64:
			case TypeCode.SByte:
				output.Append (obj);
				break;
			case TypeCode.DateTime:
				output.Append ("\"\\/Date(").Append (((DateTime) obj).Ticks).Append (")\\/");
				break;
			case TypeCode.DBNull:
				output.Append ("null");
				break;
			case TypeCode.Object:
			case TypeCode.Empty:
				if (obj is IJSONObject) {
					IJSONObject map = (IJSONObject) obj;
					output.Append ('{');
					foreach (KeyValuePair<string,object> p in map) {
						output.Append ('"').Append (p.Key).Append ("\":");
						Serialize (p.Value, output, recursion + 1);
						output.Append (',');
					}
					if (output [output.Length - 1] == ',')
						output.Length--;
					output.Append ('}');
				} else if (obj is IEnumerable) {
					output.Append ('[');
					foreach (object item in (IEnumerable) obj) {
						Serialize (item, output, recursion + 1);
						output.Append (',');
					}
					if (output [output.Length - 1] == ',')
						output.Length--;
					output.Append (']');
				} else {
					output.Append ('{');
					bool hasItem = false;
					if (resolver != null) {
						string id = resolver.ResolveTypeId (type);
						if (id != null) {
							hasItem = true;
							output.Append ("\"__type\":\"").Append (type.AssemblyQualifiedName.ToString ()).Append ("\"");
						}
					}
					foreach (PropertyInfo pi in type.GetProperties ())
						if (pi.GetCustomAttributes (typeof (ScriptIgnoreAttribute), false).Length == 0) {
							if (hasItem)
								output.Append (',');
							output.Append ('"').Append (EscapeStringLiteral (pi.Name)).Append ("\":");
							Serialize (pi.GetValue (obj, empty_args), output, recursion + 1);
							hasItem = true;
						}
					output.Append ('}');
				}
				break;
			}
		}

		static void AppendBuffer (ref StringBuilder sb, string input, int start, int i, string append)
		{
			if (sb == null)
				sb = new StringBuilder ();
			if (i != start)
				sb.Append (input, start, i - start);
			sb.Append (append);
		}

		static string EscapeStringLiteral (string input)
		{
			StringBuilder sb = null;
			int i = 0, start = 0;
			for (; i < input.Length; i++) {
				switch (input [i]) {
				case '"':
					AppendBuffer (ref sb, input, start, i, @"\""");
					break;
				case '\\':
					AppendBuffer (ref sb, input, start, i, @"\\");
					break;
				//case '/':
				//	AppendBuffer (ref sb, input, start, i, @"\/");
				//	break;
				case '\x8':
					AppendBuffer (ref sb, input, start, i, @"\b");
					break;
				case '\f':
					AppendBuffer (ref sb, input, start, i, @"\f");
					break;
				case '\n':
					AppendBuffer (ref sb, input, start, i, @"\n");
					break;
				case '\r':
					AppendBuffer (ref sb, input, start, i, @"\r");
					break;
				case '\t':
					AppendBuffer (ref sb, input, start, i, @"\t");
					break;
				default:
					continue;
				}
				start = i + 1;
			}
			string remaining = input.Substring (start, i - start);
			if (sb != null)
				return sb.Append (remaining).ToString ();
			else
				return remaining;
		}

		#endregion
	}
}
