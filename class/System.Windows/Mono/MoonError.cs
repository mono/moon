//
// MoonError.cs
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
//

using System;
using System.Windows.Markup;
using System.Runtime.InteropServices;

namespace Mono
{
	[StructLayout (LayoutKind.Sequential)]
	internal struct MoonError : IDisposable
	{
		private int number;
		private int code;
		private int char_position;
		private int line_number;
		private IntPtr message;

		private IntPtr gchandle_ptr;
		
		public int Number {
			get { return number; }
		}
		
		public int Code {
			get { return code; }
		}

		public int CharPosition {
			get { return char_position; }
		}

		public int LineNumber {
			get { return line_number; }
		}

		public string Message {
			get { return message == IntPtr.Zero ? null : Marshal.PtrToStringAuto (message); }
		}
		
		public IntPtr GCHandlePtr {
			get { return gchandle_ptr; }
		}
		
		public GCHandle GCHandle {
			get { return GCHandle.FromIntPtr (gchandle_ptr); }
		}
		
		public void Dispose ()
		{
			if (message != IntPtr.Zero) {
				Marshal.FreeHGlobal (message);
				message = IntPtr.Zero;
			}
			
			if (gchandle_ptr != IntPtr.Zero) {
				GCHandle.FromIntPtr (gchandle_ptr).Free ();
				gchandle_ptr = IntPtr.Zero;
			}
		}
		
		public MoonError (Exception ex)
		{
			GCHandle handle = GCHandle.Alloc (ex);
			gchandle_ptr = GCHandle.ToIntPtr (handle);
			number = 9;
			code = 0;
			message = Value.StringToIntPtr (ex.Message);

			XamlParseException p = ex as XamlParseException;
			if (p != null) {
				char_position = p.LinePosition;
				line_number = p.LineNumber;
				code = p.Code;

			} else {
				//System.Console.WriteLine (ex);
				char_position = -1;
				line_number = -1;
			}
		}

		public static MoonError FromIntPtr (IntPtr moon_error)
		{
			MoonError err = (MoonError)Marshal.PtrToStructure (moon_error, typeof (MoonError));
			if (err.message != IntPtr.Zero) {
				string msg = Marshal.PtrToStringAuto (err.message);
				err.message = Value.StringToIntPtr (msg);
			}

			return err;
		}
	}
}
