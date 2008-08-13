//
// DispatcherTimer.cs
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
using System.Security;
using System.Threading;

namespace System.Windows.Threading {

	public class DispatcherSynchronizationContext : System.Threading.SynchronizationContext
#if notyet
	: SynchronizationContext
#endif
	{
		public DispatcherSynchronizationContext (Dispatcher dispatcher)
		{
			throw new NotImplementedException ();
		}

#if NET_2_1
		[SecuritySafeCritical]
#endif
		public DispatcherSynchronizationContext ()
		{
			throw new NotImplementedException ();
		}

#if notyet
		public SynchronizationContext CreateCopy ()
		{
			throw new NotImplementedException ();
		}
#endif

		public virtual void Send (SendOrPostCallback d, object state)
		{
			throw new NotImplementedException ();
		}

#if NET_2_1
		[SecuritySafeCritical]
#endif
		public virtual void Post (SendOrPostCallback d, object state)
		{
			throw new NotImplementedException ();
		}
	}
}