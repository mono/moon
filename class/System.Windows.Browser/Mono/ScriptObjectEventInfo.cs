// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2008 Novell, Inc (http://www.novell.com)
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
using System.ComponentModel;
using System.Globalization;
using System.Collections.Generic;
using System.Text;
using System.Windows.Interop;
using System.Windows.Browser;
using System.Runtime.InteropServices;
using System.Reflection;

namespace Mono
{
	sealed class ScriptObjectEventInfo
	{
		System.Delegate _delegate;
		Type type;
		EventInfo _event;
		public EventInfo EventInfo {
			get { return _event; }
			set {
				_event = value;
				type = (value != null ? value.EventHandlerType : null);
			}
		}

		public ScriptObjectEventInfo (string name, ScriptObject callback, EventInfo ei)
			: this (callback)
		{
			Name = name;
			EventInfo = ei;
		}

		// for js event handlers
		public ScriptObjectEventInfo (ScriptObject callback, Type type)
			: this (callback)
		{
			this.type = type;
		}

		// extract common [SecurityCritical] parts of the public .ctors
		private ScriptObjectEventInfo (ScriptObject callback)
		{
			if (callback == null)
				throw new ArgumentNullException ("callback");

			Callback = callback;
			NativeMethods.html_object_retain (PluginHost.Handle, Callback.Handle);
		}

		~ScriptObjectEventInfo ()
		{
			if (Callback.Handle != IntPtr.Zero)
				NativeMethods.html_object_release (PluginHost.Handle, Callback.Handle);
		}

		public ScriptObject Callback {
			get; private set;
		}

		public string Name {
			get; private set;
		}

		public Delegate GetDelegate ()
		{
			if (_delegate == null)
				_delegate = Delegate.CreateDelegate (type, this,
								GetType ().GetMethod ("HandleEvent",
								BindingFlags.Instance | BindingFlags.NonPublic));
			return _delegate;
		}
				
		private void HandleEvent (object sender, EventArgs args)
		{
			Callback.InvokeSelf (sender, args);
		}
	}
}
