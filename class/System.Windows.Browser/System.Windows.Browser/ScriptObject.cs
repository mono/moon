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
			HtmlObject.SetPropertyInternal (handle, name, value);
		}

		public void SetProperty (int index, object value)
		{
			throw new System.NotImplementedException ();
		}

		public virtual object GetProperty (string name)
		{
			object result;

			result = HtmlObject.GetPropertyInternal <object> (handle, name);

			if (result != null) {
				if (result is int) {
					// When the target type is object, SL converts ints to doubles to wash out
					// browser differences. (Safari apparently always returns doubles, FF
					// ints and doubles, depending on the value).
					// See: http://msdn.microsoft.com/en-us/library/cc645079(VS.95).aspx
					result = (double) (int) result;
				} else if (result is IntPtr) {
					result = new ScriptObject ((IntPtr)result);
				}
			}

			return result;
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
			return HtmlObject.InvokeInternal <object> (handle, name, args);
		}

		public virtual object InvokeSelf (params object [] args)
		{
			return HtmlObject.InvokeInternal <object> (handle, args);
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
	}
}
