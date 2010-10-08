//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2008-2009 Novell, Inc.
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

using System.Collections.Specialized;

using Mono;

namespace System.Windows.Controls {

	public sealed partial class ItemCollection : PresentationFrameworkCollection<object>, INotifyCollectionChanged {

		bool readOnly;

		internal void SetIsReadOnly (bool readOnly)
		{
			this.readOnly = readOnly;
		}

		// Note: Parameter handling is different from other PresentationFrameworkCollection<T> types
		// but it may not be limited to this
		internal override bool NullCheck (NotifyCollectionChangedAction action, object value)
		{
			if (action == NotifyCollectionChangedAction.Remove && value == null)
				return true;
			if (value == null)
				throw new ArgumentException ();
			return false;
		}
		
		internal override void AddImpl (object value)
		{
			AddImpl (value, false);
		}

		internal override int IndexOfImpl (object value)
		{
			if (value == null)
				return -1;

			if (value.GetType ().IsValueType) {
				int count = Count;
				for (int i = 0; i < count; i++)
					if (Helper.Equals (this [i], value))
						return i;
				return -1;
			} else {
				return base.IndexOfImpl (value, false);
			}
		}

		internal override bool IsReadOnlyImpl ()
		{
			return readOnly;
		}
		
		internal event EventHandler Clearing;

		internal event NotifyCollectionChangedEventHandler ItemsChanged;

		event NotifyCollectionChangedEventHandler INotifyCollectionChanged.CollectionChanged {
			add { ItemsChanged += value; }
			remove { ItemsChanged -= value; }
		}

		internal override void InternalCollectionChanged (InternalCollectionChangedEventArgs args)
		{
			base.InternalCollectionChanged (args);

			switch (args.ChangedAction) {
			case CollectionChangedAction.Add:
				ItemsChanged.Raise (this, NotifyCollectionChangedAction.Add, args.GetNewItem (), args.Index);
				break;
			case CollectionChangedAction.Remove:
				ItemsChanged.Raise (this, NotifyCollectionChangedAction.Remove, args.GetOldItem (), args.Index);
				break;
			case CollectionChangedAction.Replace:
				ItemsChanged.Raise (this, NotifyCollectionChangedAction.Replace, args.GetNewItem (), args.GetOldItem (), args.Index);
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
