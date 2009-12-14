// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2008 Novell, Inc (http://www.novell.com)
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

using System.ComponentModel;
using System.Globalization;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using System.Reflection;
using System.Windows.Interop;
using Mono;

namespace System.Windows.Browser
{
	internal sealed class ScriptableObjectWrapper : ScriptObject {

		static InvokeDelegate invoke = new InvokeDelegate (InvokeFromUnmanagedSafe);
		static SetPropertyDelegate set_prop = new SetPropertyDelegate (SetPropertyFromUnmanagedSafe);
		static GetPropertyDelegate get_prop = new GetPropertyDelegate (GetPropertyFromUnmanagedSafe);
		static EventHandlerDelegate add_event = new EventHandlerDelegate (AddEventFromUnmanagedSafe);
		static EventHandlerDelegate remove_event = new EventHandlerDelegate (RemoveEventFromUnmanagedSafe);

		Dictionary<IntPtr, Delegate> events;
		Dictionary<string, List<MethodInfo>> methods;
		Dictionary<string, PropertyInfo> properties;
		Dictionary<string, List<ScriptableObjectEventInfo>> event_handlers;
		
		GCHandle obj_handle;
		IntPtr moon_handle;
		public IntPtr MoonHandle {
			get { return moon_handle; }
		}

		public bool HasTypes { get; set; }

		static ScriptableObjectWrapper ()
		{
		}

		public ScriptableObjectWrapper () : this(null)
		{
		}

		public ScriptableObjectWrapper (object obj) : this(obj, IntPtr.Zero)
		{
		}

		public ScriptableObjectWrapper (object obj, IntPtr parent) : base (obj)
		{
			this.events = new Dictionary<IntPtr, Delegate> ();
			this.methods = new Dictionary<string, List<MethodInfo>> ();
			this.properties = new Dictionary<string, PropertyInfo> ();

			obj_handle = GCHandle.Alloc (this);
			if (parent == IntPtr.Zero) {
				moon_handle = NativeMethods.moonlight_scriptable_object_wrapper_create_root (
							PluginHost.Handle,
							(IntPtr) obj_handle,
							invoke,
							set_prop,
							get_prop,
							add_event,
							remove_event);
			} else {
				moon_handle = NativeMethods.moonlight_scriptable_object_wrapper_create (parent,
							(IntPtr) obj_handle,
							invoke,
							set_prop,
							get_prop,
							add_event,
							remove_event);
			}
			Handle = NativeMethods.moonlight_object_to_npobject (moon_handle);
			HtmlPage.CachedObjects [Handle] = new WeakReference (this);
			
			AddManualHooks (obj);
		}

		public void Register (string scriptKey)
		{
			NativeMethods.moonlight_scriptable_object_register (PluginHost.Handle, scriptKey, moon_handle);
		}

		public static IntPtr MoonToNPObj (IntPtr ptr)
		{
			return NativeMethods.moonlight_object_to_npobject (ptr);
		}

		public void AddProperty (PropertyInfo pi)
		{
			string name = pi.Name;
			if (pi.IsDefined (typeof(ScriptableMemberAttribute), false)) {
			    ScriptableMemberAttribute att = (ScriptableMemberAttribute) pi.GetCustomAttributes (typeof (ScriptableMemberAttribute), false)[0];
				name = (att.ScriptAlias ?? name);
			}
			AddProperty (pi, name);
		}

		public void AddProperty (PropertyInfo pi, string name)
		{
			TypeCode tc = Type.GetTypeCode (pi.PropertyType);
			properties[name] = pi;
			NativeMethods.moonlight_scriptable_object_add_property (PluginHost.Handle,
									moon_handle,
									IntPtr.Zero,
									name,
									tc,
									pi.CanRead,
									pi.CanWrite);
		}

		public void AddEvent (EventInfo ei)
		{
			GCHandle event_handle = GCHandle.Alloc (ei);

			string name = ei.Name;
			if (ei.IsDefined (typeof(ScriptableMemberAttribute), false)) {
			    ScriptableMemberAttribute att = (ScriptableMemberAttribute) ei.GetCustomAttributes (typeof (ScriptableMemberAttribute), false)[0];
				name = (att.ScriptAlias ?? name);
			}
			NativeMethods.moonlight_scriptable_object_add_event (PluginHost.Handle,
									moon_handle,
									(IntPtr)event_handle,
									name);
		}

