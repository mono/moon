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
		DependencyObject source;
		bool source_set;

		internal RoutedEventArgs (IntPtr raw)
		{
			native = raw;
			NativeMethods.event_object_ref (native);
		}

		~RoutedEventArgs ()
		{
			if (native != IntPtr.Zero) {
				NativeMethods.event_object_unref (native);
				native = IntPtr.Zero;
			}
		}

		public RoutedEventArgs () : this (NativeMethods.routed_event_args_new ())
		{
		}

		public object OriginalSource {
			get {
				if (source_set)
					return source;

				return DependencyObject.FromIntPtr (NativeMethods.routed_event_args_get_source (native));
			}

			[SecuritySafeCritical]
			internal set {
				if (value == null) {
					NativeMethods.routed_event_args_set_source (native, IntPtr.Zero);
					source = null;
					source_set = false;
					return;
				}

				DependencyObject v = value as DependencyObject;
				if (v == null)
					throw new ArgumentException ();

				source_set = true;
				source = v;

				NativeMethods.routed_event_args_set_source (native, v.native);
			}
		}
	}
}
