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
using System;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Documents;
using Mono;

namespace System.Windows.Controls
{
	[TemplatePartAttribute(Name = "ContentElement", Type = typeof(FrameworkElement))]
	[TemplateVisualStateAttribute(Name = "Disabled", GroupName = "CommonStates")]
	[TemplatePartAttribute(Name = "DisabledVisualElement", Type = typeof(FrameworkElement))]
	[TemplateVisualStateAttribute(Name = "Focused", GroupName = "FocusStates")]
	[TemplatePartAttribute(Name = "FocusVisualElement", Type = typeof(FrameworkElement))]
	[TemplateVisualStateAttribute(Name = "MouseOver", GroupName = "CommonStates")]
	[TemplateVisualStateAttribute(Name = "Normal", GroupName = "CommonStates")]
	[TemplatePartAttribute(Name = "RootElement", Type = typeof(FrameworkElement))]
	[TemplateVisualStateAttribute(Name = "Unfocused", GroupName = "FocusStates")]
	public sealed partial class PasswordBox : Control {
		object contentElement;
		
		bool IsFocused {
			get; set;
		}
		
		bool IsMouseOver {
			get; set;
		}
		
		void Initialize ()
		{
			CursorPositionChanged += OnCursorPositionChanged;
			IsEnabledChanged += delegate { ChangeVisualState (); };
			Loaded += delegate { ChangeVisualState (); };
		}
		
		internal override void InvokeKeyDown (KeyEventArgs k)
		{
			base.InvokeKeyDown (k);
			if (!k.Handled)
				NativeMethods.text_box_base_on_character_key_down (native, k.NativeHandle);
		}
		
		protected override void OnKeyDown (KeyEventArgs k)
		{
			// Chain up to our parent first, so that TabNavigation
			// works as well as allowing developers to filter our
			// input.
			base.OnKeyDown (k);
			
			if (!k.Handled)
				NativeMethods.text_box_base_on_key_down (native, k.NativeHandle);
		}
		
		protected override void OnKeyUp (KeyEventArgs k)
		{
			base.OnKeyUp (k);
			
			if (!k.Handled)
				NativeMethods.text_box_base_on_key_up (native, k.NativeHandle);
		}
		
		protected override void OnMouseLeftButtonDown (MouseButtonEventArgs e)
		{
			if (!e.Handled)
				NativeMethods.text_box_base_on_mouse_left_button_down (native, e.NativeHandle);
			
			base.OnMouseLeftButtonDown (e);
		}
		
		protected override void OnMouseLeftButtonUp (MouseButtonEventArgs e)
		{
			if (!e.Handled)
				NativeMethods.text_box_base_on_mouse_left_button_up (native, e.NativeHandle);
			
			base.OnMouseLeftButtonUp (e);
		}
		
		protected override void OnMouseMove (MouseEventArgs e)
		{
			NativeMethods.text_box_base_on_mouse_move (native, e.NativeHandle);
			base.OnMouseMove (e);
		}
		
		protected override void OnMouseEnter (MouseEventArgs e)
		{
			IsMouseOver = true;
			base.OnMouseEnter (e);
			ChangeVisualState ();
		}
		
		protected override void OnMouseLeave (MouseEventArgs e)
		{
			IsMouseOver = false;
			base.OnMouseLeave (e);
			ChangeVisualState ();
		}
		
		protected override void OnGotFocus (RoutedEventArgs e)
		{
			IsFocused = true;
			base.OnGotFocus (e);
			NativeMethods.text_box_base_on_got_focus (native, e.NativeHandle);
			ChangeVisualState ();
		}
		
		protected override void OnLostFocus (RoutedEventArgs e)
		{
			IsFocused = false;
			base.OnLostFocus (e);
			NativeMethods.text_box_base_on_lost_focus (native, e.NativeHandle);
			ChangeVisualState ();
		}
		
		public void SelectAll ()
		{
			NativeMethods.text_box_base_select_all (native);
		}
		
