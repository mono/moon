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

namespace Mono
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
				throw new InvalidOperationException (string.Format ("Can't change type from {0} to {1}", value.GetType ().FullName, type.FullName));
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

			if (type.Equals (typeof(object))) {
				ManagedObject mo = value as ManagedObject;
				if (mo != null)
					ret = mo.ManagedObjectCore;
				return true;
			}

			script_object = value as ScriptObject;
			if (script_object != null) {
				if (typeof(ScriptObject).IsAssignableFrom (type))
					return true;
				else {
					value = script_object.ManagedObject;
					if (value == null) {
						if (typeof(Delegate).IsAssignableFrom(type))
							value = new ScriptObjectEventInfo (script_object, type).GetDelegate ();
						else if (type == typeof(HtmlElement))
							value = new HtmlElement (script_object.Handle);
						else if (type == typeof(DateTime)) {
							try {
								ret = new DateTime (new DateTime (1970,1,1).Ticks + script_object.InvokeInternal<long> ("getTime") * 10000);
							} catch {
								return false;
							}
							return true;
						}
					}
					ret = value;
					if (value.GetType () == type)
						return true;
				}
			}

			if (type.IsAssignableFrom (value.GetType ()))
				return true;

			if (type == typeof (ScriptObject)) {
				ret = new ManagedObject (value);
				return true;
			}

			if (type.IsEnum) {
				try {
					ret = Enum.Parse (type, value.ToString(), true);
					return true;
				} catch {
					return false;
				}
			}

			if (type == typeof(Guid) && value is string) {
				ret = new Guid ((string)value);
				return true;
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
}
