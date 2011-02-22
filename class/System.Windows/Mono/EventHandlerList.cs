//
// hacked from System.ComponentModel.EventHandlerList.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// (C) Ximian, Inc.  http://www.ximian.com
//

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
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace Mono {

	// <summary>
	//   List of Event delegates.
	// </summary>
	//
	// <remarks>
	//   Longer description
	// </remarks>
	struct EventHandlerDataKey : IEquatable<EventHandlerDataKey>
	{
		public int Token;
		public int EventId;

		public override bool Equals (object obj)
		{
			 return obj is EventHandlerDataKey ? Equals ((EventHandlerDataKey) obj) : false;
		}

		public bool Equals (EventHandlerDataKey other)
		{
			return Token == other.Token && EventId == other.EventId;
		}

		public override int GetHashCode ()
		{
			 return Token << 16 | EventId;
		}
	}

	sealed class EventHandlerData {
		public Delegate ManagedDelegate;
		public UnmanagedEventHandler NativeHandler;
		public GDestroyNotify DtorAction;
	}

	sealed class EventHandlerList : Dictionary<EventHandlerDataKey, EventHandlerData> {

		public EventHandlerList (INativeEventObjectWrapper wrapper)
		{
		}

		private void AddHandler (int eventId, int token, Delegate managedDelegate, UnmanagedEventHandler nativeHandler)
		{
			AddHandler (eventId, token, managedDelegate, nativeHandler);
		}

		private void AddHandler (int eventId, int token, Delegate managedDelegate, UnmanagedEventHandler nativeHandler, GDestroyNotify dtor_action)
		{
			Add (new EventHandlerDataKey {
					EventId = eventId,
					Token = token
			}, new EventHandlerData {
					ManagedDelegate = managedDelegate,
					NativeHandler = nativeHandler,
					DtorAction = dtor_action
			});
		}

		private void RemoveHandler (int eventId, int token)
		{
			Remove (new EventHandlerDataKey { EventId = eventId, Token = token });
		}

		public void RegisterEvent (INativeEventObjectWrapper obj, int eventId, Delegate managedHandler, UnmanagedEventHandler nativeHandler)
		{
			RegisterEvent (obj, eventId, managedHandler, nativeHandler, false);
		}

		public void RegisterEvent (INativeEventObjectWrapper obj, int eventId, Delegate managedHandler, UnmanagedEventHandler nativeHandler, bool handledEventsToo)
		{
			RegisterEvent (obj.NativeHandle, eventId, managedHandler, nativeHandler, handledEventsToo);
		}

		public void RegisterEvent (IntPtr obj, int eventId, Delegate managedHandler, UnmanagedEventHandler nativeHandler)
		{
			RegisterEvent (obj, eventId, managedHandler, nativeHandler, false);
		}

		public void RegisterEvent (IntPtr obj, int eventId, Delegate managedHandler, UnmanagedEventHandler nativeHandler, bool handledEventsToo)
		{
			if (managedHandler == null)
				return;

			int token = -1;

			GDestroyNotify dtor_action = (data) => {
				RemoveHandler (eventId, token);
			};

			token = Events.AddHandler (obj, eventId, nativeHandler, dtor_action, handledEventsToo);
			
			AddHandler (eventId, token, managedHandler, nativeHandler, dtor_action);
		}

		public void UnregisterEvent (INativeEventObjectWrapper obj, int eventId, Delegate managedHandler)
		{
			UnregisterEvent (obj.NativeHandle, eventId, managedHandler);
		}

		public void UnregisterEvent (IntPtr obj, int eventId, Delegate managedHandler)
		{
			foreach (var keypair in this) {
				if (keypair.Key.EventId == eventId && keypair.Value.ManagedDelegate == managedHandler) {
					Events.RemoveHandler (obj, eventId, keypair.Value.NativeHandler);
					return;
				}
			}
		}
	}
	
}
