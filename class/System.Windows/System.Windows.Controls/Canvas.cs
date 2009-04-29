//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2007 Novell, Inc.
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
	public partial class Canvas : Panel {
		public static double GetLeft (UIElement element)
		{
			return (double) element.GetValue (Canvas.LeftProperty);
		}
		
		public static void SetLeft (UIElement element, double length)
		{
			element.SetValue (Canvas.LeftProperty, length);
		}
		
		public static double GetTop (UIElement element)
		{
			return (double) element.GetValue (Canvas.TopProperty);
		}
		
		public static void SetTop (UIElement element, double length)
		{
			element.SetValue (Canvas.TopProperty, length);
		}
		
		public static int GetZIndex (UIElement element)
		{
			return (int) element.GetValue (Canvas.ZIndexProperty);
		}
		
		public static void SetZIndex (UIElement element, int zindex)
		{
			element.SetValue (Canvas.ZIndexProperty, zindex);
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
