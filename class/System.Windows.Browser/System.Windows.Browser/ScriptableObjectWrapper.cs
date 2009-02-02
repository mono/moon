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

using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Browser;
using System.Runtime.InteropServices;
using System.Reflection;
using System.Globalization;
using Mono;

namespace System.Windows.Browser
{

	delegate void InvokeDelegate (IntPtr obj_handle, IntPtr method_handle,
				      [MarshalAs (UnmanagedType.LPArray, SizeParamIndex = 3)]
				      IntPtr[] args,
				      int arg_count,
				      ref Value return_value);

	delegate void SetPropertyDelegate (IntPtr obj_handle, IntPtr property_handle, ref Value value);
	delegate void GetPropertyDelegate (IntPtr obj_handle, IntPtr property_handle, ref Value value);
	delegate void EventHandlerDelegate (IntPtr obj_handle, IntPtr event_handle, IntPtr scriptable_obj, IntPtr closure);
	
	internal class ScriptableObjectWrapper : ScriptObject {

		List<GCHandle> handles; /* for methods, events, properties, as well as the object itself */
		IntPtr moon_handle;
		public IntPtr MoonHandle {
			get { return moon_handle; }
		}

		public ScriptableObjectWrapper (object obj) : base(obj)
		{
			this.handles = new List<GCHandle>();

			GCHandle obj_handle = GCHandle.Alloc (obj);

			handles.Add (obj_handle);

			moon_handle = ScriptableNativeMethods.wrapper_create_root (
									WebApplication.Current.PluginHandle,
									(IntPtr)obj_handle,
									invoke,
									set_prop,
									get_prop,
									add_event,
									remove_event);

			handle = ScriptableNativeMethods.moonlight_object_to_npobject (moon_handle);
		}

		public ScriptableObjectWrapper (object obj, IntPtr parent) : base (obj)
		{
			this.handles = new List<GCHandle>();

			GCHandle obj_handle = GCHandle.Alloc (obj);

			handles.Add (obj_handle);

			moon_handle = ScriptableNativeMethods.wrapper_create (parent,
										     (IntPtr)obj_handle,
										     invoke,
										     set_prop,
										     get_prop,
										     add_event,
										     remove_event);

			handle = ScriptableNativeMethods.moonlight_object_to_npobject (moon_handle);
		}

		public void Register (string scriptKey)
		{
			ScriptableNativeMethods.register (WebApplication.Current.PluginHandle, scriptKey, moon_handle);
		}

		public void AddProperty (PropertyInfo pi)
		{
			GCHandle prop_handle = GCHandle.Alloc (pi);

			handles.Add (prop_handle);

			TypeCode tc = Type.GetTypeCode (pi.PropertyType);

			ScriptableNativeMethods.add_property (WebApplication.Current.PluginHandle,
							      moon_handle,
							      (IntPtr)prop_handle,
							      pi.Name,
							      tc,
							      pi.CanRead,
							      pi.CanWrite);
		}

		public void AddEvent (EventInfo ei)
		{
 			GCHandle event_handle = GCHandle.Alloc (ei);

			handles.Add (event_handle);

 			ScriptableNativeMethods.add_event (WebApplication.Current.PluginHandle,
						           moon_handle,
 							   (IntPtr)event_handle,
							   ei.Name);
		}

		public void AddMethod (MethodInfo mi)
		{
			ParameterInfo[] ps = mi.GetParameters();
			TypeCode[] tcs = new TypeCode [ps.Length];

			foreach (ParameterInfo p in ps) {
				TypeCode pc = Type.GetTypeCode (p.ParameterType);
				tcs[p.Position] = pc;
			}

			GCHandle method_handle = GCHandle.Alloc (mi);

			handles.Add (method_handle);

			ScriptableNativeMethods.add_method (WebApplication.Current.PluginHandle,
							    moon_handle,
							    (IntPtr)method_handle,
							    mi.Name,
							    Type.GetTypeCode (mi.ReturnType),
							    tcs,
							    tcs.Length);
		}

		class EventDelegate {
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

				ScriptableNativeMethods.emit_event (WebApplication.Current.PluginHandle,
								    scriptable_handle,
								    event_wrapper.MoonHandle,
								    closure);
			}
		}


