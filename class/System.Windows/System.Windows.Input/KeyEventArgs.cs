//
// KeyEventArgs.cs
//
// Author:
//   Miguel de Icaza (miguel@novell.com)
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
using System.Windows;
using System.Security;
using Mono;

namespace System.Windows.Input {

	public sealed class KeyEventArgs : RoutedEventArgs {
		Key key;
		int platform_key_code;

		public KeyEventArgs ()
		{
		}

		internal KeyEventArgs (bool ctrl, bool shift, int key, int platform_key_code)
		{
			this.key = (Key)key;
			this.platform_key_code = platform_key_code;
			//this.ctrl = ctrl;
			//this.shift = shift;
		}

		public bool Handled {
#if NET_2_1
			[SecuritySafeCritical]
#endif
			get { return NativeMethods.keyboard_event_args_get_handled (native); }
#if NET_2_1
			[SecuritySafeCritical]
#endif
			set { NativeMethods.keyboard_event_args_set_handled (native, value); }
		}

		public Key Key {
#if NET_2_1
			[SecuritySafeCritical]
#endif
			get { return key; }
		}

		public int PlatformKeyCode {
#if NET_2_1
			[SecuritySafeCritical]
#endif
			get { return platform_key_code; }
		}
	}
}
