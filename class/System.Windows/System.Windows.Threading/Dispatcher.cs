//
// Dispatcher.cs
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
using System.ComponentModel;
using System.Security;
using System.Threading;
using Mono;

namespace System.Windows.Threading {

	[CLSCompliant (false)]
	public class Dispatcher {
		private Action a;
		private Delegate d;
		private object[] args;
		NativeMethods.GSourceFunc callback;

		internal Dispatcher ()
		{
		}

		[EditorBrowsable (EditorBrowsableState.Never)]
#if NET_2_1
		[SecuritySafeCritical]
#endif
		public bool CheckAccess ()
		{
			throw new NotImplementedException ();
		}

		public DispatcherOperation BeginInvoke (Action a)
		{
			this.a = a;
			return BeginInvoke ();
		}

		public DispatcherOperation BeginInvoke (Delegate d, object[] args)
		{
			this.d = d;
			this.args = args;
			return BeginInvoke ();
		}
		
		private DispatcherOperation BeginInvoke ()
		{
			callback = new NativeMethods.GSourceFunc (dispatcher_callback);
			return new DispatcherOperation (NativeMethods.runtime_idle_add (callback, IntPtr.Zero));
                }

		bool dispatcher_callback (IntPtr data)
		{
			if (a != null)
				a.Invoke ();
			else if (d != null)
				d.DynamicInvoke (args);

			return false;
		}

	}

}