		static InvokeDelegate invoke = new InvokeDelegate (InvokeFromUnmanaged);
		static SetPropertyDelegate set_prop = new SetPropertyDelegate (SetPropertyFromUnmanaged);
		static GetPropertyDelegate get_prop = new GetPropertyDelegate (GetPropertyFromUnmanaged);
		static EventHandlerDelegate add_event = new EventHandlerDelegate (AddEventFromUnmanaged);
		static EventHandlerDelegate remove_event = new EventHandlerDelegate (RemoveEventFromUnmanaged);

		internal static object ObjectFromValue<T> (Value v)
		{
			switch (v.k) {
			case Kind.BOOL:
				return v.u.i32 != 0;
			case Kind.UINT64:
				return v.u.ui64;
			case Kind.INT32:
				return v.u.i32;
			case Kind.INT64:
				return v.u.i64;
			case Kind.DOUBLE:
				return v.u.d;
			case Kind.STRING:
				return Marshal.PtrToStringAnsi (v.u.p);
			case Kind.NPOBJ:
				// FIXME: Move all of this one caller up
				Type type = typeof (T);
				WeakReference reference;
				if (ScriptObject.ScriptableObjects.TryGetValue (v.u.p, out reference)) {
					return (T) reference.Target;
				} else if (!type.Equals (typeof(object)) && typeof (ScriptObject).IsAssignableFrom (type)) {
					System.Reflection.ConstructorInfo info = type.GetConstructor (BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.Instance, null, new Type[]{typeof(IntPtr)}, null);
					return (T) info.Invoke (new object[]{v.u.p});
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
				v.k = Kind.BOOL;
				v.u.i32 = ((bool) o) ? 1 : 0;
				break;
			case TypeCode.Double:
				v.k = Kind.DOUBLE;
				v.u.d = (double)o;
				break;
			case TypeCode.Int32:
				v.k = Kind.INT32;
				v.u.i32 = (int)o;
				break;
			case TypeCode.UInt32:
				v.k = Kind.UINT32;
				v.u.ui32 = (uint)o;
				break;
			case TypeCode.Int64:
				v.k = Kind.INT64;
				v.u.i64 = (long)o;
				break;
			case TypeCode.UInt64:
				v.k = Kind.UINT64;
				v.u.ui64 = (ulong)o;
				break;
			case TypeCode.String:
				v.k = Kind.STRING;
				byte[] bytes = System.Text.Encoding.UTF8.GetBytes (o as string);
				IntPtr result = Helper.AllocHGlobal (bytes.Length + 1);
				Marshal.Copy (bytes, 0, result, bytes.Length);
				Marshal.WriteByte (result, bytes.Length, 0);
				v.u.p = result;
				break;
			case TypeCode.Object:
				//Console.Write ("Trying to marshal managed object {0}...", o.GetType ().FullName);
				ScriptObject so = o as ScriptObject;
				if (so != null) {
					v.u.p = so.Handle;
					v.k = Kind.NPOBJ;
				} else {
					GCHandle handle = new GCHandle ();
					handle.Target = o;
					v.u.p = Helper.GCHandleToIntPtr (handle);
					v.k = Kind.MANAGED;
				}
				//Console.WriteLine ("  Marshalled as {0}", v.k);
				break;
			default:
				Console.WriteLine ("unsupported TypeCode.{0} = {1}", Type.GetTypeCode(o.GetType()), o.GetType ().FullName);
				throw new NotSupportedException ();
			}
		}

		static void InvokeFromUnmanaged (IntPtr obj_handle, IntPtr method_handle, IntPtr[] uargs, int arg_count, ref Value return_value)
		{
			object obj = ((GCHandle)obj_handle).Target;
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

			object rv = mi.Invoke (obj, BindingFlags.Default, new JSFriendlyMethodBinder(), margs, null);

			if (mi.ReturnType != typeof (void))
				ValueFromObject (ref return_value, rv);
		}

		static void SetPropertyFromUnmanaged (IntPtr obj_handle, IntPtr property_handle, ref Value value)
		{
			object obj = ((GCHandle)obj_handle).Target;
			PropertyInfo pi = (PropertyInfo)((GCHandle)property_handle).Target;

			object v = ObjectFromValue<object> (value);

			try {
				pi.SetValue (obj, v, null);
			} catch (Exception ex) {
				Console.WriteLine (ex);
			}
		}

