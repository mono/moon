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
		private sealed class EventInfo {
			public EventHandler handler;
			public EventHandler<HtmlEventArgs> handler_args;
			public IntPtr wrapper;
			public HtmlObject obj;
			public GCHandle handle;
			public string event_name;
			public static DomEventCallback callback = new DomEventCallback (DomEventHandler);

			static void DomEventHandler (IntPtr context, string name, int client_x, int client_y, int offset_x, int offset_y, 
				                             bool alt_key, bool ctrl_key, bool shift_key, int mouse_button, 
				                             int key_code, int char_code,
				                             IntPtr domEvent)
			{
				try {
					GCHandle handle = GCHandle.FromIntPtr (context);
					EventInfo info = (EventInfo) handle.Target;
					if (info.handler != null) {
						info.handler (info.obj, EventArgs.Empty);
					} else if (info.handler_args != null) {
						ScriptObject dom = new ScriptObject (ScriptableObjectWrapper.MoonToNPObj (domEvent));
						info.handler_args (info.obj, new HtmlEventArgs (info.obj, client_x, client_y, offset_x, offset_y, 
						                                                alt_key, ctrl_key, shift_key, (MouseButtons) mouse_button, 
						                                                key_code, char_code, name, dom));
					}
				} catch (Exception ex) {
					Console.WriteLine ("Unhandled exception in HtmlObject.EventInfo.DomEventHandler callback: {0}", ex.Message);
					//Console.WriteLine (ex);
				}
			}

			public string EventNameMozilla {
				get {
					if (event_name.StartsWith ("on"))
						return event_name.Substring (2);
					return event_name;
				}
			}

			public void DetachEvent ()
			{
				if (wrapper != IntPtr.Zero) {
					NativeMethods.html_object_detach_event (PluginHost.Handle, EventNameMozilla, wrapper);
					handle.Free ();
					wrapper = IntPtr.Zero;
				}
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

				EventInfo info = new EventInfo ();
				info.handler = handler;
				info.handler_args = handler_args;
				info.obj = obj;
				info.handle = GCHandle.Alloc (info);
				info.event_name = eventName;
				info.wrapper = NativeMethods.html_object_attach_event (PluginHost.Handle,
				                                                       obj.Handle, info.EventNameMozilla, 
				                                                       callback, GCHandle.ToIntPtr (info.handle));

				if (info.wrapper == IntPtr.Zero) {
					info.handle.Free ();
					return null;
				}
				
				return info;
			}
		}

		private Dictionary<string, List<EventInfo>> events;

		
		protected HtmlObject ()
		{
		}

		internal HtmlObject (IntPtr handle)
			: base (handle)
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
				if (info.handler == handler || info.handler_args == handler_args) {
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
