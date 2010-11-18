//
// RichTextBox.cs
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

using Mono;
using System;
using System.Windows;
using System.Windows.Documents;
using System.Windows.Input;

namespace System.Windows.Controls {
	[TemplatePart (Name = "ContentElement", Type = typeof (FrameworkElement))]
	[TemplateVisualState (Name = "MouseOver", GroupName = "CommonStates")]
	[TemplateVisualState (Name = "InvalidUnfocused", GroupName = "ValidationStates")]
	[TemplateVisualState (Name = "Focused", GroupName = "FocusStates")]
	[TemplateVisualState (Name = "Unfocused", GroupName = "FocusStates")]
	[TemplateVisualState (Name = "ReadOnly", GroupName= "CommonStates")]
	[TemplateVisualState (Name = "Valid", GroupName = "ValidationStates")]
	[TemplateVisualState (Name = "InvalidFocused", GroupName = "ValidationStates")]
	[TemplateVisualState (Name = "Disabled", GroupName = "CommonStates")]
	[TemplateVisualState (Name = "Normal", GroupName = "CommonStates")]
	public partial class RichTextBox : Control {
		bool IsMouseOver {
			get; set;
		}

		protected override void OnGotFocus (RoutedEventArgs e)
		{
			base.OnGotFocus (e);
			ChangeVisualState ();
			NativeMethods.rich_text_box_on_got_focus (native, e.NativeHandle);
		}

		protected override void OnKeyDown (KeyEventArgs e)
		{
			base.OnKeyDown (e);
			NativeMethods.rich_text_box_on_key_down (native, e.NativeHandle);
		}

		internal override void PostOnKeyDown (KeyEventArgs k)
		{
			base.PostOnKeyDown (k);
			NativeMethods.rich_text_box_post_on_key_down (native, k.NativeHandle);
		}

		protected override void OnKeyUp (KeyEventArgs e)
		{
			base.OnKeyUp (e);
			NativeMethods.rich_text_box_on_key_up (native, e.NativeHandle);
		}

		protected override void OnLostFocus (RoutedEventArgs e)
		{
			base.OnLostFocus (e);
			ChangeVisualState ();
			NativeMethods.rich_text_box_on_lost_focus (native, e.NativeHandle);
		}

		protected override void OnMouseEnter (MouseEventArgs e)
		{
			IsMouseOver = true;
			ChangeVisualState ();
			base.OnMouseEnter (e);
		}

		protected override void OnMouseLeave (MouseEventArgs e)
		{
			IsMouseOver = false;
			ChangeVisualState ();
			base.OnMouseLeave (e);
		}

		protected override void OnMouseLeftButtonDown (MouseButtonEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");

			NativeMethods.rich_text_box_on_mouse_left_button_down (native, e.NativeHandle);
			base.OnMouseLeftButtonDown (e);
		}

		protected override void OnMouseLeftButtonUp (MouseButtonEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");

			NativeMethods.rich_text_box_on_mouse_left_button_up (native, e.NativeHandle);
			base.OnMouseLeftButtonUp (e);
		}

		protected override void OnMouseMove (MouseEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");

			NativeMethods.rich_text_box_on_mouse_move (native, e.NativeHandle);
			base.OnMouseMove (e);
		}

		protected override void OnTextInput (TextCompositionEventArgs e)
		{
			base.OnTextInput (e);
		}

		protected override void OnTextInputStart (TextCompositionEventArgs e)
		{
			base.OnTextInputStart (e);
		}

		protected override void OnTextInputUpdate (TextCompositionEventArgs e)
		{
			base.OnTextInputUpdate (e);
		}

		protected override void OnLostMouseCapture (MouseEventArgs e)
		{
			base.OnLostMouseCapture (e);
		}

		protected override Automation.Peers.AutomationPeer OnCreateAutomationPeer ()
		{
			return base.OnCreateAutomationPeer ();
		}

		public void SelectAll ()
		{
			NativeMethods.rich_text_box_select_all (native);
		}

		public TextPointer GetPositionFromPoint (Point point)
		{
			IntPtr tp = NativeMethods.rich_text_box_get_position_from_point (native, point);
			return tp == IntPtr.Zero ? null : new TextPointer (tp);
		}

		TextSelection selection;
		public TextSelection Selection {
			get {
				if (selection == null) {
					IntPtr ts = NativeMethods.rich_text_box_get_selection (native);
					if (ts != IntPtr.Zero)
						selection = new TextSelection (ts);
				}
				return selection;
			}
		}

		TextPointer contentStart;
		public TextPointer ContentStart {
			get {
				if (contentStart == null) {
					IntPtr tp = NativeMethods.rich_text_box_get_content_start (native);
					if (tp != IntPtr.Zero)
						contentStart = new TextPointer (tp);
				}
				return contentStart;
			}
		}

		TextPointer contentEnd;
		public TextPointer ContentEnd {
			get {
				if (contentEnd == null) {
					IntPtr tp = NativeMethods.rich_text_box_get_content_end (native);
					if (tp != IntPtr.Zero)
						contentEnd = new TextPointer (tp);
				}
				return contentEnd;
			}
		}


		void ChangeVisualState ()
		{
			ChangeVisualState (true);
		}

		void ChangeVisualState (bool useTransitions)
		{
			if (!IsEnabled) {
				VisualStateManager.GoToState (this, "Disabled", useTransitions);
			} else if (IsReadOnly) {
				VisualStateManager.GoToState (this, "ReadOnly", useTransitions);
			} else if (IsMouseOver) {
				VisualStateManager.GoToState (this, "MouseOver", useTransitions);
			} else {
				VisualStateManager.GoToState (this, "Normal", useTransitions);
			}

			if (Focused) {
				VisualStateManager.GoToState (this, "Focused", useTransitions);
			} else {
				VisualStateManager.GoToState (this, "Unfocused", useTransitions);
			}
		}
	}
}

