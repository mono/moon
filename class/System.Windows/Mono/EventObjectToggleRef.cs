//
// EventObjectToggleRef.cs
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
	internal sealed class EventObjectToggleRef : ToggleRef
	{
		public EventObjectToggleRef (INativeEventObjectWrapper target)
			: base (target.NativeHandle, target)
		{
		}

		protected override void AddToggleRefNotifyCallback ()
		{
			NativeMethods.event_object_add_toggle_ref_notifier (handle, toggle_notify_callback);
		}

		protected override void RemoveToggleRefNotifyCallback ()
		{
			NativeMethods.event_object_remove_toggle_ref_notifier (handle);
		}

		public INativeEventObjectWrapper Target {
			get { return (INativeEventObjectWrapper)TargetCore; }
		}


		static void RefToggled (IntPtr obj, bool isLastRef)
		{
			try {
				EventObjectToggleRef tref = null;
				NativeDependencyObjectHelper.objects.TryGetValue (obj, out tref);
				if (tref != null)
					tref.Toggle (isLastRef);
			} catch (Exception e) {
				//ExceptionManager.RaiseUnhandledException (e, false);
				try {
					Console.WriteLine (e);
				} catch {
					// Ignore
				}
			}
		}

		static ToggleNotifyHandler toggle_notify_callback = new ToggleNotifyHandler (RefToggled);
	}
}