		public void AddMethod (string name, TypeCode[] args, TypeCode ret_type)
		{
			NativeMethods.moonlight_scriptable_object_add_method (
								PluginHost.Handle,
								moon_handle,
								IntPtr.Zero,
								name,
								ret_type,
								args,
								args.Length);
		}

		public void AddManualHooks (object o)
		{
			TypeCode[] tcs = new TypeCode [] {TypeCode.String, TypeCode.Object};
			AddMethod ("addEventListener", tcs, TypeCode.Empty);
			AddMethod ("removeEventListener", tcs, TypeCode.Empty);
			
			// TODO: constructor and createManagedObject
			if (o is System.Collections.ICollection) {
				AddProperty (o.GetType ().GetProperty ("Count"), "length");
			}
		}
		
		public void AddMethod (MethodInfo mi)
		{
			ParameterInfo[] ps = mi.GetParameters();
			TypeCode[] tcs = new TypeCode [ps.Length];

			foreach (ParameterInfo p in ps) {
				TypeCode pc = Type.GetTypeCode (p.ParameterType);
				tcs[p.Position] = pc;
			}

			string name = mi.Name;
			if (mi.IsDefined (typeof(ScriptableMemberAttribute), false)) {
			    ScriptableMemberAttribute att = (ScriptableMemberAttribute) mi.GetCustomAttributes (typeof (ScriptableMemberAttribute), false)[0];
				name = (att.ScriptAlias ?? name);
			}

			if (!methods.ContainsKey (name)) {
				methods[name] = new List<MethodInfo>();
			}
			methods[name].Add (mi);

			NativeMethods.moonlight_scriptable_object_add_method (PluginHost.Handle,
								moon_handle,
								IntPtr.Zero,
								name,
								mi.ReturnType == typeof(void) ? 0 : Type.GetTypeCode (mi.ReturnType),
								tcs,
								tcs.Length);
		}

		[ScriptableMember(ScriptAlias="createObject")]
		public ScriptableObjectWrapper CreateObject (string name)
		{
			if (!HtmlPage.ScriptableTypes.ContainsKey (name))
				return null;

			object o = Activator.CreateInstance (HtmlPage.ScriptableTypes[name]);
			return ScriptableObjectGenerator.Generate (o, false);
		}

		internal static T CreateInstance<T> (IntPtr ptr)
		{
			ConstructorInfo i = typeof(T).GetConstructor (BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.Instance,
						null, new Type[]{typeof(IntPtr)}, null);

			object o = i.Invoke (new object[]{ptr});
			HtmlPage.CachedObjects[ptr] = o;
			return (T) o;
		}

		internal static object ObjectFromValue<T> (Value v)
		{
			// When the target type is object, SL converts ints to doubles to wash out
			// browser differences. (Safari apparently always returns doubles, FF
			// ints and doubles, depending on the value).
			// See: http://msdn.microsoft.com/en-us/library/cc645079(VS.95).aspx

			Type type = typeof (T);
			bool isobject = type.Equals (typeof(object));

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
				else if (type.IsAssignableFrom (typeof (Int32)))
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

				object reference;
				if (HtmlPage.CachedObjects.TryGetValue (v.u.p, out reference)) {
					WeakReference wr = (reference as WeakReference);
					if (wr != null) {
						if (wr.IsAlive)
							return (T) wr.Target;
						else
							HtmlPage.CachedObjects.Remove (v.u.p);
					} else {
						return (T) reference;
					}
				}

