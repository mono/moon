/*
 * StreamCallbacks.cs.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

using System;
using System.IO;
using System.Runtime.InteropServices;

namespace Mono
{
	internal delegate bool Stream_CanSeek (IntPtr handle);
	internal delegate bool Stream_CanRead (IntPtr handle);
	internal delegate long Stream_Length (IntPtr handle);
	internal delegate long Stream_Position (IntPtr handle);
	internal delegate int Stream_Read (IntPtr handle, [In (), Out (), MarshalAs (UnmanagedType.LPArray, SizeParamIndex=3)] byte [] buffer, int offset, int count);
	internal delegate void Stream_Write (IntPtr handle, [In (), Out (), MarshalAs (UnmanagedType.LPArray, SizeParamIndex=3)] byte [] buffer, int offset, int count);
	internal delegate void Stream_Seek (IntPtr handle, long offset, SeekOrigin origin);
	internal delegate void Stream_Close (IntPtr handle);
	
	internal struct ManagedStreamCallbacks
	{
		public IntPtr handle;
		public Stream_CanSeek CanSeek;
		public Stream_CanRead CanRead;
		public Stream_Length Length;
		public Stream_Position Position;
		public Stream_Read Read;
		public Stream_Write Write;
		public Stream_Seek Seek;
		public Stream_Close Close;
	}
}
