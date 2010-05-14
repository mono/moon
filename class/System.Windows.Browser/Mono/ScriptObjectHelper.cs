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

namespace Mono {

	internal static class ScriptObjectHelper {

		public static object ObjectFromValue<T> (Value v)
		{
			// When the target type is object, SL converts ints to doubles to wash out
			// browser differences. (Safari apparently always returns doubles, FF
			// ints and doubles, depending on the value).
			// See: http://msdn.microsoft.com/en-us/library/cc645079(VS.95).aspx

			Type type = typeof (T);
			bool isobject = type.Equals (typeof(object));
			bool ismousebuttons = type.Equals (typeof(MouseButtons));

			switch (v.k) {
			case Kind.BOOL:
				return v.u.i32 != 0;
			case Kind.UINT64:
				if (isobject)
					return (double) v.u.ui64;
				else if (type.IsAssignableFrom (typeof (UInt64)))
					return Convert.ChangeType (v.u.i64, type, null);
				return v.u.ui64;
			case Kind.INT32:
				if (isobject)
					return (double) v.u.i32;
				else if (ismousebuttons) {
					switch (v.u.i32) {
						case 0: return MouseButtons.Left;
						case 1: return MouseButtons.Middle;
						case 2:	return MouseButtons.Right;
						default:
							throw new ArgumentException ("The browser returned an unsupported value for 'button'");
					}
				} else if (type.IsAssignableFrom (typeof (Int32)))
					return Convert.ChangeType (v.u.i32, type, null);
				return v.u.i32;
			case Kind.INT64:
				if (isobject)
					return (double) v.u.i64;
				else if (type.IsAssignableFrom (typeof (Int64)))
					return Convert.ChangeType (v.u.i64, type, null);
				return v.u.i64;
			case Kind.DOUBLE:
				return v.u.d;
			case Kind.STRING:
				string s = Marshal.PtrToStringAnsi (v.u.p);
				if (isobject || type.Equals (typeof (string)))
					return s;
				else if (type.Equals (typeof(DateTime)))
					return DateTime.Parse (s);
				return Convert.ChangeType (s, type, null);
			case Kind.NPOBJ:
				// FIXME: Move all of this one caller up
				if (type.Equals (typeof(IntPtr)))
				    return v.u.p;

				if (v.u.p == IntPtr.Zero)
					return null;

				ScriptObject reference = ScriptObject.LookupScriptObject (v.u.p, false);
				if (reference != null)
					return reference;

				if (!isobject && typeof (ScriptObject).IsAssignableFrom (type)) {
					return CreateInstance<T> (v.u.p);
				} else if (isobject) {
					if (NativeMethods.html_object_has_property (PluginHost.Handle, v.u.p, "nodeType")) {
						Value val;
						NativeMethods.html_object_get_property (PluginHost.Handle, v.u.p, "nodeType", out val);

						if (val.u.i32 == 9 /* HtmlDocument */) {
							object result = CreateInstance<HtmlDocument> (v.u.p);
							NativeMethods.value_free_value (ref val);
							return result;
						}
						else if (val.u.i32 == 1 /* HtmlElement */) {
							object result = CreateInstance<HtmlElement> (v.u.p);
							NativeMethods.value_free_value (ref val);
							return result;
						}

						NativeMethods.value_free_value (ref val);
					}
					else if (NativeMethods.html_object_has_property (PluginHost.Handle, v.u.p, "location")) {
						return CreateInstance<HtmlWindow> (v.u.p);
					}
					return CreateInstance<ScriptObject> (v.u.p);
				} else
					return v.u.p;
			default:
				Console.WriteLine ("unsupported Kind.{0}", v.k);
				throw new NotSupportedException ();
			}
		}
	

		public static void ValueFromObject (ref Value v, object o)
		{
			if (o == null) {
				v.k = Kind.NPOBJ;
				v.u.p = IntPtr.Zero;
				return;
			}

			switch (Type.GetTypeCode (o.GetType())) {
			case TypeCode.Boolean:
			case TypeCode.Double:		
			case TypeCode.Int32:
			case TypeCode.UInt32:			
			case TypeCode.Int64:
			case TypeCode.UInt64:			
			case TypeCode.String:
				//
				// XXX - jackson: I left the switch in because Value.FromObject allows way more types
				// than this method used to.
				//
				
				v = Value.FromObject (o);
				break;
			case TypeCode.Object:
//				Console.WriteLine ("Trying to marshal managed object {0}...", o.GetType ().FullName);
				ScriptObject so = o as ScriptObject;
				if (so != null) {
					v.u.p = so.Handle;
				} else {
					ManagedObject obj = new ManagedObject (o);
					v.u.p = obj.Handle;
				}
				v.k = Kind.NPOBJ;
//				Console.WriteLine ("  Marshalled as {0}", v.k);
				break;
			default:
				Console.WriteLine ("unsupported TypeCode.{0} = {1}", Type.GetTypeCode(o.GetType()), o.GetType ().FullName);
				throw new NotSupportedException ();
			}
		}

		public static T CreateInstance<T> (IntPtr ptr)
		{
			ConstructorInfo i = typeof(T).GetConstructor (BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.Instance,
						null, new Type[]{typeof(IntPtr)}, null);

			object o = i.Invoke (new object[]{ptr});

			return (T) o;
		}

	
	}

}