		static void GetPropertyFromUnmanaged (IntPtr obj_handle, IntPtr property_handle, ref Value value)
		{
			object obj = ((GCHandle)obj_handle).Target;
			PropertyInfo pi = (PropertyInfo)((GCHandle)property_handle).Target;

			object v = pi.GetValue (obj, null);

			if (Type.GetTypeCode (pi.PropertyType) == TypeCode.Object) {
				v = ScriptableObjectGenerator.Generate (v, false); // the type has already been validated
			}

			ValueFromObject (ref value, v);
		}

		static void AddEventFromUnmanaged (IntPtr obj_handle, IntPtr event_handle, IntPtr scriptable_obj, IntPtr closure)
		{
			object obj = ((GCHandle)obj_handle).Target;
			EventInfo ei = (EventInfo)((GCHandle)event_handle).Target;

			ei.AddEventHandler (obj, new EventDelegate (ei.EventHandlerType, scriptable_obj, closure).Delegate);
		}

		static void RemoveEventFromUnmanaged (IntPtr obj_handle, IntPtr event_handle, IntPtr scriptable_obj, IntPtr closure)
		{
			EventInfo ei = (EventInfo)((GCHandle)event_handle).Target;

			Console.WriteLine ("TODO - RemoveEventFromUnmanaged");
			Console.WriteLine (" + {0}", ei.Name);
			Console.WriteLine (" + {0}", ei.DeclaringType.FullName);
			Console.WriteLine (" + {0}", ei.EventHandlerType);
		}
		
		class ScriptableNativeMethods {
			[DllImport ("moonplugin", EntryPoint = "moonlight_scriptable_object_wrapper_create_root")]
			public static extern IntPtr wrapper_create_root (IntPtr plugin, IntPtr obj_handle,
								    InvokeDelegate invoke,
								    SetPropertyDelegate set_prop,
								    GetPropertyDelegate get_prop,
								    EventHandlerDelegate add_event,
								    EventHandlerDelegate remove_event);

			[DllImport ("moonplugin", EntryPoint = "moonlight_scriptable_object_wrapper_create")]
			public static extern IntPtr wrapper_create (IntPtr parent_handle, IntPtr obj_handle,
								    InvokeDelegate invoke,
								    SetPropertyDelegate set_prop,
								    GetPropertyDelegate get_prop,
								    EventHandlerDelegate add_event,
								    EventHandlerDelegate remove_event);

			[DllImport ("moonplugin", EntryPoint = "moonlight_scriptable_object_add_property")]
			public static extern void add_property (IntPtr plugin_handle,
								IntPtr wrapper,
								IntPtr property_handle,
								string property_name,
								TypeCode property_type,
								bool readable,
								bool writable);

			[DllImport ("moonplugin", EntryPoint = "moonlight_scriptable_object_add_event")]
			public static extern void add_event (IntPtr plugin_handle,
							     IntPtr wrapper,
							     IntPtr event_handle,
							     string event_name);

			[DllImport ("moonplugin", EntryPoint = "moonlight_scriptable_object_add_method")]
			public static extern void add_method (IntPtr plugin_handle,
							      IntPtr wrapper,
							      IntPtr method_handle,
							      string method_name,
							      TypeCode method_return_type,
							      TypeCode[] method_parameter_types,
							      int parameter_count);

			[DllImport ("moonplugin", EntryPoint = "moonlight_scriptable_object_emit_event")]
			public static extern void emit_event (IntPtr plugin_handle,
							      IntPtr scriptable_obj,
							      IntPtr event_wrapper,
							      IntPtr closure);

			[DllImport ("moonplugin", EntryPoint = "moonlight_scriptable_object_register")]
			public static extern void register (IntPtr plugin_handle,
							    string name,
							    IntPtr wrapper);

			[DllImport ("moonplugin", EntryPoint = "moonlight_object_to_npobject")]
			public static extern IntPtr moonlight_object_to_npobject (IntPtr obj);

			[DllImport ("moonplugin", EntryPoint = "npobject_to_moonlight_object")]
			public static extern IntPtr npobject_to_moonlight_object (IntPtr obj);
			
		}
		
	}
}
