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
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace System.Windows.Browser {

	public abstract class HtmlObject : ScriptObject {		
		private class EventInfo {
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
					GCHandle handle = Helper.GCHandleFromIntPtr (context);
					EventInfo info = (EventInfo) handle.Target;
					if (info.handler != null) {
						info.handler (info.obj, EventArgs.Empty);
					} else if (info.handler_args != null) {
						info.handler_args (info.obj, new HtmlEventArgs (info.obj, client_x, client_y, offset_x, offset_y, 
						                                                alt_key, ctrl_key, shift_key, (MouseButtons) mouse_button, 
						                                                key_code, char_code, name, domEvent));
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
					NativeMethods.html_object_detach_event (WebApplication.Current.PluginHandle, EventNameMozilla, wrapper);
					handle.Free ();
					wrapper = IntPtr.Zero;
				}
			}
			
			public static EventInfo AttachEvent (string eventName, EventHandler handler, EventHandler<HtmlEventArgs> handler_args, HtmlObject obj)
			{
				EventInfo info;

				info = new EventInfo ();
				info.handler = handler;
				info.handler_args = handler_args;
				info.obj = obj;
				info.handle = GCHandle.Alloc (info);
				info.event_name = eventName;
				info.wrapper = NativeMethods.html_object_attach_event (WebApplication.Current.PluginHandle, 
				                                                       obj.Handle, info.EventNameMozilla, 
				                                                       callback, Helper.GCHandleToIntPtr (info.handle));

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
		
		protected virtual object ConvertTo (Type targetType, bool allowSerialization)
		{
			throw new NotImplementedException ();
		}

		internal static T GetPropertyInternal<T> (IntPtr handle, string name)
		{
			Mono.Value res;
			NativeMethods.html_object_get_property (WebApplication.Current.PluginHandle, handle, name, out res);
			if (res.k != Mono.Kind.INVALID) {
				var o = ScriptableObjectWrapper.ObjectFromValue<T> (res);
				return (T) o;
			}

			return default (T);
		}

		internal static void SetPropertyInternal (IntPtr handle, string name, object value)
		{
			Mono.Value dp = new Mono.Value ();
			ScriptableObjectWrapper.ValueFromObject (ref dp, value);
			NativeMethods.html_object_set_property (WebApplication.Current.PluginHandle, handle, name, ref dp);
		}

		internal static T InvokeInternal<T> (IntPtr handle, string name, params object [] args)
		{
			Mono.Value res;
			Mono.Value [] vargs = new Mono.Value [args.Length];

			for (int i = 0; i < args.Length; i++)
				ScriptableObjectWrapper.ValueFromObject (ref vargs [i], args [i]);

			NativeMethods.html_object_invoke (WebApplication.Current.PluginHandle, handle, name, vargs, (uint) args.Length, out res);

			if (res.k != Mono.Kind.INVALID) {
				object o = ScriptableObjectWrapper.ObjectFromValue<T> (res);
				if (o is int && typeof(T) == typeof(object)) {
					// When the target type is object, SL converts ints to doubles to wash out
					// browser differences. (Safari apparently always returns doubles, FF
					// ints and doubles, depending on the value).
					// See: http://msdn.microsoft.com/en-us/library/cc645079(VS.95).aspx
					o = (double) (int) o;
				}
				return (T) o;
			}

			return default (T);
		}

		internal static T InvokeInternal<T> (IntPtr handle, params object [] args)
		{
			Mono.Value res;
			Mono.Value [] vargs = new Mono.Value [args.Length];

			for (int i = 0; i < args.Length; i++)
				ScriptableObjectWrapper.ValueFromObject (ref vargs [i], args [i]);

			NativeMethods.html_object_invoke_self (WebApplication.Current.PluginHandle, handle, vargs, (uint) args.Length, out res);

			if (res.k != Mono.Kind.INVALID) {
				object o = ScriptableObjectWrapper.ObjectFromValue<T> (res);
				if (o is int && typeof(T) == typeof(object)) {
					// When the target type is object, SL converts ints to doubles to wash out
					// browser differences. (Safari apparently always returns doubles, FF
					// ints and doubles, depending on the value).
					// See: http://msdn.microsoft.com/en-us/library/cc645079(VS.95).aspx
					o = (double) (int) o;
				}
				return (T) o;
			}

			return default (T);
		}
	}
}
