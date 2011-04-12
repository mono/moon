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

using System.Globalization;
using System.Collections;
using System.ComponentModel;
using System.Reflection;
using System.Windows.Interop;
using System.Windows.Threading;
using System.Collections.Generic;
using Mono;

namespace System.Windows.Browser {

	internal class ManagedObject : ScriptObject {

		object managed;
		static Dictionary<object, WeakReference> cachedObjects;

		static ManagedObject ()
		{
			cachedObjects = new Dictionary<object, WeakReference> ();
		}

		public ManagedObject (object obj)
		{
//			Console.WriteLine ("new ManagedObject created wrapping object of type {0}, handle == {1}", obj.GetType(), Handle);

			managed = obj;
			lock (cachedObjects)
				cachedObjects[obj] = new WeakReference (this);

			Type type = obj.GetType ();

			bool isScriptable = type.IsDefined (typeof(ScriptableTypeAttribute), true);

			// add properties
			TypeOps typeOps = null;

			foreach (PropertyInfo pi in type.GetProperties ()) {
				if (!isScriptable && !pi.IsDefined (typeof(ScriptableMemberAttribute), true))
					continue;
				RegisterScriptableProperty (pi);
				if (IsCreateable (pi.PropertyType)) {
					typeOps = typeOps ?? new TypeOps ();
					typeOps.RegisterCreateableType (pi.PropertyType);
				}
			}

			// add events
			foreach (EventInfo ei in type.GetEvents ()) {
				if (!isScriptable && !ei.IsDefined (typeof(ScriptableMemberAttribute), true))
					continue;
				RegisterScriptableEvent (ei);
				HasEvents = true;

				// XXX toshok - do we need to RegisterCreateableTypes for parameters on the delegate?
			}

			// add functions
			foreach (MethodInfo mi in type.GetMethods ()) {
				if (!isScriptable && !mi.IsDefined (typeof(ScriptableMemberAttribute), true))
					continue;
				RegisterScriptableMethod (mi);
				if (IsCreateable (mi)) {
					typeOps = typeOps ?? new TypeOps ();
					typeOps.RegisterCreateableTypes (mi);
				}
			}

			if (typeOps != null)
				RegisterBuiltinScriptableMethod (typeof (TypeOps).GetMethod ("CreateManagedObject"), "createManagedObject", typeOps);

			if (HasEvents) {
				EventOps eventOps = new EventOps (this);
				Type eventOpsType = typeof (EventOps);
				RegisterBuiltinScriptableMethod (eventOpsType.GetMethod ("AddEventListener"), "addEventListener", eventOps);
				RegisterBuiltinScriptableMethod (eventOpsType.GetMethod ("RemoveEventListener"), "removeEventListener", eventOps);
			}

			RegisterScriptableMethod (type.GetMethod ("ToString", Type.EmptyTypes), "toString");

			if (ManagedObject is IList) {
				if (type.GetProperty ("Length") != null)
					RegisterScriptableProperty (type.GetProperty ("Length"), "length");
				else if (type.GetProperty ("Count") != null)
					RegisterScriptableProperty (type.GetProperty ("Count"), "length");

				foreach (MethodInfo mi in type.GetMethods ()) {
					switch (mi.Name) {
					case "IndexOf":
						RegisterScriptableMethod (mi, "indexOf");
						break;
					case "LastIndexOf":
						RegisterScriptableMethod (mi, "lastIndexOf");
						break;
					case "ToArray":
						RegisterScriptableMethod (mi, "toArray");
						break;
					}
				}

				Type listType = typeof(object);
				if (type.IsArray)
					listType = type.GetElementType ();
				else {
					foreach (Type t in type.GetInterfaces()) {
						if (t.IsGenericType && t.GetGenericTypeDefinition () == typeof (IList<>)) {
							listType = t.GetGenericArguments ()[0];
							break;
						}
					}
				}

				var listOps = CreateListOpsInstance (listType);
				Type listOpsType = listOps.GetType ();

				if (type.GetProperty ("Item") != null)
					RegisterScriptableProperty (type.GetProperty ("Item"), "item");
				else
					RegisterScriptableProperty (listOpsType.GetProperty ("Item"), "item", listOps);

				RegisterScriptableMethod (listOpsType.GetMethod ("Pop"), "pop", listOps);
				RegisterScriptableMethod (listOpsType.GetMethod ("Push"), "push", listOps);
				RegisterScriptableMethod (listOpsType.GetMethod ("Reverse"), "reverse", listOps);
				RegisterScriptableMethod (listOpsType.GetMethod ("Shift"), "shift", listOps);
				RegisterScriptableMethod (listOpsType.GetMethod ("Unshift"), "unshift", listOps);
				RegisterScriptableMethod (listOpsType.GetMethod ("Splice"), "splice", listOps);
			} else if (ManagedObject is IDictionary) {
				DictOps dictOps = new DictOps ((IDictionary)ManagedObject);
				Type dictOpsType = typeof (DictOps);

				RegisterScriptableProperty (dictOpsType.GetProperty ("Item"), "item", dictOps);
			}
		}

