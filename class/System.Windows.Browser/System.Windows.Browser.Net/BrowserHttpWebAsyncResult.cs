//
// System.Windows.Browser.Net.BrowserHttpWebAsyncResult class
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

#if NET_2_1

using System;
using System.Globalization;
using System.IO;
using System.Net;
using System.Threading;

using Mono;

// I took this implementation from here:
// http://www.bluebytesoftware.com/blog/2006/05/31/ImplementingAHighperfIAsyncResultLockfreeLazyAllocation.aspx
// Thanks Joe!

namespace System.Windows.Browser.Net
{
	sealed class BrowserHttpWebAsyncResult : IAsyncResult, IDisposable
	{
		object state;
		bool completed;
		AsyncCallback callback;
		ManualResetEvent wait_handle;
		Exception exception;

		BrowserHttpWebResponse response;

		public object AsyncState {
			get { return state; }
		}

		public BrowserHttpWebResponse Response {
			get { return response; }
			set { response = value; }
		}

		public WaitHandle AsyncWaitHandle {
			get {
				if (wait_handle != null)
					return wait_handle;

				ManualResetEvent new_handle = new ManualResetEvent (false);
				if (Interlocked.CompareExchange (ref wait_handle, new_handle, null) != null)
					new_handle.Close ();

				if (completed)
					wait_handle.Set ();

				return wait_handle;
			}
		}

		public bool CompletedSynchronously {
			get { return false; }
		}

		public bool IsCompleted {
			get { return completed; }
		}

		public bool HasException {
			get { return exception != null; }
		}

		public Exception Exception {
			get { return exception; }
			set { exception = value; }
		}

		public BrowserHttpWebAsyncResult (AsyncCallback callback, object state)
		{
			this.callback = callback;
			this.state = state;
		}

		public void SetComplete ()
		{
			completed = true;

			Thread.MemoryBarrier ();

			if (wait_handle != null)
				wait_handle.Set ();

			if (callback != null)
				callback (this);
		}

		public void Dispose ()
		{
			if (wait_handle == null)
				return;

			wait_handle.Close ();
		}
	}
}

#endif
