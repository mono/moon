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
	public sealed class TextSelection : INativeDependencyObjectWrapper {
		bool free_mapping;
		IntPtr native;
		
		internal TextSelection (IntPtr raw, bool dropref)
		{
			NativeHandle = raw;
			if (dropref)
				NativeMethods.event_object_unref (raw);
		}
		
		internal TextSelection () : this (SafeNativeMethods.text_selection_new (), true)
		{
		}
		
		internal void Free ()
		{
			if (free_mapping) {
				free_mapping = false;
				NativeDependencyObjectHelper.FreeNativeMapping (this);
			}
		}
		
		~TextSelection ()
		{
			Free ();
		}

		internal IntPtr NativeHandle {
			get { return native; }
			set {
				if (native != IntPtr.Zero) {
					throw new InvalidOperationException ("TextSelection.native is already set");
				}

				native = value;

				free_mapping = NativeDependencyObjectHelper.AddNativeMapping (value, this);
			}
		}

		IntPtr INativeEventObjectWrapper.NativeHandle {
			get { return NativeHandle; }
			set { NativeHandle = value; }
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

		Kind INativeEventObjectWrapper.GetKind ()
		{
			return Kind.TEXTSELECTION;
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

			NativeMethods.text_selection_select (native, anchorPosition.NativeHandle, movingPosition.NativeHandle);
		}

		public string Text {
			get {
				return (string)((INativeDependencyObjectWrapper)this).GetValue(TextProperty);
			}
			set {
				((INativeDependencyObjectWrapper)this).SetValue(TextProperty, value);
			}
		}

		public string Xaml {
			get {
				return (string)((INativeDependencyObjectWrapper)this).GetValue(XamlProperty);
			}
			set {
				((INativeDependencyObjectWrapper)this).SetValue(XamlProperty, value);
			}
		}

		public TextPointer End {
			get {
				return (TextPointer)((INativeDependencyObjectWrapper)this).GetValue(EndProperty);
			}
		}

		public TextPointer Start {
			get {
				return (TextPointer)((INativeDependencyObjectWrapper)this).GetValue(StartProperty);
			}
		}

		private DependencyProperty TextProperty = DependencyProperty.Lookup
			(Kind.TEXTSELECTION, "Text", typeof (string));

		private DependencyProperty XamlProperty = DependencyProperty.Lookup
			(Kind.TEXTSELECTION, "Xaml", typeof (string));

		private DependencyProperty EndProperty = DependencyProperty.Lookup
			(Kind.TEXTSELECTION, "End", typeof (TextPointer));

		private DependencyProperty StartProperty = DependencyProperty.Lookup
			(Kind.TEXTSELECTION, "Start", typeof (TextPointer));
										     
	}
}
