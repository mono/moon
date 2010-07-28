//
// System.Windows.Browser.ScriptObject class
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

using System.Collections;
using System.Globalization;
using System.ComponentModel;
using System.Windows.Interop;
using System.Windows.Threading;
using System.Collections.Generic;
using System.Reflection;
using Mono;
using System.Runtime.InteropServices;

namespace System.Windows.Browser {

	class Method {
		public MethodInfo method;
		public object obj;
	}

	class Property {
		public PropertyInfo property;
		public object obj;
	}

	public class ScriptObject {
		IntPtr _handle;
		bool free_mapping;
		bool handleIsScriptableNPObject;

		internal Dictionary<string, EventInfo> events = new Dictionary<string, EventInfo> ();
		internal Dictionary<string, List<Method>> methods = new Dictionary<string, List<Method>> ();
		internal Dictionary<string, Property> properties = new Dictionary<string, Property> ();
		internal Dictionary<string, List<ScriptObjectEventInfo>> event_handlers;
		
		internal bool HasTypes { get; set; }
		internal bool HasEvents { get; set; }

		static ScriptObject ()
		{
			toggleRefs = new Dictionary<IntPtr, ScriptObjectToggleRef> ();
			cachedObjects = new Dictionary<IntPtr, WeakReference> ();
		}

		internal ScriptObject ()
		{
			// we do not want a [SecuritySafeCritical] attribute on the default ctor
			// since it mess with coreclr inheritance rules
			SetDefaultHandle ();
		}

		internal ScriptObject (IntPtr handle) : this (handle, false)
		{
		}

		internal ScriptObject (IntPtr handle, bool handleIsScriptableNPObject)
		{
			this.handleIsScriptableNPObject = handleIsScriptableNPObject;
			this.Handle = handle;
		}

		void SetDefaultHandle ()
		{
			if (PluginHost.Handle != IntPtr.Zero) {
				handleIsScriptableNPObject = true;
				Handle = NativeMethods.moonlight_scriptable_object_create (PluginHost.Handle, invalidate_handle,
											   has_method, has_property, invoke,
											   set_prop, get_prop);
			} else {
				Handle = IntPtr.Zero;
			}
		}

		internal IntPtr Handle {
			get {
				return _handle;
			}
			set {
				if (_handle != IntPtr.Zero)
					throw new InvalidOperationException ("ScriptObject.Handle is write-once.");

				_handle = value;

				if (handleIsScriptableNPObject)
					free_mapping = AddNativeMapping (value, this);
				lock (cachedObjects) {
					cachedObjects[value] = new WeakReference (this);
				}
			}
		}

		~ScriptObject ()
		{
			Free ();
		}

		internal void Free ()
		{
			if (free_mapping)
				FreeNativeMapping (this);
			lock (cachedObjects)
				cachedObjects.Remove (_handle);
		}

		public virtual void SetProperty (string name, object value)
		{
			CheckHandle ();
			SetPropertyInternal (Handle, name, value);
		}

		internal virtual void SetProperty (string name, object[] args)
		{
			SetProperty (name, args[0]);
		}

		public void SetProperty (int index, object value)
		{
			SetProperty (index.ToString(), value);
		}

		public virtual object GetProperty (string name)
		{
			CheckHandle ();
			return GetPropertyInternal <object> (Handle, name);
		}

		public object GetProperty (int index)
		{
			CheckHandle ();
			// XXX this is likely wrong, as it needs to call into the plugin using an ordinal, not a string - toshok
			// XXX SL does the same half-assed conversion, so shrugging and keeping this one as-is - shana
			return GetPropertyInternal <object> (Handle, index.ToString ());
		}

		protected virtual object ConvertTo (Type targetType, bool allowSerialization)
		{
			if (targetType.IsAssignableFrom (GetType()))
				return this;

			if (allowSerialization)
				return HostServices.Services.JsonDeserialize (this, targetType);

			return null;
		}
		
		public T ConvertTo<T> ()
		{
			return (T) ConvertTo (typeof(T), true);
		}

