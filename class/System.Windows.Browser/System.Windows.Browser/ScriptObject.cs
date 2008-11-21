//
// System.Windows.Browser.ScriptableObject class
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
using System.Security;
using System.Windows.Threading;


namespace System.Windows.Browser {
	public class ScriptObject {
		IntPtr handle;
		string script_key;

		internal ScriptObject ()
		{
		}
		
		internal ScriptObject (IntPtr handle)
		{
			// FIXME: do we need to do registration or whatever?
			this.handle = handle;
		}

		internal IntPtr Handle {
			get { return handle; }
		}

		~ScriptObject ()
		{
			// FIXME: same as .ctor().
		}

		[SecuritySafeCritical ()]
		public virtual void SetProperty (string name, object value)
		{
			HtmlObject.SetPropertyInternal (handle, name, value);
		}

		public void SetProperty (int index, object value)
		{
			throw new System.NotImplementedException ();
		}
		
		[SecuritySafeCritical ()]
		public virtual object GetProperty (string name)
		{
			object result;
			
			result = HtmlObject.GetPropertyInternal <object> (handle, name);

			if (result != null && result is int) {
				// When the target type is object, SL converts ints to doubles to wash out
				// browser differences. (Safari apparently always returns doubles, FF
				// ints and doubles, depending on the value).
				// See: http://msdn.microsoft.com/en-us/library/cc645079(VS.95).aspx
				result = (double) (int) result;
			}
			
			//Console.WriteLine ("ScriptObject.GetProperty ({0}) returned: {1} ({2})", name, result, result != null ? result.GetType () : null);
			
			return result;
		}
		
		public object GetProperty (int index)
		{
			throw new System.NotImplementedException ();
		}

		protected virtual object ConvertTo (Type targetType, bool allowSerialization)
		{
			throw new System.NotImplementedException ();
		}
		
		public T ConvertTo<T> ()
		{
			throw new System.NotImplementedException ();
		}
		
		[SecuritySafeCritical ()]
		public virtual object Invoke (string name, params object [] args)
		{
			throw new System.NotImplementedException ();
		}
		
		[SecuritySafeCritical ()]
		public virtual object InvokeSelf (params object [] args)
		{
			throw new System.NotImplementedException ();
		}

		protected void Initialize (IntPtr handle, IntPtr identity, bool addReference, bool releaseReferenceOnDispose)
		{
			throw new System.NotImplementedException ();
		}

		internal void InvokeMethod (string name, params object [] args)
		{
			WebApplication.InvokeMethod (handle, name, args);
		}

		internal T InvokeMethod<T> (string name, params object [] args)
		{
			return WebApplication.InvokeMethod<T> (handle, name, args);
		}
		
		[EditorBrowsable (EditorBrowsableState.Never)]
		public bool CheckAccess ()
		{
			throw new System.NotImplementedException ();
		}
		
		public object ManagedObject {
			get { throw new System.NotImplementedException (); }
		}
		
		[EditorBrowsable (EditorBrowsableState.Advanced)]
		public Dispatcher Dispatcher {
			get { throw new System.NotImplementedException (); }
		}
	}
}
