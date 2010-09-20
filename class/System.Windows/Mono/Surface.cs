//
// Surface.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2008 Novell, Inc.
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
using System.Reflection;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using Mono;

namespace Mono
{	
	/*
	 *  The managed equivalent of the unmanaged Surface
	 */
	internal sealed partial class Surface : INativeEventObjectWrapper, IRefContainer
	{
		private EventObjectSafeHandle safeHandle;
		
		public IntPtr Native {
			get { return safeHandle.DangerousGetHandle (); }
		}

		EventObjectSafeHandle INativeEventObjectWrapper.SafeHandle {
			get { return safeHandle; }
		}

		public Surface (IntPtr raw, bool dropref)
		{
			safeHandle = NativeDependencyObjectHelper.AddNativeMapping (raw, this);;

			strongRefs = new Dictionary<IntPtr,INativeEventObjectWrapper> ();

			NativeDependencyObjectHelper.SetManagedPeerCallbacks (this);

			if (dropref)
				NativeMethods.event_object_unref (Native);
		}
		
		Kind INativeEventObjectWrapper.GetKind ()
		{
			return NativeMethods.event_object_get_object_type (Native);
		}

		Dictionary<IntPtr,INativeEventObjectWrapper> strongRefs;

		void IRefContainer.AddStrongRef (IntPtr referent, string name)
		{
			if (strongRefs.ContainsKey (referent))
				return;

			var o = NativeDependencyObjectHelper.FromIntPtr (referent);
			if (o != null) {
#if DEBUG_REF
				Console.WriteLine ("Adding ref from {0}/{1} to {2}/{3}", GetHashCode(), this, o.GetHashCode(), o);
#endif
				strongRefs.Add (referent, o);
			}
		}

		void IRefContainer.ClearStrongRef (IntPtr referent, string name)
		{
#if DEBUG_REF
			var o = NativeDependencyObjectHelper.FromIntPtr (referent);
			Console.WriteLine ("Clearing ref from {0}/{1} to {2}/{3}", GetHashCode(), this, o.GetHashCode(), o);
#endif
			strongRefs.Remove (referent);
		}

#if HEAPVIZ
		System.Collections.ICollection IRefContainer.GetManagedRefs ()
		{
			List<HeapRef> refs = new List<HeapRef> ();
			foreach (IntPtr nativeref in strongRefs.Keys)
				refs.Add (new HeapRef (strongRefs[nativeref]));
				
			return refs;
		}
#endif

		void INativeEventObjectWrapper.MentorChanged (IntPtr mentor_ptr)
		{
		}

		void INativeEventObjectWrapper.OnAttached ()
		{
		}

		void INativeEventObjectWrapper.OnDetached ()
		{
		}
	}
}