		public virtual object Invoke (string name, params object [] args)
		{
			return InvokeInternal <object> (name, args);
		}

		public virtual object InvokeSelf (params object [] args)
		{
			return InvokeInternal <object> (args);
		}

		protected void Initialize (IntPtr handle, IntPtr identity, bool addReference, bool releaseReferenceOnDispose)
		{
			throw new System.NotImplementedException ();
		}

		[EditorBrowsable (EditorBrowsableState.Never)]
		public bool CheckAccess ()
		{
			return Dispatcher.Main.CheckAccess ();
		}

		internal virtual object ManagedObjectCore {
			get { return null; }
		}

		public object ManagedObject {
			get { return ManagedObjectCore; }
		}

		[EditorBrowsable (EditorBrowsableState.Advanced)]
		public Dispatcher Dispatcher {
			get { return Dispatcher.Main; }
		}

		internal static void CheckName (string name)
		{
			if (name == null)
				throw new ArgumentNullException ("name");
			if ((name.Length == 0) || name.IndexOf ((char) 0) != -1)
				throw new ArgumentException ("name");
		}

		internal void CheckHandle ()
		{
			if (_handle == IntPtr.Zero)
				throw new InvalidOperationException ("the NPObject has been invalidated");
		}

		internal static T GetPropertyInternal<T> (IntPtr h, string name)
		{
			T result;

			CheckName (name);
			Mono.Value res;
			NativeMethods.html_object_get_property (PluginHost.Handle, h, name, out res);
			if (res.k != Mono.Kind.INVALID)
				result = (T)ScriptObjectHelper.ObjectFromValue<T> (res);
			else
				result = default (T);

			NativeMethods.value_free_value (ref res);

			return result;
		}

		internal T GetPropertyInternal<T> (string name)
		{
			CheckHandle ();
			return GetPropertyInternal<T> (Handle, name);
		}

		internal static void SetPropertyInternal (IntPtr h, string name, object value)
		{
			CheckName (name);
			Mono.Value dp = new Mono.Value ();
			ScriptObjectHelper.ValueFromObject (ref dp, value);
			NativeMethods.html_object_set_property (PluginHost.Handle, h, name, ref dp);

			NativeMethods.value_free_value (ref dp);
		}

		internal void SetPropertyInternal (string name, object value)
		{
			CheckHandle ();
			SetPropertyInternal (Handle, name, value);
		}

		internal T InvokeInternal<T> (string name, params object [] args)
		{
			CheckName (name);
			CheckHandle ();
			int length = args == null ? 0 : args.Length;
			T result;
			Mono.Value res;
			Mono.Value [] vargs = new Mono.Value [length];

			for (int i = 0; i < length; i++)
				ScriptObjectHelper.ValueFromObject (ref vargs [i], args [i]);

			if (!NativeMethods.html_object_invoke (PluginHost.Handle, Handle, name, vargs, (uint) length, out res))
				throw new InvalidOperationException ();

			if (res.k != Mono.Kind.INVALID)
				result = (T)ScriptObjectHelper.ObjectFromValue<T> (res);
			else
				result = default (T);

			NativeMethods.value_free_value (ref res);

			return result;
		}

		internal T InvokeInternal<T> (params object [] args)
		{
			CheckHandle ();
			int length = args == null ? 0 : args.Length;
			T result;
			Mono.Value res;
			Mono.Value [] vargs = new Mono.Value [length];

			for (int i = 0; i < length; i++)
				ScriptObjectHelper.ValueFromObject (ref vargs [i], args [i]);

			if (!NativeMethods.html_object_invoke_self (PluginHost.Handle, Handle, vargs, (uint)length, out res))
				throw new InvalidOperationException ();

			if (res.k != Mono.Kind.INVALID)
				result = (T)ScriptObjectHelper.ObjectFromValue<T> (res);
			else
				result = default (T);

			NativeMethods.value_free_value (ref res);

			return result;
		}


#region Properties
		internal void RegisterScriptableProperty (PropertyInfo pi)
		{
			string scriptAlias = pi.Name;
			if (pi.IsDefined (typeof(ScriptableMemberAttribute), false)) {
			    ScriptableMemberAttribute att = (ScriptableMemberAttribute) pi.GetCustomAttributes (typeof (ScriptableMemberAttribute), false)[0];
			    scriptAlias = (att.ScriptAlias ?? scriptAlias);
			}
			RegisterScriptableProperty (pi, scriptAlias);
		}

