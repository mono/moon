//
// TextPointer.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2010 Novell, Inc (http://www.novell.com)
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
using Mono;
using System.Collections;
using System.Collections.Generic;

namespace System.Windows.Documents {
	public class TextPointer {
		internal IntPtr native;

		internal TextPointer (IntPtr raw)
		{
			IsAtInsertionPosition = NativeMethods.text_pointer_get_is_at_insertion_position (raw);
			LogicalDirection = NativeMethods.text_pointer_get_logical_direction (raw);
			Parent = NativeDependencyObjectHelper.FromIntPtr (NativeMethods.text_pointer_get_parent (raw)) as DependencyObject;

			native = raw;
		}

		public bool IsAtInsertionPosition {
			get; private set;
		}

		public LogicalDirection LogicalDirection {
			get; private set;
		}

		public DependencyObject Parent {
			get; private set;
		}

		public int CompareTo (TextPointer position)
		{
			if (position == null)
				throw new ArgumentNullException ("position");

			return NativeMethods.text_pointer_compare_to (native, position.native);
		}

		public Rect GetCharacterRect (LogicalDirection direction)
		{
			return NativeMethods.text_pointer_get_character_rect (native, direction);
		}

		public TextPointer GetNextInsertionPosition (LogicalDirection direction)
		{
			IntPtr tp = NativeMethods.text_pointer_get_next_insertion_position (native, direction);
			return tp == IntPtr.Zero ? null : new TextPointer (tp);
		}

		public TextPointer GetPositionAtOffset (int offset, LogicalDirection direction)
		{
			IntPtr tp = NativeMethods.text_pointer_get_position_at_offset (native, offset, direction);
			return tp == IntPtr.Zero ? null : new TextPointer (tp);
		}
	}
}
