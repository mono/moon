//
// System.Windows.Controls.TextBox.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2007 Novell, Inc (http://www.novell.com)
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
using System.Windows;
using System.Windows.Media;
using System.Windows.Documents;
using System.Collections.Generic;

namespace System.Windows.Controls {
	public partial class TextBox : Control {
		protected override Size ArrangeOverride (Size size)
		{
			// FIXME: implement me
			return base.ArrangeOverride (size);
		}
		
		public IEnumerable<UIElement> HitTest (Point point)
		{
			return null;
		}
		
		public IEnumerable<UIElement> HitTest (Rect rect)
		{
			return null;
		}
		
		public void Select (int start, int length)
		{
			if (start < 0)
				throw new ArgumentOutOfRangeException ("start");
			
			if (length < 0)
				throw new ArgumentOutOfRangeException ("length");
			
			// FIXME: do exceptions get thrown if the
			// start/length values are out of range? If
			// not, what is supposed to happen?
			
			NativeMethods.text_box_select (this.native, start, length);
		}
	}
}
