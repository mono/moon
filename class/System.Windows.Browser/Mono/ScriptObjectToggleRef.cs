//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2010 Novell, Inc.
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
using System.Windows.Browser;

namespace Mono
{
	internal sealed class ScriptObjectToggleRef : ToggleRef
	{
		public ScriptObjectToggleRef (ScriptObject target)
			: base (target.Handle, target)
		{
		}

		protected override void AddToggleRefNotifyCallback ()
		{
			NativeMethods.moonlight_object_add_toggle_ref_notifier (handle, toggle_notify_callback);
		}

		protected override void RemoveToggleRefNotifyCallback ()
		{
			NativeMethods.moonlight_object_remove_toggle_ref_notifier (handle);
		}

		public ScriptObject Target {
			get { return (ScriptObject)TargetCore; }
		}

		static void RefToggled (IntPtr obj, bool isLastRef)
		{
			try {
				ScriptObjectToggleRef tref = null;
				ScriptObject.toggleRefs.TryGetValue (obj, out tref);
				if (tref != null) {
					Console.WriteLine ("Toggle ({0})", isLastRef);
					tref.Toggle (isLastRef);
				}
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