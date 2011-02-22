//
// TimeManager.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2011 Novell, Inc.
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

namespace Mono {

	partial class TimeManager : INativeEventObjectWrapper
	{
		DependencyObjectHandle handle;

		public event EventHandler Render {
			add { EventList.RegisterEvent (this, EventIds.TimeManager_RenderEvent, value, Events.CreateRenderingEventHandlerDispatcher (value)); }
			remove { EventList.UnregisterEvent (this, EventIds.TimeManager_RenderEvent, value); }
		}

		public event EventHandler UpdateInput {
			add { EventList.RegisterEvent (this, EventIds.TimeManager_UpdateInputEvent, value, Events.CreateRenderingEventHandlerDispatcher (value)); }
			remove { EventList.UnregisterEvent (this, EventIds.TimeManager_UpdateInputEvent, value); }
		}

		internal TimeManager (IntPtr raw, bool dropref)
		{
			NativeHandle = raw;
			if (dropref)
				NativeMethods.event_object_unref (raw);
		}

		internal IntPtr NativeHandle {
			get { return handle.Handle; }
			set {
				if (handle != null) {
					throw new InvalidOperationException ("native handle is already set");
				}

				NativeDependencyObjectHelper.AddNativeMapping (value, this);
				handle = new DependencyObjectHandle (value, this);
			}
		}

		IntPtr INativeEventObjectWrapper.NativeHandle {
			get { return NativeHandle; }
			set { NativeHandle = value; }
		}

		void INativeEventObjectWrapper.MentorChanged (IntPtr mentor_ptr)
		{
		}

		Kind INativeEventObjectWrapper.GetKind ()
		{
			return Kind.TIMEMANAGER;
		}

		private EventHandlerList event_list;

		public EventHandlerList EventList {
			get {
				if (event_list == null)
					event_list = new EventHandlerList (this);
				return event_list;
			}
		}
	}
}
