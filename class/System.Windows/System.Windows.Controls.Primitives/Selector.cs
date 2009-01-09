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
	        public static readonly DependencyProperty SelectedIndexProperty = 
			DependencyProperty.Register ("SelectedIndex", typeof(int), typeof(Selector), null);
	        public static readonly DependencyProperty SelectedItemProperty = 
			DependencyProperty.Register ("SelectedItem", typeof(object), typeof(Selector), null);

		// looks like a bad idea (if only to test it) but SL2 does not expose it
		internal Selector ()
		{
		}

		public int SelectedIndex {
			get { return (int)GetValue (SelectedIndexProperty); }
			set { SetValue (SelectedIndexProperty, value); }
		}

		public object SelectedItem {
			get { return GetValue (SelectedItemProperty); }
			set { SetValue (SelectedItemProperty, value); }
		}

		public event SelectionChangedEventHandler SelectionChanged;

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
