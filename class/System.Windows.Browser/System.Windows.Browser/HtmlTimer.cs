//
// System.Windows.Browser.HtmLTimer class
//
// Authors:
//	Miguel de Icaza  <miguel@ximian.com>
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
using System;
using Mono;
using System.Windows.Interop;

namespace System.Windows.Browser
{
	public class HtmlTimer
	{
		
		uint source_id;
		NativeMethods.GSourceFunc callback;
		
		public HtmlTimer ()
		{
		}

		public bool Enabled {
			set {
				if (value)
					Start ();
				else
					Stop ();
			}
			
			get {
				return source_id != 0;
			}
		}

		int interval;
		public int Interval {
			set {
				if (value < 1)
					throw new ArgumentOutOfRangeException ("value");

				interval = value;
				if (source_id != 0){
					Stop ();
					Start ();
				}
			}

			get {
				return interval;
			}
		}

		bool timer_callback (IntPtr data)
		{
			OnTick (EventArgs.Empty);

			// If we are killed by Enabled or Stop, still return that value
			return source_id != 0;
		}
		
		public void Start ()
		{
			if (source_id != 0)
				return;

			callback = new NativeMethods.GSourceFunc (timer_callback);
			IntPtr handle = PluginHost.Handle;

			if (handle != IntPtr.Zero)
				source_id = NativeMethods.plugin_html_timer_timeout_add (
					handle, interval, callback, IntPtr.Zero);
			else
				source_id = NativeMethods.runtime_html_timer_timeout_add (
					interval, callback, IntPtr.Zero);
		}

		public void Stop ()
		{
			if (source_id == 0)
				return;

			IntPtr handle = PluginHost.Handle;
			if (handle != IntPtr.Zero)
				NativeMethods.plugin_html_timer_timeout_stop (PluginHost.Handle, source_id);
			else
				NativeMethods.runtime_html_timer_timeout_stop (source_id);
			source_id = 0;
			callback = null;
		}

		public override string ToString ()
		{
			return String.Format ("Enabled: {0}, Interval: {1}", Enabled, Interval);
		}

		protected virtual void OnTick (EventArgs e)
		{
			// Keep a reference, in case the callback changes the source.
			NativeMethods.GSourceFunc my_callback = callback;

			if (Tick != null)
				Tick (this, e);
			
		}

		public event EventHandler Tick;
	}
}