				if (!type.Equals (typeof(object)) && typeof (ScriptObject).IsAssignableFrom (type)) {
					return CreateInstance<T> (v.u.p);
				} else if (type.Equals (typeof(object))) {
					if (NativeMethods.html_object_has_property (PluginHost.Handle, v.u.p, "nodeType")) {
						Value val;
						NativeMethods.html_object_get_property (PluginHost.Handle, v.u.p, "nodeType", out val);

						if (v.u.i32 == 9) // HtmlDocument
							return CreateInstance<HtmlDocument> (v.u.p);
						else if (v.u.i32 == 1) //HtmlElement
							return CreateInstance<HtmlElement> (v.u.p);
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

		internal static void ValueFromObject (ref Value v, object o)
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
				//Console.Write ("Trying to marshal managed object {0}...", o.GetType ().FullName);
				ScriptObject so = o as ScriptObject;
				if (so != null) {
					v.u.p = so.Handle;
					v.k = Kind.NPOBJ;
				} else if (ScriptableObjectGenerator.ValidateType (o.GetType())) {
					ScriptableObjectWrapper wrapper = ScriptableObjectGenerator.Generate (o, false);
					v.u.p = wrapper.Handle;
					v.k = Kind.NPOBJ;
				} else {
					// This should create an object of type MANAGED
					v = Value.FromObject (o);
				}
				//Console.WriteLine ("  Marshalled as {0}", v.k);
				break;
			default:
				Console.WriteLine ("unsupported TypeCode.{0} = {1}", Type.GetTypeCode(o.GetType()), o.GetType ().FullName);
				throw new NotSupportedException ();
			}
		}

#region Methods

		bool ValidateArguments (MethodInfo mi, object[] args)
		{
			if (mi.GetParameters().Length != args.Length)
				return false;

			// TODO: refactor this, the js binder is doing this work already
			ParameterInfo[] parms = mi.GetParameters ();
			for (int i = 0; i < parms.Length; i++) {
				if (args[i] == null)
					continue;

				Type cstype = parms[i].ParameterType;
				JSFriendlyMethodBinder binder = new JSFriendlyMethodBinder ();
				object ret;
				if (binder.TryChangeType (args[i], cstype, CultureInfo.CurrentUICulture, out ret))
					continue;

				Type jstype = args[i].GetType();
				if (jstype != cstype && Type.GetTypeCode (jstype) != Type.GetTypeCode (cstype)) {
					switch (Type.GetTypeCode (jstype)) {
						case TypeCode.Int32:
							if (cstype.IsPrimitive || (cstype == typeof(object)))
								continue;
							break;
						case TypeCode.String:
							if (cstype == typeof(char) || cstype == typeof(object) || cstype == typeof(Guid))
								continue;
							break;
					}
					return false;
				}
			}
			return true;
		}

		void Invoke (string name, object[] args, ref Value ret)
		{
			if (methods.ContainsKey (name)) {
				foreach (MethodInfo mi in methods[name]) {
					if (ValidateArguments (mi, args)) {
						Invoke (mi, args, ref ret);
						return;
					}
				}
			}

			switch (name.ToLower ()) {
			case "addeventlistener": {
				ScriptableObjectEventInfo ei = new  ScriptableObjectEventInfo ();
				ei.Name = (string) args [0];
				ei.Callback = (ScriptObject) args [1];
				ei.EventInfo = ManagedObject.GetType ().GetEvent (ei.Name);
				
				if (ei.EventInfo == null) {
					// this is silently ignored.
					return;
				}
				
				List<ScriptableObjectEventInfo> list;
				if (event_handlers == null)
					event_handlers = new Dictionary<string, List<ScriptableObjectEventInfo>>();
				if (!event_handlers.TryGetValue (ei.Name, out list)) {
					list = new List<ScriptableObjectEventInfo> ();
					event_handlers.Add (ei.Name, list);
				}
				list.Add (ei);
				
				ei.EventInfo.AddEventHandler (ManagedObject, ei.GetDelegate ());	
				
				break;
			}
			case "removeeventlistener": {
				string event_name = (string) args [0];
				ScriptObject scriptobject = (ScriptObject) args [1];

				List<ScriptableObjectEventInfo> list;
				if (!event_handlers.TryGetValue (event_name, out list)) {
					// TODO: throw exception?
					Console.WriteLine ("ScriptableObjectWrapper.Invoke ('removeEventListener'): There are no event listeners registered for '{0}'", event_name);
					return;
				}
			
				for (int i = list.Count - 1; i >= 0; i--) {
					if (list [i].Callback == scriptobject) {
						ScriptableObjectEventInfo ei = list [i];
						ei.EventInfo.RemoveEventHandler (ManagedObject, ei.GetDelegate ());
						list.RemoveAt (i);
						return;
					}
				}
				
				// TODO: throw exception?
				Console.WriteLine ("ScriptableObjectWrapper.Invoke ('removeEventListener'): Could not find the specified listener in the list of registered listeners for '{0}'", event_name);
				
				break;
			}
			case "createmanagedobject":
				if (args.Length == 1) {
					ScriptableObjectWrapper wrapper = CreateObject ((string)args[0]);
					ValueFromObject (ref ret, wrapper);
				}
				break;
			case "constructor":
			default:
				Console.WriteLine ("ScriptableObjectWrapper.Invoke: NOT IMPLEMENTED: {0}", name.ToLower ());
				break;
			}
		}

