//
// GroupEnumerator.cs
//
// Authors:
//	Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2010 Novell, Inc.  http://www.novell.com
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
using System.Windows.Data;
using System.Collections.Generic;

namespace System.Windows.Data {

	class GroupEnumerator : IEnumerator<object> {

		public object Current {
			get; set;
		}

		public int CurrentGroupIndex {
			get; set;
		}

		CollectionViewGroup CurrentGroup {
			get; set;
		}

		List<object> Groups {
			get; set;
		}

		CollectionViewGroup Root {
			get; set;
		}

		public GroupEnumerator (CollectionViewGroup root)
		{
			Groups = new List<object> ();
			Root = root;
			Reset ();
		}

		public void Dispose ()
		{

		}

		public bool MoveNext ()
		{
			if (CurrentGroup != null && CurrentGroupIndex  < CurrentGroup.Items.Count) {
				Current = CurrentGroup.Items [CurrentGroupIndex ++];
				return true;
			}

			while (Groups.Count > 0) {
				var group = Groups [0];
				Groups.RemoveAt (0);
				if (!(group is CollectionViewGroup)) {
					Current = group;
					return true;
				}
				var g = (CollectionViewGroup) group;
				if (g.IsBottomLevel && g.Items.Count > 0) {
					CurrentGroup = g;
					Current = g.Items [0];
					CurrentGroupIndex = 1;
					return true;
				} else {
					for (int i = 0; i < g.Items.Count; i ++)
						Groups.Insert (i, g.Items [i]);
				}
			}

			Current = null;
			return false;
		}

		public void Reset ()
		{
			Groups.Clear ();
			Groups.Add (Root);
		}
	}
}

