/*
 * ObjectKeyFrameCollection.cs.
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
using System.Windows;

namespace System.Windows.Media.Animation
{
	public sealed partial class ObjectKeyFrameCollection : PresentationFrameworkCollection<ObjectKeyFrame> {

		public override void Add (ObjectKeyFrame value)
		{
			AddImpl (value);
		}
		
		public override bool Contains (ObjectKeyFrame value)
		{
			return ContainsImpl (value);
		}
		
		public override int IndexOf (ObjectKeyFrame value)
		{
			return IndexOfImpl (value);
		}
		
		public override void Insert (int index, ObjectKeyFrame value)
		{
			InsertImpl (index, value);
		}
		
		public override bool Remove (ObjectKeyFrame value)
		{
			return RemoveImpl (value);
		}
		
		public override ObjectKeyFrame this[int index] {
			get { return GetItemImpl (index); }
			set { SetItemImpl (index, value); }
		}
	}
}