		void Invoke (MethodInfo mi, object[] args, ref Value ret)
		{
			object rv = mi.Invoke (this.ManagedObject, BindingFlags.Default, new JSFriendlyMethodBinder (), args, null);
			if (mi.ReturnType != typeof (void))
				ValueFromObject (ref ret, rv);
		}

		static void InvokeFromUnmanagedSafe (IntPtr obj_handle, IntPtr method_handle, string name, IntPtr[] uargs, int arg_count, ref Value return_value)
		{
			try {
				InvokeFromUnmanaged (obj_handle, method_handle, name, uargs, arg_count, ref return_value);
			} catch (Exception ex) {
				try {
					Console.WriteLine ("Moonlight: Unhandled exception in ScriptableObjectWrapper.InvokeFromUnmanagedSafe: {0}", ex);
				} catch {
				}
			}
		}

		static void InvokeFromUnmanaged (IntPtr obj_handle, IntPtr method_handle, string name, IntPtr[] uargs, int arg_count, ref Value return_value)
		{
			//Console.WriteLine ("Invoke " + name);

			ScriptableObjectWrapper obj = (ScriptableObjectWrapper) ((GCHandle)obj_handle).Target;
			object[] args = new object[arg_count];
			for (int i = 0; i < arg_count; i++) {
				if (uargs[i] == IntPtr.Zero) {
					args[i] = null;
				}
				else {
					Value v = (Value)Marshal.PtrToStructure (uargs[i], typeof (Value));
					args[i] = ObjectFromValue<object> (v);
				}
			}

			if (method_handle == IntPtr.Zero) {
				obj.Invoke (name, args, ref return_value);
			} else {
				throw new Exception ("Invalid method invoke");
/*
				MethodInfo mi = (MethodInfo)((GCHandle)method_handle).Target;

				object[] margs = new object[arg_count];
				ParameterInfo[] pis = mi.GetParameters ();

				//Console.WriteLine ("arg_count = {0}", arg_count);
				for (int i = 0; i < arg_count; i ++) {
					Value v = (Value)Marshal.PtrToStructure (uargs[i], typeof (Value));

					object o = ObjectFromValue<object> (v);
					//Console.WriteLine ("margs[{1}] = {2} ({0})", o.GetType(), i, o);

					margs[i] = o; //Convert.ChangeType (o, pis[i].ParameterType);
				}

				obj.Invoke (mi, margs, ref return_value);
*/
			}
		}

#endregion

#region Properties

		public override void SetProperty (string name, object value)
		{
			if (ManagedObject != null) {
				PropertyInfo pi = properties[name];
				pi.SetValue (ManagedObject, value, BindingFlags.SetProperty, new JSFriendlyMethodBinder (), null, CultureInfo.InvariantCulture);
			} else {
				base.SetProperty (name, value);
			}
		}

		static void SetPropertyFromUnmanagedSafe (IntPtr obj_handle, string name, ref Value value)
		{
			try {
				SetPropertyFromUnmanaged (obj_handle, name, ref value);
			} catch (Exception ex) {
				try {
					Console.WriteLine ("Moonlight: Unhandled exception in ScriptableObjectWrapper.SetPropertyFromUnmanagedSafe: {0}", ex);
				} catch {
				}
			}
		}
		
		static void SetPropertyFromUnmanaged (IntPtr obj_handle, string name, ref Value value)
		{
			ScriptableObjectWrapper obj = (ScriptableObjectWrapper) ((GCHandle)obj_handle).Target;
			object v = ObjectFromValue<object> (value);
			obj.SetProperty (name, v);
		}

		public override object GetProperty (string name)
		{
			if (ManagedObject != null) {
				PropertyInfo pi = properties[name];
				return pi.GetValue (ManagedObject, null);
			} else {
				return base.GetProperty (name);
			}
		}

