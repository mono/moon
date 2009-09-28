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

using System.IO;

namespace System.Windows.Browser.Net {

	internal class BrowserHttpWebStreamWrapper : Stream {

		bool can_write;
		Stream stream;
		
		public BrowserHttpWebStreamWrapper (Stream s)
		{
			stream = s;
			can_write = s.CanWrite;
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
				 return can_write;
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
			// We need this dummy implementation of a stream since user code
			// is supposed to call Close on the stream to finish the request.
			// problem is that calling Close on a memory stream deletes the data.
			// throw new System.NotImplementedException ();
		}

		public override void SetLength(long value)
		{
			if (!can_write)
				throw new NotSupportedException ();
			stream.SetLength (value);
		}

		public override int Read(byte [] buffer, int offset, int count)
		{
			return stream.Read (buffer, offset, count);
		}

		public override void Write(byte [] buffer, int offset, int count)
		{
			if (!can_write)
				throw new NotSupportedException ();
			stream.Write (buffer, offset, count);
		}

		public override void WriteByte (byte value)
		{
			if (!can_write)
				throw new NotSupportedException ();
			stream.WriteByte (value);
		}

		public override long Seek(long offset, SeekOrigin origin)
		{
			return stream.Seek (offset, origin);
		}


		internal Stream InnerStream {
			get { return stream; }
		}

		internal void SetReadOnly ()
		{
			// we need to turn the stream read-only when we return a stream from
			// BrowserHttpWebResponse.GetResponseStream
			can_write = false;
		}
	}
}

