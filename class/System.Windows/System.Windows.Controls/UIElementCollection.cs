//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2008 Novell, Inc.
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

using Mono;
using System;
using System.Collections.Specialized;

namespace System.Windows.Controls
{
	public sealed partial class UIElementCollection : PresentationFrameworkCollection <UIElement>
	{
		internal event EventHandler Clearing;

		internal event NotifyCollectionChangedEventHandler ItemsChanged;

		internal override void InternalCollectionChanged (InternalCollectionChangedEventArgs args)
		{
			base.InternalCollectionChanged (args);

			switch (args.ChangedAction) {
			case CollectionChangedAction.Add:
				ItemsChanged.Raise (this, NotifyCollectionChangedAction.Add, args.GetNewItem (typeof (UIElement)), args.Index);
				break;
			case CollectionChangedAction.Remove:
				ItemsChanged.Raise (this, NotifyCollectionChangedAction.Remove, args.GetOldItem (typeof (UIElement)), args.Index);
				break;
			case CollectionChangedAction.Replace:
				ItemsChanged.Raise (this, NotifyCollectionChangedAction.Replace, args.GetNewItem (typeof (UIElement)), args.GetOldItem (typeof (UIElement)), args.Index);
				break;
			case CollectionChangedAction.Clearing:
				var h = Clearing;
				if (h != null)
					h (this, EventArgs.Empty);
				break;
			case CollectionChangedAction.Cleared:
				ItemsChanged.Raise (this, NotifyCollectionChangedAction.Reset);
				break;
			}
		}

	}
}
