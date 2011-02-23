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
			// note: SL4 does a NRE so 'formattingProperty.Native' without a null check is "ok"
			IntPtr val = NativeMethods.text_selection_get_property_value (native, formattingProperty.Native);
			if (val == IntPtr.Zero)
				return DependencyProperty.UnsetValue;
			return Value.ToObject (null, val);
		}

		public void ApplyPropertyValue (DependencyProperty formattingProperty, object value)
		{
			if (value == null)
				throw new ArgumentException ("value");

			using (var val = Value.FromObject (value)) {
				var v = val;
				// note: SL4 does a NRE so 'formattingProperty.Native' without a null check is "ok"
				NativeMethods.text_selection_apply_property_value (native, formattingProperty.Native, ref v);
			}
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
			set {
				if (value == null)
					throw new ArgumentNullException ("Text");
				NativeMethods.text_selection_set_text (native, value);
			}
		}

		public string Xaml {
			get { return NativeMethods.text_selection_get_xaml (native); }
			set {
				if (value == null)
					throw new ArgumentNullException ("Xaml");
				// FIXME: xaml validation -> ArgumentException
				NativeMethods.text_selection_set_xaml (native, value);
			}
		}

		TextPointer end;
		public TextPointer End {
			get {
				if (end == null) {
					IntPtr tp = NativeMethods.text_selection_get_end (native);
					if (tp != IntPtr.Zero)
						end = new TextPointer (tp);
				}
				return end;
			}
		}

		TextPointer start;
		public TextPointer Start {
			get {
				if (start == null) {
					IntPtr tp = NativeMethods.text_selection_get_start (native);
					if (tp != IntPtr.Zero)
						start = new TextPointer (tp);
				}
				return start;
			}
		}
	}
}
