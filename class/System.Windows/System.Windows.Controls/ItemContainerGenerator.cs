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

#pragma warning disable 67 // "The event 'E' is never used" shown for ItemsChanged

using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Linq;
using System.Windows;
using System.Windows.Controls.Primitives;

using Hyena.Collections;
using Mono;

namespace System.Windows.Controls {

	public sealed class ItemContainerGenerator : IRecyclingItemContainerGenerator {

		public event ItemsChangedEventHandler ItemsChanged;

		DoubleKeyedDictionary <DependencyObject, int> ContainerIndexMap {
			get; set;
		}

		Dictionary <DependencyObject, object> ContainerItemMap {
			get; set;
		}

		Queue <DependencyObject> Cache {
			get; set;
		}

		internal GenerationState GenerationState {
			get; set;
		}

		ItemsControl Owner {
			get; set;
		}

		Panel Panel {
			get { return Owner.Panel; }
		}

		RangeCollection RealizedElements {
			get; set;
		}

		internal ItemContainerGenerator (ItemsControl owner)
		{
			Cache = new Queue <DependencyObject> ();
			ContainerIndexMap = new DoubleKeyedDictionary <DependencyObject, int> ();
			ContainerItemMap  = new Dictionary <DependencyObject, object> ();
			Owner = owner;
			RealizedElements = new RangeCollection ();
		}

		public DependencyObject ContainerFromIndex (int index)
		{
			DependencyObject container;
			ContainerIndexMap.TryMap (index, out container);
			return container;
		}

		public DependencyObject ContainerFromItem (object item)
		{
			if (item == null)
				return null;

			foreach (var v in ContainerItemMap)
				if (Helper.AreEqual (v.Value, item))
					return v.Key;
			return null;
		}

		void CheckOffsetAndRealized (GeneratorPosition position, int count)
		{
			if (position.Offset != 0)
				throw new ArgumentException ("position.Offset must be zero as the position must refer to a realized element");
			
			int index = IndexFromGeneratorPosition (position);
			int rangeIndex = RealizedElements.FindRangeIndexForValue (index);
			RangeCollection.Range range = RealizedElements.Ranges[rangeIndex];
			if (index < range.Start || (index + count) > range.Start + range.Count)
				throw new InvalidOperationException ("Only items which have been Realized can be removed");
		}

		internal DependencyObject GenerateNext (out bool isNewlyRealized)
		{
			if (GenerationState == null)
				throw new InvalidOperationException ("Cannot call GenerateNext before calling StartAt");

			int index;
			// This is relative to the realised elements.
			int startAt = GenerationState.Position.Index;
			if (startAt == -1) {
				if (GenerationState.Position.Offset < 0)
					index = Owner.Items.Count + GenerationState.Position.Offset;
				else if (GenerationState.Position.Offset == 0)
					index = 0;
				else
					index = GenerationState.Position.Offset - 1;
			} else if (startAt >= 0 && startAt < RealizedElements.Count) {
				// We're starting relative to an already realised element
				index = RealizedElements [startAt] + GenerationState.Position.Offset;
			} else {
				index = -1;
			}

			bool alreadyRealized = RealizedElements.Contains (index);
			if (!GenerationState.AllowStartAtRealizedItem && alreadyRealized && GenerationState.Position.Offset == 0) {
				index += GenerationState.Step;
				alreadyRealized = RealizedElements.Contains (index);
			}
			
			if (index < 0 || index >= Owner.Items.Count) {
				isNewlyRealized = false;
				return null;
			}
			
			if (alreadyRealized) {
				GenerationState.Position = new GeneratorPosition (RealizedElements.IndexOf (index), GenerationState.Step);
				isNewlyRealized = false;
				
				return ContainerIndexMap [index];
			}
			
			DependencyObject container;
			var item = Owner.Items [index];
			if (Owner.IsItemItsOwnContainer (item)) {
				container = (DependencyObject) item;
				isNewlyRealized = true;
			} else {
				if (Cache.Count == 0) {
					container = Owner.GetContainerForItem ();
					isNewlyRealized = true;
				} else {
					container = Cache.Dequeue ();
					isNewlyRealized = false;
				}
				
				ContentControl c = container as ContentControl;
				if (c != null)
					c.ContentSetsParent = false;
			}

			FrameworkElement f = container as FrameworkElement;
			if (f != null && !(item is UIElement))
				f.DataContext = item;

			RealizedElements.Add (index);
			ContainerIndexMap.Add (container, index);
			ContainerItemMap.Add (container, item);
			
			GenerationState.Position = new GeneratorPosition (RealizedElements.IndexOf (index), GenerationState.Step);
			return container;
		}

