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
using System.Collections.Specialized;
using System.Windows.Controls.Primitives;

namespace System.Windows.Controls {

	public abstract class VirtualizingPanel : Panel {
		ItemContainerGenerator generator;
		
		public IItemContainerGenerator ItemContainerGenerator {
			get {
				if (generator == null) {
					ItemsControl owner = ItemsControl.GetItemsOwner (this);
					if (owner == null)
						throw new InvalidOperationException ("VirtualizingPanels must be in the Template of an ItemsControl in order to generate items");
					generator = owner.ItemContainerGenerator;
					generator.ItemsChanged += OnItemsChangedInternal;
				}
				return generator;
			}
		}
		
		protected VirtualizingPanel ()
		{
		}
		
		protected void AddInternalChild (UIElement child)
		{
			Children.Add (child);
		}
		
		protected void InsertInternalChild (int index, UIElement child)
		{
			Children.Insert (index, child);
		}
		
		protected void RemoveInternalChildRange (int index, int range)
		{
			for (int i = 0; i < range; i++)
				Children.RemoveAt (index);
		}
		
		protected virtual void BringIndexIntoView (int index)
		{
		}
		
		protected override Size ArrangeOverride (Size finalSize)
		{
			return base.ArrangeOverride (finalSize);
		}
		
		protected override Size MeasureOverride (Size availableSize)
		{
			return base.MeasureOverride (availableSize);
		}
		
		protected virtual void OnClearChildren ()
		{
		}
		
		void OnItemsChangedInternal (object sender, ItemsChangedEventArgs args)
 		{
			if (args.Action == NotifyCollectionChangedAction.Reset) {
				Children.Clear ();
				ItemContainerGenerator.RemoveAll ();
				OnClearChildren ();
			}

			OnItemsChanged (sender, args);
 		}

 		protected virtual void OnItemsChanged (object sender, ItemsChangedEventArgs args)
 		{
			
 		}
	}
}