		~ManagedObject ()
		{
			lock (cachedObjects)
				cachedObjects.Remove (ManagedObject);
		}

		// extract small-ish [SecurityCritical] code from the .ctor code
		object CreateListOpsInstance (Type listType)
		{
			return Activator.CreateInstance (Type.GetType ("System.Windows.Browser.ManagedObject+ListOps`1").
				MakeGenericType (listType), new object[] { (IList) ManagedObject });
		}

		internal static ManagedObject GetManagedObject (object o)
		{
			ManagedObject obj = null;
			WeakReference wref;

			lock (cachedObjects) {
				cachedObjects.TryGetValue (o, out wref);
				if (wref != null) {
					if (wref.IsAlive)
						obj = wref.Target as ManagedObject;
				} else
					obj = new ManagedObject (o);
			}

			return obj;
		}


		public override void SetProperty (string name, object value)
		{
			if (!properties.ContainsKey (name))
				throw new InvalidOperationException ("Property '" + name + "' not found");

			PropertyInfo pi = properties[name].property;
			object obj = properties[name].obj;
			if (pi.GetSetMethod () == null)
				throw new InvalidOperationException ("Property '" + name + "' cannot be set");
			pi.SetValue (obj, value, BindingFlags.SetProperty, new JSFriendlyMethodBinder (), null, CultureInfo.InvariantCulture);
		}

		internal override void SetProperty (string name, object[] args)
		{
			if (!properties.ContainsKey (name))
				throw new InvalidOperationException ("Property '" + name + "' not found");

			PropertyInfo pi = properties[name].property;
			object obj = properties[name].obj;
			MethodInfo mi = pi.GetSetMethod ();
			if (ValidateArguments (mi, args))
				Invoke (mi, obj, args);
			else
				throw new ArgumentException ("args");
		}

		public override object GetProperty (string name)
		{
			// this should likely call a GetProperty overload passing ManagedObject in, instead of "this",
			// but we already do the book keeping in the base class, so do we need this method at all?

			return base.GetProperty (name, new object[]{});
		}

		protected internal override object ConvertTo (Type targetType, bool allowSerialization)
		{
			if (targetType.IsAssignableFrom (ManagedObject.GetType ()))
				return ManagedObject;

			if (typeof (ScriptObject).IsAssignableFrom (targetType))
				return this;

			if (allowSerialization)
				return HostServices.Current.JsonDeserialize (ManagedObject, targetType);

			return null;
		}

		public override object Invoke (string name, params object [] args)
		{
			// this should likely call an Invoke overload passing ManagedObject in, instead of "this",
			// but we already do the book keeping in the base class, so do we need this method at all?

			Value v = new Value ();

			base.Invoke (name, args, ref v);

			return ScriptObjectHelper.FromValue (v);
		}

		public override object InvokeSelf (params object [] args)
		{
			Delegate d = managed as Delegate;
			if (d == null)
				throw new InvalidOperationException ("Invoke failed");
			return d.DynamicInvoke (args);
		}

		internal override object ManagedObjectCore {
			get { return managed; }
		}

		bool IsCreateable (MethodInfo mi)
		{
			if (IsCreateable (mi.ReturnType))
				return true;

			ParameterInfo[] ps = mi.GetParameters();
			foreach (ParameterInfo p in ps) {
				if (IsCreateable (p.ParameterType))
					return true;
			}

			return false;
		}

