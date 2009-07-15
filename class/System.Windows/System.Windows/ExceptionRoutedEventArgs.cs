//
// ExceptionRoutedEventArgs.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
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

using Mono;

namespace System.Windows
{
	public sealed class ExceptionRoutedEventArgs : RoutedEventArgs
	{
		private Exception error_exception;
		
		internal ExceptionRoutedEventArgs (IntPtr raw) : base (raw)
		{
			string msg = NativeMethods.error_event_args_get_error_message (raw);
			int code = NativeMethods.error_event_args_get_error_code (raw);
			int type = NativeMethods.error_event_args_get_error_type (raw);
			
			error_exception = new Exception (msg); // TODO: find out which kind of exception we need to create here
		}

		public Exception ErrorException {
			get { return error_exception; }				
		}
	}
}