		public GeneratorPosition GeneratorPositionFromIndex (int itemIndex)
		{
			if (itemIndex < 0) {
				return new GeneratorPosition (-1, 0);
			}
			else if (RealizedElements.Contains (itemIndex)) {
				return new GeneratorPosition (RealizedElements.IndexOf (itemIndex), 0);
			}
			else if (itemIndex > Owner.Items.Count) {
				return new GeneratorPosition (-1, 0);
			}

			if (RealizedElements.Count == 0)
				return new GeneratorPosition (-1, itemIndex + 1);

			int index = -1;
			for (int i = 0; i < RealizedElements.Count; i ++) {
				if (RealizedElements [i] > itemIndex)
					break;
				index = i;
			}
			if (index == -1) {
				return new GeneratorPosition (index, itemIndex + 1);
			} else {
				return new GeneratorPosition (index, itemIndex - RealizedElements [index]);
			}
		}

		internal ItemContainerGenerator GetItemContainerGeneratorForPanel (Panel panel)
		{
			// FIXME: Double check this, but i think it's right
			return panel == Panel ? this : null;
		}

		public int IndexFromContainer (DependencyObject container)
		{
			int index;
			if (!ContainerIndexMap.TryMap (container, out index))
				index = -1;
			return index;
		}

		public int IndexFromGeneratorPosition (GeneratorPosition position)
		{
			// We either have everything realised or nothing realised, so we can
			// simply just add Index and Offset together to get the right index (i think)
			if (position.Index == -1) {
				if (position.Offset < 0)
					return Owner.Items.Count + position.Offset;
				//else if (position.Offset == 0)
				//	return 0;
				else
					return position.Offset - 1;
			} else {
				if (position.Index > Owner.Items.Count)
					return -1;
				if (position.Index >= 0 && position.Index < RealizedElements.Count)
					return RealizedElements [position.Index] + position.Offset;
				return position.Index + position.Offset;
			}
		}

		public object ItemFromContainer (DependencyObject container)
		{
			object item;
			ContainerItemMap.TryGetValue (container, out item);
			return item ?? DependencyProperty.UnsetValue;
		}

		internal void OnOwnerItemsItemsChanged (object sender, NotifyCollectionChangedEventArgs e)
		{
			int itemCount;
			int itemUICount;
			GeneratorPosition oldPosition = new GeneratorPosition(-1, 0);
			GeneratorPosition position;
			
			switch (e.Action) {
			case NotifyCollectionChangedAction.Add:
				if ((e.NewStartingIndex + 1) != Owner.Items.Count)
					MoveExistingItems (e.NewStartingIndex, 1);
				itemCount = 1;
				itemUICount = 0;
				position = GeneratorPositionFromIndex (e.NewStartingIndex);
				// Yes, this looks like a Silverlight Bug.
				position.Offset = 1;
				break;
			case NotifyCollectionChangedAction.Remove:
				itemCount = 1;
				itemUICount = RealizedElements.Contains (e.OldStartingIndex) ? 1 : 0;
				position = GeneratorPositionFromIndex (e.OldStartingIndex);
				if (itemUICount == 1)
					Remove (position, 1);
				MoveExistingItems (e.OldStartingIndex, -1);
				break;
			case NotifyCollectionChangedAction.Replace:
				if (!RealizedElements.Contains (e.NewStartingIndex))
					return;

				itemCount = 1;
				itemUICount = 1;
				position = GeneratorPositionFromIndex (e.NewStartingIndex);
				Remove (position, 1);

				bool fresh;
				var newPos = GeneratorPositionFromIndex (e.NewStartingIndex);
				using (StartAt (newPos, GeneratorDirection.Forward, true))
					PrepareItemContainer (GenerateNext (out fresh));
				break;
			case NotifyCollectionChangedAction.Reset:
				itemCount = e.OldItems == null ? 0 : e.OldItems.Count;
				itemUICount = RealizedElements.Count;
				position = new GeneratorPosition (-1, 0);
				RemoveAll ();
				break;
			default:
				Console.WriteLine ("*** Critical error in ItemContainerGenerator.OnOwnerItemsItemsChanged. NotifyCollectionChangedAction.{0} is not supported", e.Action);
				return;
			}
			
			ItemsChangedEventArgs args = new ItemsChangedEventArgs {
				Action = e.Action,
				ItemCount = itemCount,
				ItemUICount = itemUICount,
				OldPosition = oldPosition,
				Position = position
			};
			var h = ItemsChanged;
			if (h != null)
				h (this, args);
		}

		
		internal void PrepareItemContainer (DependencyObject container)
		{
			var index = ContainerIndexMap [container];
			var item = Owner.Items [index];

			Owner.PrepareContainerForItem (container, item);
		}

