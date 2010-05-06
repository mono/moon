//
// CollectionViewNode.cs
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
using System.ComponentModel;

namespace System.Windows.Data {

	class CollectionViewNode : PropertyPathNode {

		public override bool IsBroken {
			get { return Source == null; }
		}

		bool BindsDirectlyToSource {
			get; set;
		}

		public CollectionViewNode (bool bindsDirectlyToSource)
		{
			BindsDirectlyToSource = bindsDirectlyToSource;
		}

		protected override void OnSourceChanged (object oldSource, object newSource)
		{
			CollectionViewSource source;
			base.OnSourceChanged (oldSource, newSource);

			source = oldSource as CollectionViewSource;
			if (source != null)
				source.View.CurrentChanged -= HandleSourceViewCurrentChanged;

			source = newSource as CollectionViewSource;
			if (source != null)
				source.View.CurrentChanged += HandleSourceViewCurrentChanged;
		}

		void HandleSourceViewCurrentChanged (object sender, EventArgs e)
		{
			UpdateValue ();
			if (Next != null)
				Next.SetSource (Value);
		}

		public override void SetValue (object value)
		{
			throw new System.NotImplementedException ();
		}

		public override void UpdateValue ()
		{
			// If the source is a CollectionViewSource we use its View property,
			// if it is a CollectionView we use that directly. If we are binding
			// directly to the source, this means we need to bind to the CollectionView
			// object directly if it exists as opposed to CollectionView.CurrentItem.

			ICollectionView view = null;
			if (Source is CollectionViewSource)
				view = ((CollectionViewSource) Source).View;
			else if (Source is ICollectionView)
				view = (ICollectionView) Source;

			if (view == null || BindsDirectlyToSource) {
				// If our source object is not (or does not have) an ICollectionView, then we
				// fall back to using the Source object directly.
				object o = view ?? Source;
				ValueType = o == null ? null : o.GetType ();
				Value = o;
			} else {
				ValueType = view.CurrentItem == null ? null : view.CurrentItem.GetType ();
				Value = view.CurrentItem;
			}
		}
	}
}