		internal void RegisterScriptableProperty (PropertyInfo pi, string scriptAlias)
		{
			RegisterScriptableProperty (pi, scriptAlias, ManagedObject);
		}

		internal void RegisterScriptableProperty (PropertyInfo pi, string scriptAlias, object target)
		{
			properties[scriptAlias] = new Property () {property = pi, obj = target};
		}

		private bool HasProperty (string scriptAlias)
		{
			if (ManagedObject is IDictionary) {
				return true;
			}
			return properties.ContainsKey (scriptAlias) || events.ContainsKey (scriptAlias);
		}

		private bool SetPropertyFromUnmanaged (string scriptAlias, IntPtr[] uargs, int arg_count, ref Value value)
		{
			if (properties.ContainsKey (scriptAlias)) {
				object[] args = new object[arg_count + 1];
				for (int i = 0; i < arg_count; i++) {
					if (uargs[i] == IntPtr.Zero) {
						args[i] = null;
					} else {
						Value val = (Value)Marshal.PtrToStructure (uargs[i], typeof (Value));
						args[i] = ScriptObjectHelper.ObjectFromValue<object> (val);
					}
				}

				args[arg_count] = ScriptObjectHelper.ObjectFromValue<object> (value);
				SetProperty (scriptAlias, args);

				return true;
			}
			else if (events.ContainsKey (scriptAlias)) {
				if (arg_count != 0)
					throw new InvalidOperationException ("arg count != 0");

				EventInfo einfo = events[scriptAlias];

				if (value.k != Kind.NPOBJ)
					throw new InvalidOperationException ("html bridge only allows function objects as event delegates");

				ScriptObject event_object = (ScriptObject)ScriptObjectHelper.ObjectFromValue<ScriptObject> (value);

				ScriptObjectEventInfo ei = new ScriptObjectEventInfo (scriptAlias, event_object, einfo);

				AddEventHandler (ei);

				return true;
			}
			else
				return false;
		}


		internal object GetProperty (string scriptAlias, object[] args)
		{
			PropertyInfo pi = properties[scriptAlias].property;
			object obj = properties[scriptAlias].obj;
			if (pi.GetIndexParameters().Length > 0) {
				MethodInfo mi = pi.GetGetMethod ();
				if (ValidateArguments (mi, args))
					return Invoke (mi, obj, args);
				else
					throw new ArgumentException ("args");
			}
			return pi.GetValue (obj, null);
		}

		private bool GetPropertyFromUnmanaged (string scriptAlias, IntPtr[] uargs, int arg_count, ref Value value)
		{
			object[] args;
			if (ManagedObject is IDictionary) {
				args = new object[]{scriptAlias};
				scriptAlias = "item";
			} else {
				args = new object[arg_count];
				for (int i = 0; i < arg_count; i++) {
					if (uargs[i] == IntPtr.Zero) {
						args[i] = null;
					} else {
						Value val = (Value)Marshal.PtrToStructure (uargs[i], typeof (Value));
						args[i] = ScriptObjectHelper.ObjectFromValue<object> (val);
					}
				}
			}

			object v = GetProperty (scriptAlias, args);

			if (v != null && Type.GetTypeCode (v.GetType ()) == TypeCode.Object)
				v = new ManagedObject (v);

			ScriptObjectHelper.ValueFromObject (ref value, v);

			return true;
		}
#endregion

#region Events
		internal void RegisterScriptableEvent (EventInfo ei)
		{
			string scriptAlias = ei.Name;
			if (ei.IsDefined (typeof(ScriptableMemberAttribute), false)) {
			    ScriptableMemberAttribute att = (ScriptableMemberAttribute) ei.GetCustomAttributes (typeof (ScriptableMemberAttribute), false)[0];
			    scriptAlias = (att.ScriptAlias ?? scriptAlias);
			}
			RegisterScriptableEvent (ei, scriptAlias);
		}