		internal void Remove (GeneratorPosition position, int count)
		{
			CheckOffsetAndRealized (position, count);

			int index = IndexFromGeneratorPosition (position);
			for (int i = 0; i < count; i++) {
				var container = ContainerIndexMap [index + i];
				var item = ContainerItemMap [container];
				ContainerIndexMap.Remove (container, index + i);
				ContainerItemMap.Remove (container);
				RealizedElements.Remove (index + i);
				Owner.ClearContainerForItem (container, item);
			}
		}

		void MoveExistingItems (int index, int offset)
		{
			// This is a little horrible. I should really collapse the existing
			// RangeCollection so that every > the current index is decremented by 1.
			// This is easier for now though. I may think of a better way later on.
			RangeCollection newRanges = new RangeCollection ();
			List <int> list = new List<int> ();
			for (int i = 0; i < RealizedElements.Count; i++)
				list.Add (RealizedElements [i]);
			
			if (offset > 0)
				list.Reverse ();

			foreach (int i in list) {
				int oldIndex = i;
				if (oldIndex < index) {
					newRanges.Add (oldIndex);
				} else {
					newRanges.Add (oldIndex + offset);
					var container = ContainerIndexMap [oldIndex];
					ContainerIndexMap.Remove (container, oldIndex);
					ContainerIndexMap.Add (container, oldIndex + offset);
				}
			}

			RealizedElements = newRanges;
		}

		internal void RemoveAll ()
		{
			foreach (var pair in ContainerItemMap)
				Owner.ClearContainerForItem (pair.Key, pair.Value);
			RealizedElements.Clear ();
			ContainerIndexMap.Clear ();
			ContainerItemMap.Clear ();
		}

		internal IDisposable StartAt (GeneratorPosition position,
							     GeneratorDirection direction,
							     bool allowStartAtRealizedItem)
		{
			if (GenerationState != null)
				throw new InvalidOperationException ("Cannot call StartAt while a generation operation is in progress");

			GenerationState = new GenerationState {
				AllowStartAtRealizedItem = allowStartAtRealizedItem,
				Direction = direction,
				Position = position,
				Generator = this
			};
			return GenerationState;
		}

		internal void Recycle (GeneratorPosition position, int count)
		{
			CheckOffsetAndRealized (position, count);

			int index = IndexFromGeneratorPosition (position);
			for (int i = 0; i < count; i++)
				Cache.Enqueue (ContainerIndexMap[index + i]);
			Remove (position, count);
		}

		DependencyObject IItemContainerGenerator.GenerateNext (out bool isNewlyRealized)
		{
			return GenerateNext (out isNewlyRealized);
		}

		ItemContainerGenerator IItemContainerGenerator.GetItemContainerGeneratorForPanel (Panel panel)
		{
			return GetItemContainerGeneratorForPanel (panel);
		}

		void IItemContainerGenerator.PrepareItemContainer (DependencyObject container)
		{
			PrepareItemContainer (container);
		}

		void IItemContainerGenerator.Remove (GeneratorPosition position, int count)
		{
			Remove (position, count);
		}

		void IItemContainerGenerator.RemoveAll ()
		{
			RemoveAll ();
		}

		IDisposable IItemContainerGenerator.StartAt (GeneratorPosition position,
							     GeneratorDirection direction,
							     bool allowStartAtRealizedItem)
		{
			return StartAt (position, direction, allowStartAtRealizedItem);
		}

		void IRecyclingItemContainerGenerator.Recycle (GeneratorPosition position, int count)
		{
			Recycle (position, count);
		}
	}
}

