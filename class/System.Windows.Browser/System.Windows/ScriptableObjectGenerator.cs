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
		public static bool ValidateType (Type t)
		{
			if (!t.IsDefined (typeof(ScriptableTypeAttribute), true)) {
				if (t.IsGenericType) {
					foreach (Type type in t.GetGenericArguments ()) {
						if (!IsSupportedType (type))
							return false;
					}
					return true;
				} else if (t.IsArray) {
					return IsSupportedType (t.GetElementType ());
				} else {
					if (ValidateProperties (t) | ValidateMethods (t) | ValidateEvents (t))
						return true;
				}
			} else
				return true;
			return false;
		}

		static bool ValidateProperties (Type t) {
			bool ret = false;

			foreach (PropertyInfo pi in t.GetProperties ()) {
				if (!pi.IsDefined (typeof(ScriptableMemberAttribute), true))
					continue;

				if (pi.PropertyType != t && !IsSupportedType (pi.PropertyType)) {
					throw new NotSupportedException (
						 String.Format ("The scriptable object type {0} has a property {1} whose type {2} is not supported",
								t, pi, pi.PropertyType));
				}
				ret = true;
			}
			return ret;
		}

		static bool ValidateMethods (Type t) {
			bool ret = false;

			foreach (MethodInfo mi in t.GetMethods ()) {
				if (!mi.IsDefined (typeof(ScriptableMemberAttribute), true))
					continue;

				if (mi.ReturnType != typeof (void) && mi.ReturnType != t && !IsSupportedType (mi.ReturnType))
					throw new NotSupportedException (
						 String.Format ("The scriptable object type {0} has a method {1} whose return type {2} is not supported",
								t, mi, mi.ReturnType));

				ParameterInfo[] ps = mi.GetParameters();
				foreach (ParameterInfo p in ps) {
					if (p.IsOut || (p.ParameterType != t && !IsSupportedType (p.ParameterType)))
						throw new NotSupportedException (
						 String.Format ("The scriptable object type {0} has a method {1} whose parameter {2} is of not supported type",
								t, mi, p));
				}

				ret = true;
			}

			return ret;
		}

		static bool ValidateEvents (Type t) {
			bool ret = false;

			foreach (EventInfo ei in t.GetEvents ()) {
				if (!ei.IsDefined (typeof(ScriptableMemberAttribute), true))
					continue;

// 				Console.WriteLine ("event handler type = {0}", ei.EventHandlerType);
// 				Console.WriteLine ("typeof (EventHandler<>) == {0}", typeof (EventHandler<>));

				if (ei.EventHandlerType != typeof (EventHandler) &&
					typeof (EventHandler<>).IsAssignableFrom (ei.EventHandlerType)) {
					if (!ValidateType (ei.EventHandlerType)) {
						throw new NotSupportedException (
							String.Format ("The scriptable object type {0} has a event {1} whose type {2} is not supported",
							t, ei, ei.EventHandlerType));
					}
				}

				ret = true;
			}
			return ret;
		}

		public static ScriptableObjectWrapper Generate (object instance, bool validate)
		{
			Type type = instance.GetType ();

			if (validate && !ValidateType (type))
				throw new ArgumentException (String.Format ("The scriptable type {0} does not have scriptable members", type));

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

		internal static bool IsSupportedType (Type t)
		{
			TypeCode tc = Type.GetTypeCode (t);
			if (tc == TypeCode.Object) {
				return ValidateType (t);
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

		static bool IsCreateable (Type type)
		{
			if (type != null && (Type.GetTypeCode (type) != TypeCode.Object || type == typeof (object)))
				return false;

			if (!type.IsVisible || type.IsAbstract ||
				type.IsInterface || type.IsPrimitive ||
				type.IsGenericTypeDefinition)
				return false;

			if (type.IsValueType)
				return false;

			// default constructor
			if (type.GetConstructor (BindingFlags.Public | BindingFlags.Instance, null, Type.EmptyTypes, null) == null)
				return false;

			return true;
		}
	}
}
