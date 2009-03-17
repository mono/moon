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
using System.Windows;
using Mono;

namespace System.Windows.Controls {

	public partial class Grid : Panel {

		public static int GetColumn (FrameworkElement element)
		{
			return (int) element.GetValue (ColumnProperty);
		}

		public static int GetColumnSpan (FrameworkElement element)
		{
			return (int) element.GetValue (ColumnSpanProperty);
		}

		public static int GetRow (FrameworkElement element)
		{
			return (int) element.GetValue (RowProperty);
		}

		public static int GetRowSpan (FrameworkElement element)
		{
			return (int) element.GetValue (RowSpanProperty);
		}

		public static void SetColumn (FrameworkElement element, int value)
		{
			element.SetValue (ColumnProperty, value);
		}

		public static void SetColumnSpan (FrameworkElement element, int value)
		{
			element.SetValue (ColumnSpanProperty, value);
		}

		public static void SetRow (FrameworkElement element, int value)
		{
			element.SetValue (RowProperty, value);
		}

		public static void SetRowSpan (FrameworkElement element, int value)
		{
			element.SetValue (RowSpanProperty, value);
		}

		protected sealed override Size ArrangeOverride (Size arrangeSize)
		{
			return base.ArrangeOverride (arrangeSize);
		}

		protected sealed override Size MeasureOverride (Size constraint)
		{
			return base.MeasureOverride (constraint);
		}
	}
}
