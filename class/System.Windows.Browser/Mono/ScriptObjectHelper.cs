//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2010 Novell, Inc (http://www.novell.com)
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

using System.Windows.Browser;
using System.Runtime.InteropServices;
using System.Reflection;
using System.Windows.Interop;
using System;
using System.Globalization;

namespace Mono {

	internal static class ScriptObjectHelper {

		public static bool TryChangeType (object value, Type type, CultureInfo culture, out object ret)
		{
			ret = value;
			if (value == null)
				return true;

			if (value.GetType() == type)
				return true;

			ScriptObject so = value as ScriptObject;
			if (so != null) {
				if (value.GetType ().Equals (typeof(ManagedObject)) || value.GetType().Equals(typeof(ScriptObject))) {
					ret = so.ConvertTo (type, false);
					return true;
				} else if (type.IsAssignableFrom (value.GetType ())) {
					return true;
				}
				return false;
			}

			bool ismousebuttons = type.Equals (typeof(MouseButtons));
			if (ismousebuttons) {
				try {
					int d = (int) Convert.ChangeType (value, typeof(int), culture);
					switch (d) {
						case 0: ret = MouseButtons.Left; break;
						case 1: ret = MouseButtons.Middle; break;
						case 2:	ret = MouseButtons.Right; break;
						default:
							return false;
					}
					return true;
				} catch {
					return false;
				}
			}

			if (type.IsAssignableFrom (value.GetType ())) {
				return true;
			}

			if (type.IsEnum) {
				try {
					ret = Enum.Parse (type, value.ToString (), true);
					return true;
				} catch {
					return false;
				}
			}

			if (type == typeof(Guid) && value is string) {
				ret = new Guid ((string)value);
				return true;
			}

			if (type == typeof (ScriptObject)) {
				ret = new ManagedObject (value);
				return true;
			}

			try {
				ret = Convert.ChangeType (value, type, culture);
				return true;
			}
			catch {
				switch (Type.GetTypeCode (type))
				{
					case TypeCode.Char:
					case TypeCode.Byte:
					case TypeCode.SByte:
					case TypeCode.Int16:
					case TypeCode.Int32:
					case TypeCode.Int64:
					case TypeCode.UInt16:
					case TypeCode.UInt32:
					case TypeCode.UInt64:
					case TypeCode.Single:
					case TypeCode.Double:
						ret = Convert.ChangeType (0, type, culture);
						return true;
					case TypeCode.String:
						ret = "";
						return true;
					case TypeCode.Boolean:
						ret = false;
						return true;
				}
			}

			return false;

		}

		public static object FromValue (Value v)
		{
			// When the target type is a number or equivalent, SL converts ints
			// to doubles to wash out
			// browser differences. (Safari apparently always returns doubles, FF
			// ints and doubles, depending on the value).
			// See: http://msdn.microsoft.com/en-us/library/cc645079(VS.95).aspx

			switch (v.k) {
				case Kind.BOOL:
					return v.u.i32 != 0;
				case Kind.INT32:
					return (double) v.u.i32;
				case Kind.DOUBLE:
					return v.u.d;
				case Kind.STRING:
					return Marshal.PtrToStringAnsi (v.u.p);
				case Kind.NPOBJ:
					if (v.IsNull)
						return null;

					ScriptObject reference = ScriptObject.LookupScriptObject (v.u.p);
					if (reference != null)
						return reference;

					if (NativeMethods.html_object_has_property (PluginHost.Handle, v.u.p, "_internal_moonlight_marker")) {
						Value val;
						NativeMethods.html_object_get_property (PluginHost.Handle, v.u.p, "_internal_moonlight_marker", out val);
						reference = ScriptObject.LookupScriptObject (new IntPtr (val.u.i32));
						if (reference != null) {
							return reference;
						}
					}

					if (NativeMethods.html_object_has_property (PluginHost.Handle, v.u.p, "nodeType")) {
						Value val;
						NativeMethods.html_object_get_property (PluginHost.Handle, v.u.p, "nodeType", out val);
						int r = val.u.i32;
						NativeMethods.value_free_value (ref val);
						if (r == (int)HtmlElement.NodeType.Document)
							return new HtmlDocument (v.u.p);
						else if (r == (int)HtmlElement.NodeType.Element ||
								 r == (int)HtmlElement.NodeType.Comment ||
								 r == (int)HtmlElement.NodeType.Text)
							return new HtmlElement (v.u.p);

					}
					else if (NativeMethods.html_object_has_property (PluginHost.Handle, v.u.p, "location"))
						return new HtmlWindow (v.u.p);
					else if (NativeMethods.html_object_has_property (PluginHost.Handle, v.u.p, "length") &&
							 NativeMethods.html_object_has_method (PluginHost.Handle, v.u.p, "item"))
						return new ScriptObjectCollection (v.u.p);
					return new ScriptObject (v.u.p);
			default:
				Console.WriteLine ("unsupported Kind.{0}", v.k);
				throw new NotSupportedException ();
			}
		}

		public static void ToValue (ref Value v, object o)
		{
			if (o == null) {
				v.k = Kind.NPOBJ;
				v.u.p = IntPtr.Zero;
				return;
			}

			if (o is sbyte || o is short || o is int || o is byte || o is ushort ||
				o is uint || o is long || o is ulong || o is float || o is double ||
				o is decimal || o.GetType ().IsEnum) {
				v.k = Kind.DOUBLE;
				v.u.d = Convert.ToDouble (o);
			} else if (o is bool) {
				v.k = Kind.BOOL;
				v.u.i32 = ((bool) o) ? 1 : 0;
			} else if (o is char || o is string || o is Guid) {
				v.k = Kind.STRING;
				v.u.p = Value.StringToIntPtr (o.ToString ());
			} else if (o is DateTime) {
				v.k = Kind.DATETIME;
				v.u.i64 = (((DateTime)o).Ticks - new DateTime (1970,1,1).Ticks) / 10000;
			} else if (o is ScriptObject) {
				// FIXME: We should ref the SO here
				v.k = Kind.NPOBJ;
				v.u.p = ((ScriptObject)o).Handle;
			} else {
				// FIXME: We should ref the SO here
				v.k = Kind.NPOBJ;
				v.u.p = ManagedObject.GetManagedObject (o).Handle;
			}
		}

		public static T CreateInstance<T> (IntPtr ptr)
		{
			ConstructorInfo i = typeof(T).GetConstructor (BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.Instance,
						null, new Type[]{typeof(IntPtr)}, null);
			object o = i.Invoke (new object[]{ptr});
			return (T) o;
		}

		public static object CreateInstance (Type type, IntPtr ptr)
		{
			ConstructorInfo i = type.GetConstructor (BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.Instance,
						null, new Type[]{typeof(IntPtr)}, null);
			object o = i.Invoke (new object[]{ptr});
			return o;
		}

	}

}