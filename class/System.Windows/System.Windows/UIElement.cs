//
// System.Windows.UIElement.cs
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

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Security;
using System.Windows;
using System.Windows.Automation.Peers;
using System.Windows.Media;
using System.Windows.Input;
using Mono;

namespace System.Windows {
	public abstract partial class UIElement : DependencyObject {

		public bool CaptureMouse ()
		{
			return NativeMethods.uielement_capture_mouse (native);
		}

		public void ReleaseMouseCapture ()
		{
			NativeMethods.uielement_release_mouse_capture (native);
		}

		public void Arrange (Rect finalRect)
		{
			if (finalRect.IsEmpty)
				throw new InvalidOperationException ("Empty Rect");

			NativeMethods.uielement_arrange(native, finalRect);
		}

		public void InvalidateArrange ()
		{
			NativeMethods.uielement_invalidate_arrange(native);
		}

		public void Measure (Size availableSize)
		{
			NativeMethods.uielement_measure (native, availableSize);
		}

		public void InvalidateMeasure ()
		{
			NativeMethods.uielement_invalidate_measure (native);
		}

		public void UpdateLayout ()
		{
			NativeMethods.uielement_update_layout (native);
		}

		public GeneralTransform TransformToVisual (UIElement visual)
		{
			if (visual == null)
				throw new ArgumentException ("visual");

			IntPtr t = NativeMethods.uielement_get_transform_to_uielement (native, visual.native);

			return (GeneralTransform)DependencyObject.Lookup (Kind.GENERALTRANSFORM, t);
		}

		protected virtual AutomationPeer OnCreateAutomationPeer ()
		{
			// there's no automation object associated with UIElement so null is returned
			// it could have been abtract but that that would have forced everyone (without 
			// automation support) to override this default
			return null;
		}

		public Size DesiredSize {
			get {
				return NativeMethods.uielement_get_desired_size (native);
			}
		}

		public Size RenderSize {
			get {
				return NativeMethods.uielement_get_render_size (native);
			}
		}

		static object GotFocusEvent = new object ();
		static object LostFocusEvent = new object ();
		static object LostMouseCaptureEvent = new object ();
		static object KeyDownEvent = new object ();
		static object KeyUpEvent = new object ();
		static object MouseEnterEvent = new object ();
		static object MouseLeaveEvent = new object ();
		static object MouseLeftButtonDownEvent = new object ();
		static object MouseLeftButtonUpEvent = new object ();
		static object MouseMoveEvent = new object ();
		
		public event RoutedEventHandler GotFocus {
			add {
				if (events[GotFocusEvent] == null)
					Events.AddHandler (this, "GotFocus", Events.got_focus);
				events.AddHandler (GotFocusEvent, value);
			}
			remove {
				events.RemoveHandler (GotFocusEvent, value);
				if (events[GotFocusEvent] == null)
					Events.RemoveHandler (this, "GotFocus", Events.got_focus);
			}
		}
		
		public event RoutedEventHandler LostFocus {
			add {
				if (events[LostFocusEvent] == null)
					Events.AddHandler (this, "LostFocus", Events.lost_focus);
				events.AddHandler (LostFocusEvent, value);
			}
			remove {
				events.RemoveHandler (LostFocusEvent, value);
				if (events[LostFocusEvent] == null)
					Events.RemoveHandler (this, "LostFocus", Events.lost_focus);
			}
		}

		public event MouseEventHandler LostMouseCapture {
			add {
				if (events[LostMouseCaptureEvent] == null)
					Events.AddHandler (this, "LostMouseCapture", Events.lost_mouse_capture);
				events.AddHandler (LostMouseCaptureEvent, value);
			}
			remove {
				events.RemoveHandler (LostMouseCaptureEvent, value);
				if (events[LostMouseCaptureEvent] == null)
					Events.RemoveHandler (this, "LostMouseCapture", Events.lost_mouse_capture);
			}
		}

		public event KeyEventHandler KeyDown {
			add {
				if (events[KeyDownEvent] == null)
					Events.AddHandler (this, "KeyDown", Events.key_down);
				events.AddHandler (KeyDownEvent, value);
			}
			remove {
				events.RemoveHandler (KeyDownEvent, value);
				if (events[KeyDownEvent] == null)
					Events.RemoveHandler (this, "KeyDown", Events.key_down);
			}
		}

		public event KeyEventHandler KeyUp {
			add {
				if (events[KeyUpEvent] == null)
					Events.AddHandler (this, "KeyUp", Events.key_up);
				events.AddHandler (KeyUpEvent, value);
			}
			remove {
				events.RemoveHandler (KeyUpEvent, value);
				if (events[KeyUpEvent] == null)
					Events.RemoveHandler (this, "KeyUp", Events.key_up);
			}
		}

