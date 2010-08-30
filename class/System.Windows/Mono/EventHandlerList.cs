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
	sealed class EventHandlerData {
		public int Token;
		public Delegate ManagedDelegate;
		public GCHandle NativeHandler;
		public GDestroyNotify DtorAction;
	}

	sealed class EventHandlerList : Dictionary<int,Dictionary<int,EventHandlerData>> {
		public void AddHandler (int eventId, int token, Delegate managedDelegate, UnmanagedEventHandler nativeHandler)
		{
			AddHandler (eventId, token, managedDelegate, nativeHandler);
		}

		public void AddHandler (int eventId, int token, Delegate managedDelegate, UnmanagedEventHandler nativeHandler, GDestroyNotify dtor_action)
		{
			Dictionary<int, EventHandlerData> events;

			if (ContainsKey (eventId)) {
				events = this[eventId];
			}
			else {
				events = new Dictionary<int, EventHandlerData>();
				Add (eventId, events);
			}

			events.Add (token, new EventHandlerData () {
						Token = token,
						ManagedDelegate = managedDelegate,
						NativeHandler = GCHandle.Alloc (nativeHandler),
						DtorAction = dtor_action });
		}

		public void RemoveHandler (int eventId, Delegate managedDelegate)
		{
			if (ContainsKey (eventId)) {
				Dictionary<int, EventHandlerData> events = this[eventId];

				foreach (EventHandlerData data in events.Values) {
					if (data.ManagedDelegate == managedDelegate) {
						events.Remove (data.Token);

						UnmanagedEventHandler nativeHandler = data.NativeHandler.Target as UnmanagedEventHandler;

						data.NativeHandler.Free ();

						return;
					}
				}
			}
		}

		public UnmanagedEventHandler LookupHandler (int eventId, Delegate managedDelegate)
		{
			if (ContainsKey (eventId)) {
				Dictionary<int, EventHandlerData> events = this[eventId];

				foreach (EventHandlerData data in events.Values) {
					if (data.ManagedDelegate == managedDelegate)
						return data.NativeHandler.Target as UnmanagedEventHandler;
				}
			}

			return null;
		}

	}
	
}
