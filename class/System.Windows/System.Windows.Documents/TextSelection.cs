// 
// TextSelection.cs
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
using System.Threading;
using Mono;
using System.Collections;
using System.Collections.Generic;

namespace System.Windows.Documents {
	public sealed class TextSelection {
		IntPtr native;
		
		internal TextSelection (IntPtr raw)
		{
			native = raw;
		}
		
		public object GetPropertyValue (DependencyProperty formattingProperty)
		{
			Console.WriteLine ("System.Windows.Documents.TextSelection.GetPropertyValue () NIEX");
			throw new NotImplementedException ();
		}

		public void ApplyPropertyValue (DependencyProperty formattingProperty, object value)
		{
			Console.WriteLine ("System.Windows.Documents.TextSelection.ApplyPropertyValue: () NIEX");
			throw new NotImplementedException ();
		}

		public void Insert (TextElement element)
		{
			if (element == null)
				throw new ArgumentNullException ("element");

			NativeMethods.text_selection_insert (native, element.native);
		}

		public void Select (TextPointer anchorPosition, TextPointer movingPosition)
		{
			if (anchorPosition == null)
				throw new ArgumentNullException ("anchorPosition");
			if (movingPosition == null)
				throw new ArgumentNullException ("movingPosition");

			NativeMethods.text_selection_select (native, anchorPosition.native, movingPosition.native);
		}

		public string Text {
			get { return NativeMethods.text_selection_get_text (native); }
			set { NativeMethods.text_selection_set_text (native, value); }
		}

		public string Xaml {
			get { return NativeMethods.text_selection_get_xaml (native); }
			set { NativeMethods.text_selection_set_xaml (native, value); }
		}

		public TextPointer End {
			get {
				IntPtr tp = NativeMethods.text_selection_get_end (native);
				return tp == IntPtr.Zero ? null : new TextPointer (tp);
			}
		}

		public TextPointer Start {
			get {
				IntPtr tp = NativeMethods.text_selection_get_start (native);
				return tp == IntPtr.Zero ? null : new TextPointer (tp);
			}
		}
	}
}
