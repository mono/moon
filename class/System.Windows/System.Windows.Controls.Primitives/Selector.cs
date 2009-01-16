//
// Selector.cs
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


namespace System.Windows.Controls.Primitives {
	public abstract class Selector : ItemsControl {
		
		public static readonly DependencyProperty SelectedIndexProperty;
		public static readonly DependencyProperty SelectedItemProperty;
		static int DefaultSelectedIndex = -1;
		
		static Selector ()
		{
			PropertyMetadata metadata = new PropertyMetadata (DefaultSelectedIndex, delegate (DependencyObject o, DependencyPropertyChangedEventArgs e) {
				((Selector) o).SelectedIndexChanged (o, e);
			});
			SelectedIndexProperty = DependencyProperty.Register ("SelectedIndex", typeof(int), typeof(Selector), metadata);

			metadata = new PropertyMetadata (null, delegate (DependencyObject o, DependencyPropertyChangedEventArgs e) {
				((Selector) o).SelectedItemChanged (o, e);
			});
			SelectedItemProperty = DependencyProperty.Register ("SelectedItem", typeof(object), typeof(Selector), metadata);
		}

		// looks like a bad idea (if only to test it) but SL2 does not expose it
		internal Selector ()
		{
		}

		int selectedIndex = DefaultSelectedIndex;
		object selectedItem;
		
		public int SelectedIndex {
			get { return selectedIndex; }
			set { SetValue (SelectedIndexProperty, value); }
		}

		
		public object SelectedItem {
			get { return selectedItem; }
			set { SetValue (SelectedItemProperty, value); }
		}

		bool changing;
		public event SelectionChangedEventHandler SelectionChanged;

		void SelectedIndexChanged (DependencyObject o, DependencyPropertyChangedEventArgs e)
		{
			int newVal = (int) e.NewValue;
			if (newVal == (int) e.OldValue || changing) {
				selectedIndex = newVal;
				return;
			}

			selectedIndex = newVal;
			changing = true;
			try {
				if (newVal < 0)
					ClearValue (SelectedItemProperty);
				else if (newVal < Items.Count)
					SelectedItem = Items [newVal];

			} finally {
				changing = false;
			}
			RaiseSelectionChanged (o, new SelectionChangedEventArgs (new object[] { e.OldValue }, new object [] { e.NewValue }));
		}
		
		void SelectedItemChanged (DependencyObject o, DependencyPropertyChangedEventArgs e)
		{
			if (e.NewValue == e.OldValue || changing) {
				selectedItem = e.NewValue;
				return;
			}
			
			changing = true;
			try {
				int index = e.NewValue == null ? -1 : Items.IndexOf (e.NewValue);
				if (index == -1) {
					SelectedIndex = e.OldValue == null ? -1 : Items.IndexOf (e.OldValue);
					if (e.OldValue == null)
						ClearValue (SelectedItemProperty);
					else
						SelectedItem = e.OldValue;
				}
				else {
					selectedItem = e.NewValue;
					SelectedIndex = index;
					RaiseSelectionChanged (o, new SelectionChangedEventArgs (new object[] { e.OldValue }, new object [] { e.NewValue }));
				}
			} finally {
				changing = false;
			}
		}

		void RaiseSelectionChanged (object o, SelectionChangedEventArgs e)
		{
			SelectionChangedEventHandler h = SelectionChanged;
			if (h != null)
				h (o, e);
		}
		

		[MonoTODO]
		public static bool GetIsSelectionActive (DependencyObject element)
		{
			if (element == null)
				throw new ArgumentNullException ("element");

			Selector s = (element as Selector);
			if (s == null)
				return false;

			// FIXME: return true if focused (but there's no public IsFocused available)
			return false;
		}
	}
}
