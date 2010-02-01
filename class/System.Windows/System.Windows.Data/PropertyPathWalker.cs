//
// PropertyPathWalker.cs
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
using System.Linq;
using System.Collections.Generic;

namespace System.Windows
{
	public class PropertyPathWalker {

		public event EventHandler ValueChanged;

		public IPropertyPathNode FinalNode {
			get {
				var n = Node;
				while (n.Next != null)
					n = n.Next;
				return n;
			}
		}

		public bool IsPathBroken {
			get {
				var node = Node;
				while (node != null) {
					if (node.IsBroken)
						return true;
					node = node.Next;
				}
				return false;
			}
		}

		IPropertyPathNode Node {
			get; set;
		}

		public object Value {
			get; private set;
		}

		public PropertyPathWalker (string path)
		{
			// PropertyPath nodes are essentially a singly linked list
			// so the easiest way to construct the list is to process them
			// in reverse order.
			foreach (var node in path.Split ('.').Reverse ()) {
				var propertyName = node;

				int close = propertyName.LastIndexOf (']');
				if (close > -1) {
					int open = propertyName.LastIndexOf ('[');
					int index = int.Parse (propertyName.Substring (open + 1, close - open - 1));
					propertyName = propertyName.Substring (0, open);
					
					Node = new IndexedPropertyPathNode (index, Node);
				}
				Node = new StandardPropertyPathNode (propertyName, Node);
			}

			FinalNode.ValueChanged += delegate (object o, EventArgs e) {
				Value = ((PropertyPathNode) o).Value;
				var h = ValueChanged;
				if (h != null)
					h (this, EventArgs.Empty);
			};
		}

		public void Update (object source)
		{
			Node.SetSource (source);
		}
	}
}
