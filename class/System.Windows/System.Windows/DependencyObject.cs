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
using System.Windows;
using System.Runtime.InteropServices;
using System.Windows.Threading;
using System.Threading;

namespace System.Windows {
	public abstract partial class DependencyObject : INativeDependencyObjectWrapper {
		internal static Thread moonlight_thread;
		internal IntPtr _native;
		EventHandlerList event_list;
		bool free_mapping;

		internal EventHandlerList EventList {
			get {
				if (event_list == null)
					event_list = new EventHandlerList ();
				return event_list;
			}
		}

		IntPtr INativeEventObjectWrapper.NativeHandle {
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

				_native = value;
				free_mapping = NativeDependencyObjectHelper.AddNativeMapping (value, this);
			}
		}
		
		static DependencyObject ()
		{
			moonlight_thread = Thread.CurrentThread;
		}

		// This created objects with a managed lifetime, so the native ref will be dropped
		// as soon as the object is created and handed to ToggleRef
		protected DependencyObject () : this (NativeMethods.dependency_object_new (), true)
		{
		}

		internal DependencyObject (IntPtr raw, bool dropref)
		{
			native = raw;
			NativeMethods.event_object_set_object_type (native, GetKind ());
			// Objects created on the managed side have a normal managed lifetime,
			// so drop the native ref hold
			if (dropref)
				NativeMethods.event_object_unref (native);
		}

		internal void Free ()
		{
			UnregisterAllEvents ();

			if (free_mapping)
				NativeDependencyObjectHelper.FreeNativeMapping (this);
		}

		~DependencyObject ()
		{
			Free ();
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

		internal void RegisterEvent (int eventId, Delegate managedHandler, UnmanagedEventHandler nativeHandler)
		{
			if (managedHandler == null)
				return;

			int token = Events.AddHandler (this, eventId, nativeHandler);

			EventList.AddHandler (eventId, token, managedHandler, nativeHandler);
		}

		internal void UnregisterEvent (int eventId, Delegate managedHandler)
		{
			UnmanagedEventHandler nativeHandler = EventList.RemoveHandler (eventId, managedHandler);

			if (nativeHandler == null)
				return;

			Events.RemoveHandler (this, eventId, nativeHandler);
		}

		void UnregisterAllEvents ()
		{
			foreach (int eventId in EventList.Keys) {
				foreach (EventHandlerData d in EventList[eventId].Values) {
					Events.RemoveHandler (this, eventId, d.NativeHandler);
				}
			}
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
				return Dispatcher.Main;
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
			if (name == null)
				throw new ArgumentNullException ("name");

			Kind k;
			IntPtr o = NativeMethods.dependency_object_find_name (native, name, out k);
			if (o == IntPtr.Zero)
				return null;

			return NativeDependencyObjectHelper.Lookup (k, o) as DependencyObject;
		}

		Kind INativeEventObjectWrapper.GetKind ()
		{
			return GetKind ();
		}

		internal Kind GetKind ()
		{
			return Deployment.Current.Types.Find (GetType()).native_handle;
		}

		[System.ComponentModel.EditorBrowsable (System.ComponentModel.EditorBrowsableState.Never)]
		public bool CheckAccess ()
		{
			return Thread.CurrentThread == moonlight_thread;
		}
		
		internal static void Initialize ()
		{
			// Here just to ensure that the static ctor is executed and
			// runtime init is initialized from some entry points
		}
	}
}
