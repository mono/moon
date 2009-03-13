//
// MediaStreamSample.cs
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
using System.Collections.Generic;
using System.IO;

namespace System.Windows.Media
{
	public class MediaStreamSample
	{
		private IDictionary <MediaSampleAttributeKeys, string> attributes;
		private long count;
		private MediaStreamDescription media_stream_description;
		private long offset;
		private Stream stream;
		private long timestamp;
		
		public MediaStreamSample (MediaStreamDescription mediaStreamDescription, Stream stream, long offset, long count, long timestamp, IDictionary<MediaSampleAttributeKeys, string> attributes)
		{
			this.media_stream_description = mediaStreamDescription;
			this.stream = stream;
			this.offset = offset;
			this.count = count;
			this.timestamp = timestamp;
			this.attributes = attributes;
		}
		
		public IDictionary<MediaSampleAttributeKeys, string> Attributes {
			get { return attributes; }
		}
		
		public long Count { 
			get { return count; }
		}
		
		public MediaStreamDescription MediaStreamDescription { 
			get { return media_stream_description; }
		}
		
		public long Offset { 
			get { return offset; }
		}
		
		public Stream Stream { 
			get { return stream; }
		}
		
		public long Timestamp { 
			get { return timestamp; }
		}
	}
}
