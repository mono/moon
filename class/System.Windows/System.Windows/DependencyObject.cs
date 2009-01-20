//
// DependencyObject.cs
//
// Author:
//   Iain McCoy (iain@mccoy.id.au)
//   Moonlight Team (moonlight-list@lists.ximian.com)
//
// Copyright 2005 Iain McCoy
// Copyright 2007 Novell, Inc.
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

using Mono;
using System.Collections.Generic;
using System.Security;
using System.Windows;
using System.Runtime.InteropServices;
using System.Windows.Threading;
using System.Threading;

namespace System.Windows {
	public abstract partial class DependencyObject : INativeDependencyObjectWrapper {
		static Thread moonlight_thread;
		internal IntPtr _native;
		EventHandlerList event_list;
		
		internal EventHandlerList EventList {
			get {
				if (event_list == null)
					event_list = new EventHandlerList ();
				return event_list;
			}
		}

		[ThreadStatic] static private Dispatcher dispatcher;
		private GCHandle _handle;
		internal GCHandle GCHandle {
			get {
				if (!_handle.IsAllocated)
					_handle = GCHandle.Alloc (this);
				return _handle;
			}
		}

		IntPtr INativeDependencyObjectWrapper.NativeHandle {
			get { return native; }
			set { native = value; }
		}

		internal IntPtr native {
			get {
				return _native;
			}

			set {
				if (_native != IntPtr.Zero) {
					throw new InvalidOperationException ("DependencyObject.native is already set");
				}

				NativeDependencyObjectHelper.AddNativeMapping (value, this);

				_native = value;
			}
		}
		
		static DependencyObject ()
		{
			moonlight_thread = Thread.CurrentThread;
		}

		protected DependencyObject ()
		{
			native = NativeMethods.dependency_object_new ();
		}

		internal DependencyObject (IntPtr raw)
		{
			native = raw;
		}
		
		internal void Free ()
		{
			if (this.native != IntPtr.Zero) {
				NativeMethods.event_object_unref(this.native);
				this.native = IntPtr.Zero;
			}
		}
		
		~DependencyObject ()
		{
			Free ();
			if (_handle.IsAllocated) {
				_handle.Free();
			}
		}

		public object GetValue (DependencyProperty dp)
		{
			return NativeDependencyObjectHelper.GetValue (this, dp);
		}

		public object GetAnimationBaseValue (DependencyProperty dp)
		{
			return NativeDependencyObjectHelper.GetAnimationBaseValue (this, dp);
		}

		public object ReadLocalValue (DependencyProperty dp)
		{
			return ReadLocalValueImpl (dp);
		}

		internal void RegisterEvent (object eventObject, string eventName, UnmanagedEventHandler nativeHandler, Delegate managedHandler)
		{
			if (EventList[eventObject] == null)
				Events.AddHandler (this, eventName, nativeHandler);
			EventList.AddHandler (eventObject, managedHandler);
		}

		internal void UnregisterEvent (object eventObject, string eventName, UnmanagedEventHandler nativeHandler, Delegate managedHandler)
		{
			EventList.RemoveHandler (eventObject, managedHandler);
			if (EventList[eventObject] == null)
				Events.RemoveHandler (this, eventName, nativeHandler);
		}

		internal virtual object ReadLocalValueImpl (DependencyProperty dp)
		{
			return NativeDependencyObjectHelper.ReadLocalValue (this, dp);
		}
		
		public void ClearValue (DependencyProperty dp)
		{
			ClearValueImpl (dp);
		}

		internal virtual void ClearValueImpl (DependencyProperty dp)
		{
			NativeDependencyObjectHelper.ClearValue (this, dp);
		}
		
		[System.ComponentModel.EditorBrowsable (System.ComponentModel.EditorBrowsableState.Advanced)]
		public Dispatcher Dispatcher {
			get {
				if (dispatcher == null)
					dispatcher = new Dispatcher ();

				return dispatcher;
			}
		}
		

		public void SetValue (DependencyProperty dp, object value)
		{
			if (dp == null)
				throw new ArgumentNullException ("property");
			if (dp.IsReadOnly)
				throw new InvalidOperationException ();
			SetValueImpl (dp, value);
		}

		internal virtual void SetValueImpl (DependencyProperty dp, object value)
		{
			NativeDependencyObjectHelper.SetValue (this, dp, value);
		}

		internal DependencyObject DepObjectFindName (string name)
		{
			Kind k;
			IntPtr o = NativeMethods.dependency_object_find_name (native, name, out k);
			if (o == IntPtr.Zero)
				return null;

			return NativeDependencyObjectHelper.Lookup (k, o) as DependencyObject;
		}

		Kind INativeDependencyObjectWrapper.GetKind ()
		{
			return GetKind ();
		}

		internal virtual Kind GetKind ()
		{
			return Kind.DEPENDENCY_OBJECT;
		}

		[System.ComponentModel.EditorBrowsable (System.ComponentModel.EditorBrowsableState.Never)]
		public bool CheckAccess ()
		{
			return Thread.CurrentThread == moonlight_thread;
		}
		
		private void CheckNativeAndThread ()
		{
			if (native == IntPtr.Zero) {
				throw new Exception (
					string.Format ("Uninitialized object: this object ({0}) has not set its native handle set", GetType ().FullName));
			}

			if (!CheckAccess ())
				throw new UnauthorizedAccessException ("Invalid access of Moonlight from an external thread");
		}

#if NET_2_1
		internal
#else
		public
#endif
		static void Initialize ()
		{
			// Here just to ensure that the static ctor is executed and
			// runtime init is initialized from some entry points
		}
	}
}