		public event MouseEventHandler MouseEnter {
			add {
				if (events[MouseEnterEvent] == null)
					Events.AddHandler (this, "MouseEnter", Events.mouse_enter);
				events.AddHandler (MouseEnterEvent, value);
			}
			remove {
				events.RemoveHandler (MouseEnterEvent, value);
				if (events[MouseEnterEvent] == null)
					Events.RemoveHandler (this, "MouseEnter", Events.mouse_enter);
			}
		}

		public event MouseEventHandler MouseLeave {
			add {
				if (events[MouseLeaveEvent] == null)
					Events.AddHandler (this, "MouseLeave", Events.mouse_leave);
				events.AddHandler (MouseLeaveEvent, value);
			}
			remove {
				events.RemoveHandler (MouseLeaveEvent, value);
				if (events[MouseLeaveEvent] == null)
					Events.RemoveHandler (this, "MouseLeave", Events.mouse_leave);
			}
		}

		public event MouseButtonEventHandler MouseLeftButtonDown {
			add {
				if (events[MouseLeftButtonDownEvent] == null)
					Events.AddHandler (this, "MouseLeftButtonDown", Events.mouse_button_down);
				events.AddHandler (MouseLeftButtonDownEvent, value);
			}
			remove {
				events.RemoveHandler (MouseLeftButtonDownEvent, value);
				if (events[MouseLeftButtonDownEvent] == null)
					Events.RemoveHandler (this, "MouseLeftButtonDown", Events.mouse_button_down);
			}
		}

		public event MouseButtonEventHandler MouseLeftButtonUp {
			add {
				if (events[MouseLeftButtonUpEvent] == null)
					Events.AddHandler (this, "MouseLeftButtonUp", Events.mouse_button_up);
				events.AddHandler (MouseLeftButtonUpEvent, value);
			}
			remove {
				events.RemoveHandler (MouseLeftButtonUpEvent, value);
				if (events[MouseLeftButtonUpEvent] == null)
					Events.RemoveHandler (this, "MouseLeftButtonUp", Events.mouse_button_up);
			}
		}

		public event MouseEventHandler MouseMove {
			add {
				if (events[MouseMoveEvent] == null)
					Events.AddHandler (this, "MouseMove", Events.mouse_motion);
				events.AddHandler (MouseMoveEvent, value);
			}
			remove {
				events.RemoveHandler (MouseMoveEvent, value);
				if (events[MouseMoveEvent] == null)
					Events.RemoveHandler (this, "MouseMove", Events.mouse_motion);
			}
		}


		internal virtual void InvokeGotFocus (RoutedEventArgs r)
		{
			RoutedEventHandler h = (RoutedEventHandler)events[GotFocusEvent];
			if (h != null)
				h (this, r);
		}

		internal virtual void InvokeLostFocus (RoutedEventArgs r)
		{
			RoutedEventHandler h = (RoutedEventHandler)events[LostFocusEvent];
			if (h != null)
				h (this, r);
		}

		internal virtual void InvokeLostMouseCapture (MouseEventArgs m)
		{
			MouseEventHandler h = (MouseEventHandler) events [LostMouseCaptureEvent];
			if (h != null)
				h (this, m);
		}
		
		internal virtual void InvokeMouseMove (MouseEventArgs m)
		{
			MouseEventHandler h = (MouseEventHandler)events[MouseMoveEvent];
			if (h != null)
				h (this, m);
		}

		internal virtual void InvokeMouseButtonDown (MouseButtonEventArgs e)
		{
			MouseButtonEventHandler h = (MouseButtonEventHandler)events[MouseLeftButtonDownEvent];
			if (h != null)
				h (this, e);
		}

		internal virtual void InvokeMouseButtonUp (MouseButtonEventArgs e)
		{
			MouseButtonEventHandler h = (MouseButtonEventHandler)events[MouseLeftButtonUpEvent];
			if (h != null)
				h (this, e);
		}

		internal virtual void InvokeKeyDown (KeyEventArgs k)
		{
			KeyEventHandler h = (KeyEventHandler)events[KeyDownEvent];
			if (h != null)
				h (this, k);
		}

		internal virtual void InvokeKeyUp (KeyEventArgs k)
		{
			KeyEventHandler h = (KeyEventHandler)events[KeyUpEvent];
			if (h != null)
				h (this, k);
		}

		internal virtual void InvokeMouseLeave (MouseEventArgs e)
		{
			MouseEventHandler h = (MouseEventHandler)events[MouseLeaveEvent];
			if (h != null)
				h (this, e);
		}

		internal virtual void InvokeMouseEnter (MouseEventArgs m)
		{
			MouseEventHandler h = (MouseEventHandler)events[MouseEnterEvent];
			if (h != null)
				h (this, m);
		}
	}
}
