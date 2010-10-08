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

		Dictionary<string, Dictionary<object,DOMEventListener>> events;

		
		protected HtmlObject ()
		{
		}

		internal HtmlObject (IntPtr handle, bool handleIsScriptableNPObject)
			: base (handle, handleIsScriptableNPObject)
		{
		}

		internal static void CheckEvent (string eventName, EventHandler handler, EventHandler<HtmlEventArgs> handler_args)
		{
			if (eventName == null)
				throw new ArgumentNullException ("eventName");
			if ((eventName.Length == 0) || (eventName.IndexOf ('\0') != -1))
				throw new ArgumentException ("eventName");
			if ((handler == null) && (handler_args == null))
				throw new ArgumentNullException ("handler");
		}

		public bool AttachEvent (string eventName, EventHandler handler)
		{
			CheckEvent (eventName, handler, null);

			DOMEventListener listener = new DOMEventListener (this, eventName, handler);

			listener.AttachEvent();

			return AddListener (handler, listener);
		}

		public bool AttachEvent (string eventName, EventHandler<HtmlEventArgs> handler)
		{
			CheckEvent (eventName, null, handler);

			DOMEventListener listener = new DOMEventListener (this, eventName, handler);

			listener.AttachEvent();

			return AddListener (handler, listener);
		}

		bool AddListener (object handler, DOMEventListener listener)
		{
			Dictionary<object,DOMEventListener> listeners;

			if (listener == null)
				return false;

			if (events == null)
				events = new Dictionary<string, Dictionary<object,DOMEventListener>> ();
			
			if (!events.TryGetValue (listener.EventName, out listeners)) {
				listeners = new Dictionary<object,DOMEventListener> ();
				events.Add (listener.EventName, listeners);
			}
			listeners[handler] = listener;
			return true;
		}
		
		public void DetachEvent (string eventName, EventHandler handler)
		{
			CheckEvent (eventName, handler, null);

			DetachListener (eventName, handler);
			
		}

		public void DetachEvent (string eventName, EventHandler<HtmlEventArgs> handler)
		{
			CheckEvent (eventName, null, handler);

			DetachListener (eventName, handler);
		}

		void DetachListener (string eventName, object handler)
		{
			Dictionary<object,DOMEventListener> listeners;

			if (events == null)
				return;
			
			if (!events.TryGetValue (eventName, out listeners))
				return;

			DOMEventListener listener;
			if (!listeners.TryGetValue (handler, out listener))
				return;

			listener.DetachEvent();

			listeners.Remove (handler);
		}

		protected internal override object ConvertTo (Type targetType, bool allowSerialization)
		{
			// documented as "not supported" in SL2 and to throw a ArgumentException "in all cases"
			// not quite true since a null targetType throws a NRE but otherwise seems correct
			throw new ArgumentException (targetType.ToString ());
		}

		class DOMEventListener {

			HtmlObject obj;
			string name;
			EventHandler handler;
			EventHandler<HtmlEventArgs> handler_args;

			public DOMEventListener (HtmlObject obj, string name, EventHandler<HtmlEventArgs> handler_args) : this (obj, name)
			{
				this.handler_args = handler_args;
			}

			public DOMEventListener (HtmlObject obj, string name, EventHandler handler) : this (obj, name)
			{
				this.handler = handler;
			}

			public DOMEventListener (HtmlObject obj, string name)
			{
				this.obj = obj;
				this.name = name;
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

			public string EventName {
				get { return name; }
			}

			string EventNameMozilla {
				get {
					if (name.StartsWith ("on"))
						return name.Substring (2);
					return name;
				}
			}

			public void AttachEvent ()
			{
				obj.Invoke ("addEventListener", new object[] { EventNameMozilla, this, false });
			}

			public void DetachEvent ()
			{
				obj.Invoke ("removeEventListener", new object[] { EventNameMozilla, this, false });
			}
		}
	}
}
