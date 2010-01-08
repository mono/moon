//
// System.Windows.ScriptableObjectGenerator class
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2007 Novell, Inc (http://www.novell.com)
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

using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Browser;
using System.Runtime.InteropServices;
using System.Reflection;
using System.Globalization;
using Mono;

namespace System.Windows
{
	// XXX This class shouldn't be needed.  MS just calls
	// Convert.ChangeType on the arguments, and *somehow* gets the
	// exception returned to JS.  We don't do that yet, so
	// unhandled exceptions crash the browser.  In an effort to
	// keep things limping along, we use this binder.
	sealed class JSFriendlyMethodBinder : Binder {
		public override FieldInfo BindToField (BindingFlags bindingAttr, FieldInfo [] match, object value, CultureInfo culture)
		{
			throw new NotImplementedException ();
		}

		public override MethodBase BindToMethod (BindingFlags bindingAttr, MethodBase [] match, ref object [] args, ParameterModifier [] modifiers, CultureInfo culture, string [] names, out object state)
		{
			throw new NotImplementedException ();
		}

		public override object ChangeType (object value, Type type, CultureInfo culture)
		{
			object ret;
			if (!TryChangeType (value, type, culture, out ret))
				throw new NotSupportedException (string.Format ("Can't change type from {0} to {1}", value.GetType ().FullName, type.FullName));
			return ret;
		}

		public bool TryChangeType (object value, Type type, CultureInfo culture, out object ret)
		{
			ScriptObject script_object;
			
			ret = value;

			if (value == null)
				return true;

			if (value.GetType() == type)
				return true;

			script_object = value as ScriptObject;
			if (script_object != null) {
				value = script_object.ManagedObject;
				if (value == null && type == typeof(HtmlElement))
					value = new HtmlElement (script_object.Handle);
				ret = value;
				if (value.GetType () == type)
					return true;
			}

			if (type.IsAssignableFrom (value.GetType ()))
				return true;
			
			if (type.IsEnum) {
				try {
					ret = Enum.Parse (type, value.ToString(), true);
					return true;
				} catch {
					return false;
				}
			}

			/* the set of source types for JS functions is
			 * very, very small, so we switch over the
			 * parameter type first */
			try {
				ret = Convert.ChangeType (value, type, culture);
				return true;
			}
			catch {
				// no clue if this is right.. if we
				// fail to convert, what do we return?

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

		public override void ReorderArgumentArray (ref object [] args, object state)
		{
			throw new NotImplementedException ();
		}

		public override MethodBase SelectMethod (BindingFlags bindingAttr, MethodBase [] match, Type [] types, ParameterModifier [] modifiers)
		{
			throw new NotImplementedException ();
		}

		public override PropertyInfo SelectProperty (BindingFlags bindingAttr, PropertyInfo [] match, Type returnType, Type [] indexes, ParameterModifier [] modifiers)
		{
			throw new NotImplementedException ();
		}
	}


	internal static class ScriptableObjectGenerator
	{
		public static ScriptableObjectWrapper Generate (object instance)
		{
			Type type = instance.GetType ();

			ScriptableObjectWrapper scriptable = new ScriptableObjectWrapper (instance);

			bool isScriptable = type.IsDefined (typeof(ScriptableTypeAttribute), true);

			// add properties

			foreach (PropertyInfo pi in type.GetProperties ()) {
				if (!isScriptable && !pi.IsDefined (typeof(ScriptableMemberAttribute), true))
					continue;
				scriptable.AddProperty (pi);
				if (AddTypes (pi))
					scriptable.HasTypes = true;
			}

			// add events
			foreach (EventInfo ei in type.GetEvents ()) {
				if (!isScriptable && !ei.IsDefined (typeof(ScriptableMemberAttribute), true))
					continue;
				scriptable.AddEvent (ei);
				scriptable.HasEvents = true;
			}

			// add functions
			foreach (MethodInfo mi in type.GetMethods ()) {
				if (!isScriptable && !mi.IsDefined (typeof(ScriptableMemberAttribute), true))
					continue;
				scriptable.AddMethod (mi);
				if (AddTypes (mi))
					scriptable.HasTypes = true;
			}

			if (scriptable.HasTypes)
				scriptable.AddMethod ("createManagedObject", new TypeCode[]{TypeCode.String}, TypeCode.Object);

			if (scriptable.HasEvents) {
				TypeCode[] tcs = new TypeCode [] {TypeCode.String, TypeCode.Object};
				scriptable.AddMethod ("addEventListener", tcs, TypeCode.Empty);
				scriptable.AddMethod ("removeEventListener", tcs, TypeCode.Empty);
			}

			return scriptable;
		}

		static bool AddTypes (PropertyInfo pi)
		{
			bool ret = false;
			if (IsCreateable (pi.PropertyType)) {
				ret = true;
				AddType (pi.PropertyType);
			}
			return ret;
		}

		static bool AddTypes (MethodInfo mi)
		{
			bool ret = false;
			if (IsCreateable (mi.ReturnType)) {
				ret = true;
				AddType (mi.ReturnType);
			}

			ParameterInfo[] ps = mi.GetParameters();
			foreach (ParameterInfo p in ps) {
				if (IsCreateable (p.ParameterType)) {
					ret = true;
					AddType (p.ParameterType);
				}
			}
			return ret;
		}

		static void AddType (Type type)
		{
			if (!HtmlPage.ScriptableTypes.ContainsKey (type.Name))
				HtmlPage.ScriptableTypes[type.Name] = type;
		}

		public static bool IsSupportedType (Type t)
		{
			TypeCode tc = Type.GetTypeCode (t);
			if (tc == TypeCode.Object) {
				return true;
			}

			switch (tc) {
			// string
			case TypeCode.Char:
			case TypeCode.String:
			// boolean
			case TypeCode.Boolean:
			// number
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
			// case TypeCode.Decimal: // decimal is unsupported(!)
				return true;
			}

			return false;
		}

		public static bool IsCreateable (Type type)
		{
			if (type != null && (type == typeof (object) || typeof(Delegate).IsAssignableFrom(type)))
				return false;

			if (!type.IsVisible || type.IsAbstract ||
				type.IsInterface || type.IsPrimitive ||
				type.IsGenericTypeDefinition)
				return false;

			// we like value types and arrays and things with default constructors
			if (!type.IsValueType && !type.IsArray && type.GetConstructor (BindingFlags.Public | BindingFlags.Instance, null, Type.EmptyTypes, null) == null)
				return false;

			return true;
		}

		public static bool IsScriptable (Type t)
		{
			if (t.IsDefined (typeof(ScriptableTypeAttribute), true))
				return true;

			foreach (MethodInfo mi in t.GetMethods ())
				if (mi.IsDefined (typeof(ScriptableMemberAttribute), true))
					return true;

			foreach (PropertyInfo pi in t.GetProperties ())
				if (pi.IsDefined (typeof(ScriptableMemberAttribute), true))
					return true;

			foreach (EventInfo ei in t.GetEvents ())
				if (ei.IsDefined (typeof(ScriptableMemberAttribute), true))
					return true;

			return false;
		}
	}
}
