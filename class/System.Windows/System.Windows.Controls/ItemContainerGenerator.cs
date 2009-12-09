//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2009 Novell, Inc.
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

using System.Windows.Controls.Primitives;

namespace System.Windows.Controls {

	public sealed class ItemContainerGenerator : IRecyclingItemContainerGenerator, IItemContainerGenerator {

		public event ItemsChangedEventHandler ItemsChanged;
		
		ItemsControl Owner {
			get; set;
		}

		internal ItemContainerGenerator (ItemsControl owner)
		{
			Owner = owner;
		}

		public DependencyObject ContainerFromIndex (int index)
		{
			return Owner.GetContainerItem (index);
		}

		public DependencyObject ContainerFromItem (object item)
		{
			return Owner.GetContainerItem (Owner.Items.IndexOf (item));
		}

		public GeneratorPosition GeneratorPositionFromIndex (int itemIndex)
		{
			// FIXME: No idea if this is actually right.
			object container = Owner.GetContainerItem (itemIndex);
			if (container == null) {
				// None realised
				return new GeneratorPosition (-1, itemIndex);
			} else {
				// All realised
				return new GeneratorPosition (itemIndex, 0);
			}
		}

		public int IndexFromContainer (DependencyObject container)
		{
			int count = Owner.Items.Count;
			for (int i = 0; i < count; i++)
				if (Owner.GetContainerItem (i) == container)
					return i;
			return -1;
		}

		public int IndexFromGeneratorPosition (GeneratorPosition position)
		{
			// We either have everything realised or nothing realised, so we can
			// simply just add Index and Offset together to get the right index (i think)
			if (position.Index == -1)
				position.Index ++;
			if (position.Offset == -1)
				position.Offset ++;
			if (position.Index + position.Offset > Owner.Items.Count)
				return -1;
			
			return position.Index + position.Offset;
		}

		public object ItemFromContainer (DependencyObject container)
		{
			int count = Owner.Items.Count;
			for (int i = 0; i < count; i ++)
				if (Owner.GetContainerItem (i) == container)
					return Owner.Items [i];
			return null;
		}

		DependencyObject IItemContainerGenerator.GenerateNext (out bool isNewlyRealized)
		{
			throw new NotImplementedException ();
		}

		ItemContainerGenerator IItemContainerGenerator.GetItemContainerGeneratorForPanel (Panel panel)
		{
			throw new NotImplementedException ();
		}

		void IItemContainerGenerator.PrepareItemContainer (DependencyObject container)
		{
			throw new NotImplementedException ();
		}

		void IItemContainerGenerator.Remove (GeneratorPosition position, int count)
		{
			throw new NotImplementedException ();
		}

		void IItemContainerGenerator.RemoveAll ()
		{
			throw new NotImplementedException ();
		}

		IDisposable IItemContainerGenerator.StartAt (GeneratorPosition position,
							     GeneratorDirection direction,
							     bool allowStartAtRealizedItem)
		{
			throw new NotImplementedException ();
		}

		void IRecyclingItemContainerGenerator.Recycle (GeneratorPosition position,
							       int count)
		{
			throw new NotImplementedException ();
		}
	}
}

