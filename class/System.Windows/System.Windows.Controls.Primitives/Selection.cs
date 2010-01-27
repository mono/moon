//
// Selection.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2010 Novell, Inc.
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

namespace System.Windows.Controls.Primitives
{
	class Selection  {

		object selectedItem;

		Selector Owner {
			get; set;
		}

		public SelectionMode Mode {
			get; set;
		}

		object SelectedItem {
			get { return selectedItem; }
			set {
				if (SelectedItem != value) {
					object oldVal = SelectedItem;
					selectedItem = value;
					Owner.RaiseSelectionChanged (oldVal, SelectedItem);
				}
			}
		}

		public bool Updating {
			get; private set;
		}

		public Selection (Selector owner)
		{
			Owner = owner;
		}

		public void ClearSelection ()
		{
			try {
				Updating = true;
				Owner.SelectedItem = null;
				Owner.SelectedIndex = -1;
				SelectedItem = null;
			} finally {
				Updating = false;
			}
		}

		public void Select (object item)
		{
			if (item == null) {
				Console.WriteLine ("It's a null");
				Console.ReadLine ();
			}
			try {
				Updating = true;
				Owner.SelectedItem = item;
				Owner.SelectedIndex  = Owner.Items.IndexOf (item);
				SelectedItem = item;
			} finally {
				Updating = false;
			}
		}
	}
}

