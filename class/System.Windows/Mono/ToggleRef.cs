//
// ToggleRef.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2009 Novell, Inc.
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
using System.Runtime.InteropServices;
using System.Collections.Generic;

namespace Mono
{
	internal class ToggleRef
	{
		IntPtr handle;
		object reference;
		GCHandle gch;

		public ToggleRef (INativeEventObjectWrapper target)
		{
			handle = target.NativeHandle;
			gch = GCHandle.Alloc (this);
			reference = target;
		}
		
		public void Initialize () {
			NativeMethods.event_object_add_toggle_ref_notifier (handle, ToggleNotifyCallback);
		}

		public bool IsAlive {
			get {
				if (reference is WeakReference) {
					WeakReference weak = reference as WeakReference;
					return weak.IsAlive;
				} else if (reference == null)
					return false;
				return true;
			}
		}

		public IntPtr Handle {
			get {
				return handle;
			}
		}

		public INativeEventObjectWrapper Target {
			get {
				if (reference == null)
					return null;
				else if (reference is INativeEventObjectWrapper)
					return reference as INativeEventObjectWrapper;

				WeakReference weak = reference as WeakReference;
				return weak.Target as INativeEventObjectWrapper;
			}
		}

		public void Free ()
		{
			NativeMethods.event_object_remove_toggle_ref_notifier (handle);
			reference = null;
			gch.Free ();
		}

		void Toggle (bool isLastRef)
		{
			if (isLastRef && reference is INativeEventObjectWrapper)
				reference = new WeakReference (reference);
			else if (!isLastRef && reference is WeakReference) {
				WeakReference weak = reference as WeakReference;
				if (weak.IsAlive)
					reference = weak.Target;
			}
		}

		internal delegate void ToggleNotifyHandler (IntPtr obj, bool isLastref);

		static void RefToggled (IntPtr obj, bool isLastRef)
		{
			try {
				ToggleRef tref = null;
				NativeDependencyObjectHelper.objects.TryGetValue (obj, out tref);
				if (tref != null)
					tref.Toggle (isLastRef);
			} catch (Exception e) {
				//ExceptionManager.RaiseUnhandledException (e, false);
				Console.WriteLine (e);
			}
		}

		static ToggleNotifyHandler toggle_notify_callback;
		static ToggleNotifyHandler ToggleNotifyCallback {
			get {
				if (toggle_notify_callback == null)
					toggle_notify_callback = new ToggleNotifyHandler (RefToggled);
				return toggle_notify_callback;
			}
		}
	}
}