		internal void RegisterScriptableEvent (EventInfo ei, string scriptAlias)
		{
			events[scriptAlias] = ei;
		}

		internal void AddEventHandler (ScriptObjectEventInfo ei)
		{
			List<ScriptObjectEventInfo> list;
			if (event_handlers == null)
				event_handlers = new Dictionary<string, List<ScriptObjectEventInfo>>();
			if (!event_handlers.TryGetValue (ei.Name, out list)) {
				list = new List<ScriptObjectEventInfo> ();
				event_handlers.Add (ei.Name, list);
			}
			list.Add (ei);
				
			ei.EventInfo.AddEventHandler (ManagedObject, ei.GetDelegate ());
		}
#endregion

#region Methods
		static List<string> reservedNames = new List<string>() {"addEventListener", "removeEventListener", "createManagedObject", "constructor"};

		internal void RegisterBuiltinScriptableMethod (MethodInfo mi, string scriptAlias, object target)
		{
			if (!methods.ContainsKey (scriptAlias)) {
				methods[scriptAlias] = new List<Method>();
			}

			methods[scriptAlias].Add (new Method { method = mi, obj = target });
		}

		internal void RegisterScriptableMethod (MethodInfo mi, string scriptAlias)
		{
			RegisterScriptableMethod (mi, scriptAlias, ManagedObject);
		}

		internal void RegisterScriptableMethod (MethodInfo mi, string scriptAlias, object target)
		{
			if (reservedNames.Contains (scriptAlias))
				throw new InvalidOperationException ("Reserved name '" + scriptAlias + "'.");

			if (!methods.ContainsKey (scriptAlias)) {
				methods[scriptAlias] = new List<Method>();
			}
			methods[scriptAlias].Add (new Method { method = mi, obj = target });
		}

		internal void RegisterScriptableMethod (MethodInfo mi)
		{
			string scriptAlias = mi.Name;
			if (mi.IsDefined (typeof(ScriptableMemberAttribute), false)) {
			    ScriptableMemberAttribute att = (ScriptableMemberAttribute) mi.GetCustomAttributes (typeof (ScriptableMemberAttribute), false)[0];
				scriptAlias = (att.ScriptAlias ?? scriptAlias);
			}
			RegisterScriptableMethod (mi, scriptAlias, ManagedObject);
		}

		private bool HasMethod (string scriptAlias)
		{
			return methods.ContainsKey (scriptAlias);
		}
#endregion

#region mappings from native handles to ScriptObjects
		internal static Dictionary<IntPtr, ScriptObjectToggleRef> toggleRefs;
		static Dictionary<IntPtr, WeakReference> cachedObjects;

		internal static ScriptObject LookupScriptObject (IntPtr obj_handle)
		{
			ScriptObject obj = null;
			ScriptObjectToggleRef tref = null;
			WeakReference wref = null;

			lock (toggleRefs) {
				toggleRefs.TryGetValue (obj_handle, out tref);
			}

			if (tref != null)
				obj = tref.Target;
			else {
				lock (cachedObjects) {
					cachedObjects.TryGetValue (obj_handle, out wref);
					if (wref != null) {
						if (wref.IsAlive)
							obj = wref.Target as ScriptObject;
						else
							cachedObjects.Remove (obj_handle);
					}
				}
			}
			return obj;
		}

