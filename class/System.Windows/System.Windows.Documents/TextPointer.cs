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
	public class TextPointer : INativeDependencyObjectWrapper {
		EventObjectSafeHandle safeHandle;

		IntPtr NativeHandle {
			get { return safeHandle.DangerousGetHandle (); }
		}

		EventObjectSafeHandle INativeEventObjectWrapper.SafeHandle {
			get { return safeHandle; }
		}

		private static readonly DependencyProperty IsAtInsertionPositionProperty =
			DependencyProperty.Lookup (Kind.TEXTPOINTER, "IsAtInsertionPosition", typeof (bool));

		private static readonly DependencyProperty LogicalDirectionProperty =
			DependencyProperty.Lookup (Kind.TEXTPOINTER, "LogicalDirection", typeof (LogicalDirection));

		internal TextPointer (IntPtr raw, bool dropref)
		{
			safeHandle = NativeDependencyObjectHelper.AddNativeMapping (raw, this);
			if (dropref)
				NativeMethods.event_object_unref (raw);
		}
		
		Kind INativeEventObjectWrapper.GetKind ()
		{
			return Deployment.Current.Types.TypeToKind (GetType ());
		}

		void INativeEventObjectWrapper.MentorChanged (IntPtr mentor_ptr)
		{
		}

		void INativeEventObjectWrapper.OnAttached ()
		{
		}

		void INativeEventObjectWrapper.OnDetached ()
		{
		}
		
		public bool IsAtInsertionPosition {
			get { return (bool)((INativeDependencyObjectWrapper)this).GetValue (IsAtInsertionPositionProperty); }
		}

		public LogicalDirection LogicalDirection {
			get { return (LogicalDirection)((INativeDependencyObjectWrapper)this).GetValue (LogicalDirectionProperty); }
		}

		public DependencyObject Parent {
			get { return NativeDependencyObjectHelper.FromIntPtr (NativeMethods.text_pointer_get_parent (NativeHandle)) as DependencyObject; }
		}

		public int CompareTo (TextPointer position)
		{
			if (position == null)
				throw new ArgumentNullException ("position");

			var positionHandle = ((INativeEventObjectWrapper) position).SafeHandle;
			return NativeMethods.text_pointer_compare_to (NativeHandle, positionHandle.DangerousGetHandle ());
		}

		public Rect GetCharacterRect (LogicalDirection direction)
		{
			return NativeMethods.text_pointer_get_character_rect (NativeHandle, direction);
		}

		public TextPointer GetNextInsertionPosition (LogicalDirection direction)
		{
			return NativeDependencyObjectHelper.FromIntPtr (NativeMethods.text_pointer_get_next_insertion_position (NativeHandle, direction)) as TextPointer;
		}

		public TextPointer GetPositionAtOffset (int offset, LogicalDirection direction)
		{
			return NativeDependencyObjectHelper.FromIntPtr (NativeMethods.text_pointer_get_position_at_offset (NativeHandle, offset, direction)) as TextPointer;
		}
	}
}