		static string ScriptName (Type type) {
			if (type.IsGenericType) {
				string str = "";
				foreach (Type t in type.GetGenericArguments()) {
					if (str != "")
						str += ",";
					str += ScriptName(t);
				}
				return ((type.Name.IndexOf('`') > 0 ? type.Name.Substring(0, type.Name.IndexOf('`')) : type.Name) + "<" + str + ">");
			} else if (type.IsArray) {
				return ScriptName (type.GetElementType ()) + "[]";
			} else if (type.IsPrimitive) {
				if (type == typeof(int))
					return "int";
				if (type == typeof(byte))
					return "byte";
				if (type == typeof(char))
					return "char";
				if (type == typeof(bool))
					return "bool";
				if (type == typeof(decimal))
					return "decimal";
				if (type == typeof(double))
					return "double";
				if (type == typeof(sbyte))
					return "sbyte";
				if (type == typeof(long))
					return "long";
				if (type == typeof(short))
					return "short";
				if (type == typeof(float))
					return "float";
				if (type == typeof(uint))
					return "uint";
				if (type == typeof(ulong))
					return "ulong";
				if (type == typeof(ushort))
					return "ushort";
			} else if (type == typeof(string))
				return "string";
			return type.Name;
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

#region built-in operations on objects which expose types
		class TypeOps {
			Dictionary<string, Type> createableTypes;

			public TypeOps ()
			{
				createableTypes = new Dictionary<string, Type> ();
			}

			public void RegisterCreateableTypes (PropertyInfo pi)
			{
				RegisterCreateableType (pi.PropertyType);
			}

			public void RegisterCreateableTypes (MethodInfo mi)
			{
				if (IsCreateable (mi.ReturnType))
					RegisterCreateableType (mi.ReturnType);

				ParameterInfo[] ps = mi.GetParameters();
				foreach (ParameterInfo p in ps) {
					if (IsCreateable (p.ParameterType))
						RegisterCreateableType (p.ParameterType);
				}
			}

			public void RegisterCreateableType (Type type)
			{
				string name = ScriptName (type);
				if (!createableTypes.ContainsKey (name))
					createableTypes[name] = type;
			}

			public ScriptObject CreateManagedObject (string name, params object [] args)
			{
				if (!createableTypes.ContainsKey (name))
					return null;

				Type type = createableTypes[name];
				if (args != null)
					return HostServices.Current.CreateObject (type, args[0]);
				return HostServices.Current.CreateObject (type);
			}
		}
#endregion

#region built-in operations on objects which expose events
		class EventOps {
			ManagedObject obj;
			public EventOps (ManagedObject obj)
			{
				this.obj = obj;
			}

			public void AddEventListener (string eventName, ScriptObject handler)
			{
				EventInfo einfo = obj.ManagedObject.GetType ().GetEvent (eventName);

				if (einfo == null) {
					// this is silently ignored.
					return;
				}

				obj.AddEventHandler (eventName, handler, einfo);
			}

			public void RemoveEventListener (string eventName, ScriptObject handler)
			{
				obj.RemoveEventHandler (eventName, handler);
			}
		}
#endregion

#region built-in operations on collections
		class ListOps<T> {
			IList col;

			public ListOps (IList obj)
			{
				col = obj;
			}

			public T this[int index] {
				get { return (T) col[index]; }
				set { col[index] = value; }
			}

			public object Pop ()
			{
				if (col.IsFixedSize)
					throw new NotSupportedException ();
				object ret = col[col.Count - 1];
				col.RemoveAt (col.Count - 1);
				return ret;
			}

			public void Push (object value, params object[] valueN)
			{
				if (col.IsFixedSize)
					throw new NotSupportedException ();
				col.Add (value);
				if (valueN != null)
					foreach (object o in valueN)
						col.Add (o);
			}

			public void Reverse ()
			{
				int mid = col.Count / 2;
				for (int i = 0, j = col.Count-1; i < mid; i++, j--) {
					object o = col[i];
					col[i] = col[j];
					col[j] = o;
				}
			}

			public object Shift ()
			{
				if (col.IsFixedSize)
					throw new NotSupportedException ();
				object ret = col[0];
				col.RemoveAt (0);
				return ret;
			}

			public void Unshift (object value, params object[] valueN)
			{
				if (col.IsFixedSize)
					throw new NotSupportedException ();
				int i = 0;
				col.Insert (i++, value);
				if (valueN != null)
					foreach (object o in valueN)
						col.Insert (i++, o);
			}

			public void Splice (int startIndex, params object[] args)
			{
				if (col.IsFixedSize)
					throw new NotSupportedException ();
				double count = col.Count - startIndex;

				if (args != null && args.Length > 0)
					count = (double)args[0];

				for (; count > 0; count--)
					col.RemoveAt (startIndex);

				if (args != null && args.Length > 0)
					for (int i = 1; i < args.Length; i++)
						col.Insert (startIndex + i - 1, args[i]);
			}
		}

		class DictOps {
			IDictionary col;

			public DictOps (IDictionary obj)
			{
				col = obj;
			}

			public object this[string index] {
				get {
					if (col.Contains (index))
						return col[index];
					return null;
				}
				set { col[index] = value; }
			}
		}
#endregion
	}

}
