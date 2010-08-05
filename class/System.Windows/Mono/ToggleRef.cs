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
	internal abstract class ToggleRef
	{
		protected IntPtr handle;
#if HEAPVIZ
		internal
#endif
		protected object reference;
		protected GCHandle gch;

		public ToggleRef (IntPtr handle, object reference)
		{
			this.handle = handle;
			this.reference = reference;

			gch = GCHandle.Alloc (this);
		}
		
		protected abstract void AddToggleRefNotifyCallback ();
		protected abstract void RemoveToggleRefNotifyCallback ();

		protected object TargetCore {
			get {
				if (reference == null)
					return null;

				WeakReference weak = reference as WeakReference;
				if (weak == null)
					return reference;
				else
					return weak.Target;
			}
		}


		public void Initialize ()
		{
			AddToggleRefNotifyCallback  ();
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

		public void Free ()
		{
			RemoveToggleRefNotifyCallback ();
			reference = null;
			gch.Free ();
		}

		protected void Toggle (bool isLastRef)
		{
			WeakReference weak = reference as WeakReference;
			if (!isLastRef) {
				if (weak != null && weak.IsAlive) {
					reference = weak.Target;
#if DEBUG_REF
					Console.WriteLine ("Toggling weak to strong for {0}/{1}", reference.GetHashCode (), reference);
#endif
				}
			}
			else {
				if (weak == null) {
#if DEBUG_REF
					Console.WriteLine ("Toggling strong to weak for {0}/{1}", reference.GetHashCode (), reference);
#endif
					reference = new WeakReference (reference);
				}
			}
		}

		internal delegate void ToggleNotifyHandler (IntPtr obj, bool isLastref);
	}
}