		/* thread-safe */
		private static bool AddNativeMapping (IntPtr native, ScriptObject obj)
		{
			ScriptObjectToggleRef tref;
			
			if (native == IntPtr.Zero)
				return false;

			lock (toggleRefs) {
				if (toggleRefs.ContainsKey (native)) {
#if DEBUG
					throw new ExecutionEngineException ("multiple mappings registered for the same NPObject peer");
#endif
					Console.WriteLine ("multiple mappings registered for the same NPObject peer 0x{0:x}, type = {1}", native, obj.GetType());
					Console.WriteLine (Environment.StackTrace);
					return false;
				}
				
				tref = new ScriptObjectToggleRef (obj);
				toggleRefs[native] = tref;
			}
			tref.Initialize ();
			return true;
		}
		
		/* thread-safe */
		private static void FreeNativeMapping (ScriptObject obj)
		{
			ScriptObjectToggleRef tref = null;
			IntPtr native = obj.Handle;
			
			if (native == IntPtr.Zero)
				return;

			toggleRefs.TryGetValue (native, out tref);
			if (tref != null) {
				tref.Free ();
				toggleRefs.Remove (native);
			}
			GC.SuppressFinalize (obj);
		}


		bool InvalidateHandle ()
		{
			if (free_mapping)
				FreeNativeMapping (this);
			free_mapping = false;

			_handle = IntPtr.Zero;

			return true;
		}
#endregion

#region Delegates we pass to moonlight_scriptable_object_create

		static InvalidateHandleDelegate invalidate_handle = new InvalidateHandleDelegate (InvalidateHandleFromUnmanagedSafe);
		static HasMemberDelegate has_method = new HasMemberDelegate (HasMethodFromUnmanagedSafe);
		static HasMemberDelegate has_property = new HasMemberDelegate (HasPropertyFromUnmanagedSafe);
		static InvokeDelegate invoke = new InvokeDelegate (InvokeFromUnmanagedSafe);
		static SetPropertyDelegate set_prop = new SetPropertyDelegate (SetPropertyFromUnmanagedSafe);
		static GetPropertyDelegate get_prop = new GetPropertyDelegate (GetPropertyFromUnmanagedSafe);

		private static bool InvalidateHandleFromUnmanagedSafe (IntPtr obj_handle)
		{
			try {
				ScriptObject obj = LookupScriptObject (obj_handle);
				return obj.InvalidateHandle ();
			}
			catch (Exception ex) {
				try {
					Console.WriteLine ("Moonlight: Unhandled exception in HTML bridge: {0}", ex);
				} catch {
				}
				return true;
			}
		}

		private static bool HasPropertyFromUnmanagedSafe (IntPtr obj_handle, string name)
		{
			try {
				ScriptObject obj = LookupScriptObject (obj_handle);
				return obj.HasProperty (name);
			}
			catch (Exception ex) {
				try {
					Console.WriteLine ("Moonlight: Unhandled exception in HTML bridge: {0}", ex);
				} catch {
				}
				return true;
			}
		}

		private static bool HasMethodFromUnmanagedSafe (IntPtr obj_handle, string name)
		{
			try {
				ScriptObject obj = LookupScriptObject (obj_handle);
				return obj.HasMethod (name);
			}
			catch (Exception ex) {
				try {
					Console.WriteLine ("Moonlight: Unhandled exception in HTML bridge: {0}", ex);
				} catch {
				}
				return true;
			}
		}

		private static bool InvokeFromUnmanagedSafe (IntPtr obj_handle, string name, IntPtr[] uargs, int arg_count, ref Value return_value, out string exc_string)
		{
			exc_string = null;
			try {
				ScriptObject obj = LookupScriptObject (obj_handle);
				return obj.InvokeFromUnmanaged (name, uargs, arg_count, ref return_value);
			}
			catch (Exception ex) {
				try {
					exc_string = ex.ToString();
					Console.WriteLine ("Moonlight: Unhandled exception in HTML bridge: {0}", ex);
				} catch {
				}
				return true;
			}
		}