		static UnmanagedEventHandler password_changed = Events.CreateSafeHandler (password_changed_cb);
		static object PasswordChangedEvent = new object ();
		
		void InvokePasswordChanged (RoutedEventArgs args)
		{
			RoutedEventHandler h = (RoutedEventHandler) EventList [PasswordChangedEvent];
			
			if (h != null)
				h (this, args);
		}
		
		static void password_changed_cb (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			PasswordBox passwordbox = (PasswordBox) NativeDependencyObjectHelper.FromIntPtr (closure);
			RoutedEventArgs args = new RoutedEventArgs (calldata, false);
			
			passwordbox.InvokePasswordChanged (args);
		}
		
		public event RoutedEventHandler PasswordChanged {
			add {
				RegisterEvent (PasswordChangedEvent, "PasswordChanged", password_changed, value);
			}
			remove {
				UnregisterEvent (PasswordChangedEvent, "PasswordChanged", password_changed, value);
			}
		}
		
		static UnmanagedEventHandler cursor_position_changed = Events.CreateSafeHandler (cursor_position_changed_cb);
		
		static object CursorPositionChangedEvent = new object ();
		
		void OnCursorPositionChanged (object sender, CursorPositionChangedEventArgs args)
		{
			if (contentElement == null)
				contentElement = GetTemplateChild ("ContentElement");
			
			if (contentElement != null && contentElement is ScrollViewer) {
				ScrollViewer scrollview = contentElement as ScrollViewer;
				double offset = scrollview.HorizontalOffset;
				
				// Note: for horizontal scrollage, we offset by 1.0 pixel for the width of the cursor ibeam
				
				if (args.CursorX < offset) {
					// need to scroll to the left a bit
					scrollview.ScrollToHorizontalOffset (Math.Max (args.CursorX - 1.0, 0.0));
				} else if (args.CursorX > offset + scrollview.ViewportWidth) {
					// need to scroll to the right
					offset = (args.CursorX + 1.0) - scrollview.ViewportWidth;
					scrollview.ScrollToHorizontalOffset (offset);
				}
				
				offset = scrollview.VerticalOffset;
				if (args.CursorY < offset) {
					// need to scroll up a bit
					scrollview.ScrollToVerticalOffset (args.CursorY);
				} else if (args.CursorY + args.CursorHeight > offset + scrollview.ViewportHeight) {
					// need to scroll down a bit
					offset = (args.CursorY + args.CursorHeight) - Math.Max (args.CursorHeight, scrollview.ViewportHeight);
					scrollview.ScrollToVerticalOffset (offset);
				}
			}
		}
		
		void InvokeCursorPositionChanged (CursorPositionChangedEventArgs args)
		{
			CursorPositionChangedEventHandler h = (CursorPositionChangedEventHandler) EventList [CursorPositionChangedEvent];
			
			if (h != null)
				h (this, args);
		}
		
		static void cursor_position_changed_cb (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			PasswordBox passwdbox = (PasswordBox) NativeDependencyObjectHelper.FromIntPtr (closure);
			CursorPositionChangedEventArgs args = new CursorPositionChangedEventArgs (calldata);
			
			passwdbox.InvokeCursorPositionChanged (args);
		}
		
		event CursorPositionChangedEventHandler CursorPositionChanged {
			add {
				RegisterEvent (CursorPositionChangedEvent, "CursorPositionChanged", cursor_position_changed, value);
			}
			remove {
				UnregisterEvent (CursorPositionChangedEvent, "CursorPositionChanged", cursor_position_changed, value);           				
			}
		}
		
		void ChangeVisualState ()
		{
			if (!IsEnabled) {
				VisualStateManager.GoToState (this, "Disabled", true);
			} else if (IsMouseOver) {
				VisualStateManager.GoToState (this, "MouseOver", true);
			} else {
				VisualStateManager.GoToState (this, "Normal", true);
			}
			
			if (IsFocused) {
				VisualStateManager.GoToState (this, "Focused", true);
			} else {
				VisualStateManager.GoToState (this, "Unfocused", true);
			}
		}
	}
}
