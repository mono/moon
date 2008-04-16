//
// System.IO.SimpleUnmanagedMemoryStream.cs
//
// Copyright (C) 2006 Sridhar Kulkarni, All Rights Reserved
//
// Authors:
// 	Sridhar Kulkarni (sridharkulkarni@gmail.com)
// 	Gert Driesen (drieseng@users.sourceforge.net)
//
// Copyright (C) 2005-2006 Novell, Inc (http://www.novell.com)
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
using System.IO;
using System.Runtime.InteropServices;

namespace System.IO
{
	public class SimpleUnmanagedMemoryStream : Stream
	{
		long length;
		long capacity;
		IntPtr initial_pointer;
		long initial_position;
		long current_position;
		
		protected SimpleUnmanagedMemoryStream()
		{
			initial_position = 0;
			current_position = initial_position;
		}
		
		public unsafe SimpleUnmanagedMemoryStream (byte *pointer, long length)
		{
			if (pointer == null)
				throw new ArgumentNullException("pointer");
			if (length < 0)
				throw new ArgumentOutOfRangeException("length", "Non-negative number required.");
			this.length = length;
			capacity = length;
			initial_position = 0;
			current_position = initial_position;
			initial_pointer = new IntPtr((void*)pointer);
		}
	
		public override bool CanRead {
			get {
				return true;
			}
		}

		public override bool CanSeek {
			get {
				return true;
			}
		}
		
		public override bool CanWrite {
			get {
				return false;
			}
		}

		public override long Length {
			get {
				return (length);
			}
		}
		public override long Position {
			get {
				return (current_position);
			}
			set {
				if (value < 0)
					throw new ArgumentOutOfRangeException("value", "Non-negative number required.");
				if (value > (long)Int32.MaxValue)
					throw new ArgumentOutOfRangeException("value", "The position is larger than Int32.MaxValue.");
				current_position = value;
			}
		}

		public override int Read ([InAttribute] [OutAttribute] byte[] buffer, int offset, int count)
		 {
			 if (buffer == null)
				 throw new ArgumentNullException("buffer");
			 if (offset < 0)
				 throw new ArgumentOutOfRangeException("offset", "Non-negative number required.");
			 if (count < 0)
				 throw new ArgumentOutOfRangeException("count", "Non-negative number required.");
			 if ((buffer.Length - offset) < count)
				 throw new ArgumentException("The length of the buffer array minus the offset parameter is less than the count parameter");
			 
			 if (current_position >= length)
				 return (0);
			 else {
				 int progress = current_position + count < length ? count : (int) (length - current_position);
				 for (int i = 0; i < progress; i++)
					 buffer [offset + i] = Marshal.ReadByte (initial_pointer, (int) current_position++);
				 return progress;
			 }
		 }

		public override int ReadByte ()
		{
			if (current_position >= length)
				return (-1);
			return (int) Marshal.ReadByte(initial_pointer, (int) current_position++);
		}

		public override long Seek (long offset,	SeekOrigin loc)
		{
			long refpoint;
			switch(loc) {
			case SeekOrigin.Begin:
				if (offset < 0)
					throw new IOException("An attempt was made to seek before the beginning of the stream");
				refpoint = initial_position;
				break;
			case SeekOrigin.Current:
				refpoint = current_position;
				break;
			case SeekOrigin.End:
				refpoint = length;
				break;
			default:
				throw new ArgumentException("Invalid SeekOrigin option");
			}
			refpoint += (int)offset;
			if (refpoint < initial_position)
				throw new IOException("An attempt was made to seek before the beginning of the stream");
			current_position = refpoint;
			return(current_position);
		}
		 
		public override void SetLength (long value)
		{
			if (value < 0)
				throw new ArgumentOutOfRangeException("length", "Non-negative number required.");
			if (value > capacity)
				throw new IOException ("Unable to expand length of this stream beyond its capacity.");
			length = value;
			if (length < current_position)
				current_position = length;
		}

		public override void Flush ()
		{
		}
		 
		protected override void Dispose (bool disposing)
		{
		}
		 
		public override void Write (byte[] buffer, int offset, int count)
		{
			throw new NotSupportedException ("Stream does not support writing.");
		}
		
		public override void WriteByte (byte value)
		{
			 throw new NotSupportedException("Stream does not support writing.");
		}
	}
}