		private static bool SetPropertyFromUnmanagedSafe (IntPtr obj_handle, string name, IntPtr[] uargs, int arg_count, ref Value value, out string exc_string)
		{
			exc_string = null;
			try {
				ScriptObject obj = LookupScriptObject (obj_handle);
				return obj.SetPropertyFromUnmanaged (name, uargs, arg_count, ref value);
			}
			catch (Exception ex) {
				try {
					exc_string = ex.ToString();
					Console.WriteLine ("Moonlight: Unhandled exception in HTML bridge: {0}", ex);
				} catch {
				}
				return true;
			}
		}

		private static bool GetPropertyFromUnmanagedSafe (IntPtr obj_handle, string name, IntPtr[] uargs, int arg_count, ref Value value, out string exc_string)
		{
			exc_string = null;
			try {
				ScriptObject obj = LookupScriptObject (obj_handle);
				return obj.GetPropertyFromUnmanaged (name, uargs, arg_count, ref value);
			}
			catch (Exception ex) {
				try {
					exc_string = ex.ToString();
					Console.WriteLine ("Moonlight: Unhandled exception in HTML bridge: {0}", ex);
				} catch {
				}
				return true;
			}
		}

#endregion

#region Methods

		bool IsOptional (ParameterInfo pi)
		{
			return pi.IsOptional || pi.GetCustomAttributes(typeof (ParamArrayAttribute), false).Length > 0;
		}

		protected bool ValidateArguments (MethodInfo mi, object[] args)
		{
			if (mi.GetParameters().Length != args.Length) {
				int required_paramcount = 0;
				bool optional_param = false;
				foreach (ParameterInfo p in mi.GetParameters()) {
					if (IsOptional (p)) {
						optional_param = true;
						break;
					}
					required_paramcount++;
				}
				
				// if we don't supply enough required arguments, we fail regardless of optional parameters.
				if (required_paramcount > args.Length)
					return false;

				// if we don't have optional parameters, make sure we don't supply more than the required arguments.
				if (!optional_param && args.Length > required_paramcount)
					return false;
			}

			// TODO: refactor this, the js binder is doing this work already
			ParameterInfo[] parms = mi.GetParameters ();
			for (int i = 0; i < parms.Length; i++) {

				if (IsOptional (parms[i]))
					break;

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

		internal void Invoke (string name, object[] args, ref Value ret)
		{
			if (methods.ContainsKey (name)) {
				foreach (Method method in methods[name]) {
					if (method.method != null)
						if (ValidateArguments (method.method, args)) {
							object rv = Invoke (method.method, method.obj, args);

							if (method.method.ReturnType != typeof (void))
								ScriptObjectHelper.ValueFromObject (ref ret, rv);

							return;
						}
				}
			}

			Console.WriteLine ("ScriptObject.Invoke: NOT IMPLEMENTED: {0}", name.ToLower ());
		}

		internal object Invoke (MethodInfo method, object obj, object[] args)
		{
			ParameterInfo[] pis = method.GetParameters ();
			object[] methodArgs = new object[pis.Length];

			for (int i = 0; i < args.Length; i++) {
				if (pis[i].GetCustomAttributes(typeof (ParamArrayAttribute), false).Length > 0) {
					// check for params and stuff all the remaining args in an array for that
					object[] extras = new object[args.Length - i];
					for (int j = i, k = 0; j < args.Length; j++, k++)
						extras[k] = args[j];
					methodArgs[i] = extras;
					break;
				}
				methodArgs[i] = args[i];
			}
			
			return method.Invoke (obj, BindingFlags.Default, new JSFriendlyMethodBinder (), methodArgs, null);
		}

		bool InvokeFromUnmanaged (string name, IntPtr[] uargs, int arg_count, ref Value return_value)
		{
			object[] args = new object[arg_count];
			for (int i = 0; i < arg_count; i++) {
				if (uargs[i] == IntPtr.Zero) {
					args[i] = null;
				}
				else {
					Value v = (Value)Marshal.PtrToStructure (uargs[i], typeof (Value));
					args[i] = ScriptObjectHelper.ObjectFromValue<object> (v);
				}
			}

			Invoke (name, args, ref return_value);

			return true;
		}

#endregion
	}
}
