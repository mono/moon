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
using System.Reflection;
using System.ComponentModel;

namespace System.Windows.Data
{
	class PropertyPathWalker {
		static readonly PropertyInfo[] CollectionViewProperties = typeof (ICollectionView).GetProperties ();

		public event EventHandler ValueChanged;

		public IPropertyPathNode FinalNode {
			get {
				var n = Node;
				while (n != null && n.Next != null)
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
			: this (path, true, false)
		{

		}

		public PropertyPathWalker (string path, bool bindDirectlyToSource, bool bindsToView)
		{
			string index;
			string propertyName;
			string typeName;
			PropertyNodeType type;

			if (string.IsNullOrEmpty (path)) {
				// If the property path is null or empty, we still need to add a CollectionViewNode
				// to handle the case where we bind diretly to a CollectionViewSource. i.e. new Binding () { Source = cvs }
				Node = new CollectionViewNode (bindDirectlyToSource, bindsToView);
			} else {
				var parser = new PropertyPathParser (path);
				while ((type = parser.Step (out typeName, out propertyName, out index)) != PropertyNodeType.None) {
					bool isViewProperty = CollectionViewProperties.Any (prop => prop.Name == propertyName);
					IPropertyPathNode node = new CollectionViewNode (bindDirectlyToSource, isViewProperty || bindsToView);
					switch (type) {
					case PropertyNodeType.AttachedProperty:
					case PropertyNodeType.Property:
						node.Next = new StandardPropertyPathNode (typeName, propertyName);
						break;
					case PropertyNodeType.Indexed:
						node.Next = new IndexedPropertyPathNode (index);
						break;
					default:
						throw new Exception ("Unsupported node type");
					}
					if (Node == null)
						Node = node;
					else
						FinalNode.Next = node;
				}
			}

			FinalNode.ValueChanged += delegate (object o, EventArgs e) {
				Value = ((PropertyPathNode) o).Value;
				var h = ValueChanged;
				if (h != null)
					h (this, EventArgs.Empty);
			};
		}

		public object GetValue (object item)
		{
			Update (item);
			object o = FinalNode.Value;
			Update (null);
			return o;
		}

		public void Update (object source)
		{
			Node.SetSource (source);
		}
	}
}
