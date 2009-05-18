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

using System.ComponentModel;
using System.Windows.Threading;
using System.Collections.Generic;
using Mono;

namespace System.Windows.Browser {
	public class ScriptObject {
		internal IntPtr handle;
		string script_key;
		object managed;

		internal ScriptObject ()
		{
		}

		internal ScriptObject (IntPtr handle)
		{
			// FIXME: do we need to do registration or whatever?
			this.handle = handle;
		}

		internal ScriptObject (object obj)
		{
			managed = obj;
		}

		internal IntPtr Handle {
			get { return handle; }
		}

		~ScriptObject ()
		{
			// FIXME: same as .ctor().
		}

		public virtual void SetProperty (string name, object value)
		{
			SetPropertyInternal (name, value);
		}

		public void SetProperty (int index, object value)
		{
			throw new System.NotImplementedException ();
		}

		public virtual object GetProperty (string name)
		{
			return GetPropertyInternal <object> (name);
		}

		public object GetProperty (int index)
		{
			return GetProperty (index.ToString ());
		}

		protected virtual object ConvertTo (Type targetType, bool allowSerialization)
		{
			if (targetType.IsAssignableFrom (GetType()))
				return this;

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

		public object ManagedObject {
			get { return managed; }
		}

		[EditorBrowsable (EditorBrowsableState.Advanced)]
		[CLSCompliant (false)]
		public Dispatcher Dispatcher {
			get { return Dispatcher.Main; }
		}

		internal static T GetPropertyInternal<T> (IntPtr h, string name)
		{
			Mono.Value res;
			NativeMethods.html_object_get_property (WebApplication.Current.PluginHandle, h, name, out res);
			if (res.k != Mono.Kind.INVALID)
				return (T)ScriptableObjectWrapper.ObjectFromValue<T> (res);
			return default (T);
		}

		internal T GetPropertyInternal<T> (string name)
		{
			Mono.Value res;
			NativeMethods.html_object_get_property (WebApplication.Current.PluginHandle, handle, name, out res);
			if (res.k != Mono.Kind.INVALID)
				return (T)ScriptableObjectWrapper.ObjectFromValue<T> (res);
			return default (T);
		}

		internal static void SetPropertyInternal (IntPtr h, string name, object value)
		{
			Mono.Value dp = new Mono.Value ();
			ScriptableObjectWrapper.ValueFromObject (ref dp, value);
			NativeMethods.html_object_set_property (WebApplication.Current.PluginHandle, h, name, ref dp);
		}

		protected void SetPropertyInternal (string name, object value)
		{
			Mono.Value dp = new Mono.Value ();
			ScriptableObjectWrapper.ValueFromObject (ref dp, value);
			NativeMethods.html_object_set_property (WebApplication.Current.PluginHandle, handle, name, ref dp);
		}

		protected T InvokeInternal<T> (string name, params object [] args)
		{
			Mono.Value res;
			Mono.Value [] vargs = new Mono.Value [args.Length];

			for (int i = 0; i < args.Length; i++)
				ScriptableObjectWrapper.ValueFromObject (ref vargs [i], args [i]);

			NativeMethods.html_object_invoke (WebApplication.Current.PluginHandle, handle, name, vargs, (uint)args.Length, out res);

			if (res.k != Mono.Kind.INVALID)
				return (T)ScriptableObjectWrapper.ObjectFromValue<T> (res);

			return default (T);
		}

		protected T InvokeInternal<T> (params object [] args)
		{
			Mono.Value res;
			Mono.Value [] vargs = new Mono.Value [args.Length];

			for (int i = 0; i < args.Length; i++)
				ScriptableObjectWrapper.ValueFromObject (ref vargs [i], args [i]);

			NativeMethods.html_object_invoke_self (WebApplication.Current.PluginHandle, handle, vargs, (uint)args.Length, out res);

			if (res.k != Mono.Kind.INVALID)
				return (T)ScriptableObjectWrapper.ObjectFromValue<T> (res);

			return default (T);
		}

	}
}
