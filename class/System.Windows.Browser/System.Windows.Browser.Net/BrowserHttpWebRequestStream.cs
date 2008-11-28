/*
 * BrowserHttpWebRequestStream.cs.
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

namespace System.Windows.Browser.Net {
	internal class BrowserHttpWebRequestStream : Stream	{
		// We need this dummy implementation of a stream since user code
		// is supposed to call Close on the stream to finish the request.
		// problem is that calling Close on a memory stream deletes the data.
		internal MemoryStream stream = new MemoryStream ();
		
		public BrowserHttpWebRequestStream ()
		{
		}

		public override bool CanRead {
			get {
				 return stream.CanRead;
			}
		}

		public override bool CanSeek {
			get {
				 return stream.CanSeek;
			}
		}

		public override bool CanWrite {
			get {
				 return stream.CanWrite;
			}
		}

		public override long Length {
			get {
				 return stream.Length;
			}
		}

		public override long Position {
			get {
				 return stream.Position;
			}
			set {
				stream.Position = value;
			}
		}

		public override void Flush()
		{
			stream.Flush ();
		}

		public override void Close()
		{
			// throw new System.NotImplementedException ();
		}

		public override void SetLength(long value)
		{
			stream.SetLength (value);
		}

		public override int Read(byte [] buffer, int offset, int count)
		{
			return stream.Read (buffer, offset, count);
		}

		public override void Write(byte [] buffer, int offset, int count)
		{
			stream.Write (buffer, offset, count);
		}

		public override long Seek(long offset, SeekOrigin origin)
		{
			return stream.Seek (offset, origin);
		}

	}
}