		static void GetPropertyFromUnmanagedSafe (IntPtr obj_handle, string name, ref Value value)
		{
			try {
				GetPropertyFromUnmanaged (obj_handle, name, ref value);
			} catch (Exception ex) {
				try {
					Console.WriteLine ("Moonlight: Unhandled exception in ScriptableObjectWrapper.GetPropertyFromUnmanagedSafe: {0}", ex);
				} catch {
				}
			}
		}
		
		static void GetPropertyFromUnmanaged (IntPtr obj_handle, string name, ref Value value)
		{
			ScriptableObjectWrapper obj = (ScriptableObjectWrapper) ((GCHandle)obj_handle).Target;
			object v = obj.GetProperty (name);

			if (Type.GetTypeCode (v.GetType ()) == TypeCode.Object) {
				v = ScriptableObjectGenerator.Generate (v, false); // the type has already been validated
			}

			ValueFromObject (ref value, v);
		}

#endregion

#region Events
		void AddEvent (EventInfo ei, IntPtr scriptable_handle, IntPtr closure)
		{
			Delegate d = new EventDelegate (ei.EventHandlerType, scriptable_handle, closure).Delegate;
			ei.AddEventHandler (this.ManagedObject, d);
			if (!this.events.ContainsKey (closure))
				this.events[closure] = d;
		}

		static void AddEventFromUnmanagedSafe (IntPtr obj_handle, IntPtr event_handle, IntPtr scriptable_obj, IntPtr closure)
		{
			try {
				AddEventFromUnmanaged (obj_handle, event_handle, scriptable_obj, closure);
			} catch (Exception ex) {
				try {
					Console.WriteLine ("Moonlight: Unhandled exception in ScriptableObjectWrapper.AddEventFromUnmanagedSafe: {0}", ex);
				} catch {
				}
			}
		}
		
		static void AddEventFromUnmanaged (IntPtr obj_handle, IntPtr event_handle, IntPtr scriptable_obj, IntPtr closure)
		{
			ScriptableObjectWrapper obj = (ScriptableObjectWrapper) ((GCHandle)obj_handle).Target;
			EventInfo ei = (EventInfo)((GCHandle)event_handle).Target;
			obj.AddEvent (ei, scriptable_obj, closure);
		}

		void RemoveEvent (EventInfo ei, IntPtr closure)
		{
			Delegate d = this.events[closure];
			ei.RemoveEventHandler (this.ManagedObject, d);
			events.Remove (closure);
		}

		static void RemoveEventFromUnmanagedSafe (IntPtr obj_handle, IntPtr event_handle, IntPtr scriptable_obj, IntPtr closure)
		{
			try {
				RemoveEventFromUnmanaged (obj_handle, event_handle, scriptable_obj, closure);
			} catch (Exception ex) {
				try {
					Console.WriteLine ("Moonlight: Unhandled exception in ScriptableObjectWrapper.RemoveEventFromUnmanagedSafe: {0}", ex);
				} catch {
				}
			}
		}
		
		static void RemoveEventFromUnmanaged (IntPtr obj_handle, IntPtr event_handle, IntPtr scriptable_obj, IntPtr closure)
		{
			ScriptableObjectWrapper obj = (ScriptableObjectWrapper) ((GCHandle)obj_handle).Target;
			EventInfo ei = (EventInfo)((GCHandle)event_handle).Target;

			obj.RemoveEvent (ei, closure);
		}

		sealed class EventDelegate {
			public EventDelegate (Type event_handler_type, IntPtr scriptable_handle, IntPtr closure)
			{
				this.event_handler_type = event_handler_type;
				this.scriptable_handle = scriptable_handle;
				this.closure = closure;
			}

			Type event_handler_type;
			IntPtr scriptable_handle;
			IntPtr closure;

			public Delegate Delegate {
				get {
					return Delegate.CreateDelegate (event_handler_type, this, GetType().GetMethod ("del"));
				}
			}

			public void del (object sender, object args)
			{
				// don't need to validate the type
				// again, this was done when the class
				// containing the event was validated.
				ScriptableObjectWrapper event_wrapper = ScriptableObjectGenerator.Generate (args, false);

				//Console.WriteLine ("emitting scriptable event!");

				NativeMethods.moonlight_scriptable_object_emit_event (PluginHost.Handle,
								    scriptable_handle,
								    event_wrapper.MoonHandle,
								    closure);
			}
		}

#endregion
	}
}
