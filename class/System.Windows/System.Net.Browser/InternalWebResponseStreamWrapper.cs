/*
 * InternalWebResponseStreamWrapper.cs.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008,2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

using System.IO;
using System.Threading;
using System.Windows;

namespace System.Net.Browser {

	internal sealed class InternalWebResponseStreamWrapper : Stream {

		private Stream stream;
		private bool progressive;
		
		internal InternalWebResponseStreamWrapper (Stream s, bool progressive)
		{
			stream = s;
			this.progressive = progressive;
		}

		public override bool CanRead {
			get {
				 return stream.CanRead;
			}
		}

		public override bool CanSeek {
			get {
				 return progressive;
			}
		}

		public override bool CanTimeout {
			get {
				 return !progressive;
			}
		}

		public override bool CanWrite {
			get {
				 return false;
			}
		}

		public override long Length {
			get {
				 return stream.Length;
			}
		}

		public override long Position {
			get {
				 return InnerStream.Position;
			}
			set {
				if (!CanSeek)
					throw new NotSupportedException ();
				InnerStream.Position = value;
			}
		}

		public override void Flush ()
		{
			throw new NotSupportedException ();
		}

		public override void Close ()
		{
			// We cannot call "stream.Close" on a memory stream since it deletes the data
		}

		public override void SetLength (long value)
		{
			throw new NotSupportedException ();
		}

		public override int Read (byte [] buffer, int offset, int count)
		{
			return InnerStream.Read (buffer, offset, count);
		}

		public override void Write (byte [] buffer, int offset, int count)
		{
			throw new NotSupportedException ();
		}

		public override void WriteByte (byte value)
		{
			throw new NotSupportedException ();
		}

		public override long Seek (long offset, SeekOrigin origin)
		{
			if (!CanSeek)
				throw new NotSupportedException ();
			return InnerStream.Seek (offset, origin);
		}

		internal Stream InnerStream {
			get {
				if (!progressive && CheckAccess ())
					throw new NotSupportedException ();
				return stream;
			}
		}

		bool CheckAccess ()
		{
			return (Thread.CurrentThread == DependencyObject.moonlight_thread);
		}
	}
}

