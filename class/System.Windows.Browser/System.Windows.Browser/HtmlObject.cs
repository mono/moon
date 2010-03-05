//
// System.Windows.Browser.HtmlObject class
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2007 Novell, Inc (http://www.novell.com)
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
using System.Runtime.InteropServices;
using System.Windows.Interop;

namespace System.Windows.Browser {

	public abstract class HtmlObject : ScriptObject {

		private class DOMEventListener {
			public DOMEventListener (HtmlObject obj, string name, EventHandler handler, EventHandler<HtmlEventArgs> handler_args)
			{
				this.obj = obj;
				this.name = name;
				this.handler = handler;
				this.handler_args = handler_args;
			}

			[ScriptableMember (ScriptAlias = "handleEvent")]
			public void HandleEvent (ScriptObject eventObject)
			{
				try {
					if (handler != null) {
						handler (obj, EventArgs.Empty);
					} else if (handler_args != null) {
						handler_args (obj, eventObject == null ? null : new HtmlEventArgs (obj, eventObject));
					}
				} catch (Exception ex) {
					Console.WriteLine ("Unhandled exception in DOMEventListener.HandleEvent: {0}", ex.Message);
					//Console.WriteLine (ex);
				}
			}

			public HtmlObject obj;
			public string name;
			public EventHandler handler;
			public EventHandler<HtmlEventArgs> handler_args;
		}

		private sealed class EventInfo {
			public EventInfo (HtmlObject obj, string eventName, EventHandler handler, EventHandler<HtmlEventArgs> handler_args)
			{
				this.obj = obj;
				this.event_name = eventName;
				listener = new DOMEventListener (obj, EventNameMozilla, handler, handler_args);
			}

			public string EventNameMozilla {
				get {
					if (event_name.StartsWith ("on"))
						return event_name.Substring (2);
					return event_name;
				}
			}

			public void AttachEvent ()
			{
				obj.Invoke ("addEventListener", new object[] { EventNameMozilla, listener, false });
			}

			public void DetachEvent ()
			{
				obj.Invoke ("removeEventListener", new object[] { EventNameMozilla, listener, false });
			}

			static internal void CheckEvent (string eventName, EventHandler handler, EventHandler<HtmlEventArgs> handler_args)
			{
				if (eventName == null)
					throw new ArgumentNullException ("eventName");
				if ((eventName.Length == 0) || (eventName.IndexOf ('\0') != -1))
					throw new ArgumentException ("eventName");
				if ((handler == null) && (handler_args == null))
					throw new ArgumentNullException ("handler");
			}
			
			public static EventInfo AttachEvent (string eventName, EventHandler handler, EventHandler<HtmlEventArgs> handler_args, HtmlObject obj)
			{
				CheckEvent (eventName, handler, handler_args);

				EventInfo info = new EventInfo (obj, eventName, handler, handler_args);

				info.AttachEvent ();

				return info;
			}

			public HtmlObject obj;
			public string event_name;
			public DOMEventListener listener;
		}

		private Dictionary<string, List<EventInfo>> events;

		
		protected HtmlObject ()
		{
		}

		internal HtmlObject (IntPtr handle, bool handleIsScriptableNPObject)
			: base (handle, handleIsScriptableNPObject)
		{
		}

		public bool AttachEvent (string eventName, EventHandler handler)
		{
			return AddEventInfo (EventInfo.AttachEvent (eventName, handler, null, this));
		}

		public bool AttachEvent (string eventName, EventHandler<HtmlEventArgs> handler)
		{
			return AddEventInfo (EventInfo.AttachEvent (eventName, null, handler, this));
		}

		private bool AddEventInfo (EventInfo info)
		{
			List<EventInfo> list;

			if (info == null)
				return false;

			if (events == null)
				events = new Dictionary<string, List<EventInfo>> ();
			
			if (!events.TryGetValue (info.event_name, out list)) {
				list = new List<EventInfo> ();
				events.Add (info.event_name, list);
			}
			list.Add (info);
			return true;
		}
		
		public void DetachEvent (string eventName, EventHandler handler)
		{
			DetachEvent (eventName, handler, null);
		}

		public void DetachEvent (string eventName, EventHandler<HtmlEventArgs> handler)
		{
			DetachEvent (eventName, null, handler);
		}

		private void DetachEvent (string eventName, EventHandler handler, EventHandler<HtmlEventArgs> handler_args)
		{
			EventInfo.CheckEvent (eventName, handler, handler_args);

			List<EventInfo> list;

			if (events == null)
				return;
			
			if (!events.TryGetValue (eventName, out list)) {
				return;
			}

			for (int i = list.Count - 1; i >= 0; i--) {
				EventInfo info = list [i];
				if (info.listener.handler == handler || info.listener.handler_args == handler_args) {
					list.RemoveAt (i);
					info.DetachEvent ();
					// Do we continue looking for duplicates?
					// break;
				}
			}
		}

		protected override object ConvertTo (Type targetType, bool allowSerialization)
		{
			// documented as "not supported" in SL2 and to throw a ArgumentException "in all cases"
			// not quite true since a null targetType throws a NRE but otherwise seems correct
			throw new ArgumentException (targetType.ToString ());
		}
	}
}
