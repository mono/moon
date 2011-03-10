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
		private IntPtr native;
		bool free_mapping;
		
		private EventHandlerList event_list;

		public EventHandlerList EventList {
			get {
				if (event_list == null)
					event_list = new EventHandlerList (this);
				return event_list;
			}
		}

		public IntPtr Native {
			get { return native; }
			set {
				if (native != IntPtr.Zero) {
					throw new InvalidOperationException ("Surface.native is already set");
				}

				native = value;

				free_mapping = NativeDependencyObjectHelper.AddNativeMapping (value, this);
			}

		}

		public TimeManager TimeManager {
			get { return (TimeManager) NativeDependencyObjectHelper.Lookup (NativeMethods.surface_get_time_manager (native)); }
		}

		public Surface (IntPtr native, bool dropref)
		{
			this.Native = native;

			strongRefs = new Dictionary<IntPtr,object> ();

			NativeDependencyObjectHelper.SetManagedPeerCallbacks (this);

			if (dropref)
				NativeMethods.event_object_unref (native);
		}
		
		Kind INativeEventObjectWrapper.GetKind ()
		{
			return NativeMethods.event_object_get_object_type (native);
		}
		
		IntPtr INativeEventObjectWrapper.NativeHandle {
			get { return Native; }
			set { Native = value; }
		}

		Dictionary<IntPtr,object> strongRefs;

		void IRefContainer.SetStrongRef (IntPtr id, object value)
		{
#if DEBUG
			if (id == IntPtr.Zero)
				Console.WriteLine ("Moon Error: Surface.SetStrongRef was called with an invalid ID with value: {0}", value);
#endif
#if DEBUG_REF
			Console.WriteLine ("Setting ref from {0}/{1} to {2}/{3}", GetHashCode(), this, value == null ? 0 : value.GetHashCode(), value);
#endif
			if (value == null)
				strongRefs.Remove (id);
			else
				strongRefs [id] = value;
		}

#if HEAPVIZ
		System.Collections.ICollection IRefContainer.GetManagedRefs ()
		{
			List<HeapRef> refs = new List<HeapRef> ();
				foreach (var keypair in strongRefs)
					if (keypair.Value is INativeEventObjectWrapper)
						refs.Add (new HeapRef (true, (INativeEventObjectWrapper)keypair.Value, NativeDependencyObjectHelper.IdToName (keypair.Key)));
				
			return refs;
		}
#endif

		void INativeEventObjectWrapper.MentorChanged (IntPtr mentor_ptr)
		{
		}

		internal void Free ()
		{
			if (free_mapping) {
				free_mapping = false;
				NativeDependencyObjectHelper.FreeNativeMapping (this);
			}
		}
		
		~Surface ()
		{
			Free ();
		}
	}
}
