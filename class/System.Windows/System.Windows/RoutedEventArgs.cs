//
// RoutedEventArgs.cs
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

using System.Security;
using Mono;

namespace System.Windows {

	public class RoutedEventArgs : EventArgs {
		internal IntPtr native;

		internal RoutedEventArgs (IntPtr raw)
		{
			native = raw;
			NativeMethods.base_ref (native);
		}

		~RoutedEventArgs ()
		{
			if (this.native != IntPtr.Zero) {
				NativeMethods.base_unref (this.native);
				this.native = IntPtr.Zero;
			}
		}

		public RoutedEventArgs () : this (NativeMethods.routed_event_args_new ())
		{
		}

		object source;
		public object Source {
			//[SecuritySafeCritical]
			get { return source; }
			//[SecuritySafeCritical]
			set {
				// not sure what to do here...  test
				// if there's an exception if you set
				// it to a non-DO.
				DependencyObject v = value as DependencyObject;
				if (v == null)
					NativeMethods.routed_event_args_set_source (native, IntPtr.Zero);
				else
					NativeMethods.routed_event_args_set_source (native, v.native);
				source = value;
			}
		}
	}
}
