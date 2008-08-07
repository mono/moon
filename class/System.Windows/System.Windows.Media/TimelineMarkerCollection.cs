// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2007 Novell, Inc.
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
using System.Windows;
using Mono;

namespace System.Windows.Media {
	public sealed class TimelineMarkerCollection : PresentationFrameworkCollection<TimelineMarker> {
		public TimelineMarkerCollection () : base (NativeMethods.timeline_marker_collection_new ())
		{
		}
		
		internal TimelineMarkerCollection (IntPtr raw) : base (raw)
		{
		}
		
		internal override Kind GetKind ()
		{
			return Kind.TIMELINEMARKER_COLLECTION;
		}

		public override void Add (TimelineMarker value)
		{
			AddImpl (value);
		}
		
		public override bool Contains (TimelineMarker value)
		{
			return ContainsImpl (value);
		}
		
		public override int IndexOf (TimelineMarker value)
		{
			return IndexOfImpl (value);
		}
		
		public override void Insert (int index, TimelineMarker value)
		{
			InsertImpl (index, value);
		}
		
		public override bool Remove (TimelineMarker value)
		{
			return RemoveImpl (value);
		}
		
		public override TimelineMarker this[int index] {
			get { return GetItemImpl (index); }
			set { SetItemImpl (index, value); }
		}
	}
}
