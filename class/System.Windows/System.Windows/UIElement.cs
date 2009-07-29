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
using System.Windows;
using System.Windows.Automation.Peers;
using System.Windows.Media;
using System.Windows.Input;
using Mono;

namespace System.Windows {
	public abstract partial class UIElement : DependencyObject {

		public Transform RenderTransform {
			get {
				Transform t = (Transform)GetValue (RenderTransformProperty);
				return t == null ? new MatrixTransform () : t;
			}
			set {
				if (value == null)
					ClearValue (RenderTransformProperty);
				else
					SetValue (RenderTransformProperty, value);
			}
		}

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

			if (Double.IsInfinity (finalRect.Width) || Double.IsInfinity (finalRect.Height) || Double.IsInfinity (finalRect.X) || Double.IsInfinity (finalRect.Y))
				throw new InvalidOperationException ("Infinite Rect");
			if (Double.IsNaN (finalRect.Width) || Double.IsNaN (finalRect.Height) || Double.IsNaN (finalRect.X) || Double.IsNaN (finalRect.Y))
				throw new InvalidOperationException ("NaN Rect");

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
			IntPtr t = NativeMethods.uielement_get_transform_to_uielement (native, visual == null ? IntPtr.Zero : visual.native);

			return (MatrixTransform) NativeDependencyObjectHelper.Lookup (Kind.MATRIXTRANSFORM, t);
		}

		protected virtual AutomationPeer OnCreateAutomationPeer ()
		{
			// there's no automation object associated with UIElement so null is returned
			// it could have been abtract but that that would have forced everyone (without 
			// automation support) to override this default
			return null;
		}

		internal AutomationPeer AutomationPeer {
			get; set;
		}
			
		// needed by FrameworkElementAutomationPeer
		internal AutomationPeer CreateAutomationPeer ()
		{
			return OnCreateAutomationPeer ();
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
#if NET_3_0
		static object MouseRightButtonDownEvent = new object ();
		static object MouseRightButtonUpEvent = new object ();
		static object MouseWheelEvent = new object ();
#endif
		
		public event RoutedEventHandler GotFocus {
			add {
				RegisterEvent (GotFocusEvent, "GotFocus", Events.got_focus, value);
			}
			remove {
				UnregisterEvent (GotFocusEvent, "GotFocus", Events.got_focus, value);
			}
		}
		
		public event RoutedEventHandler LostFocus {
			add {
				RegisterEvent (LostFocusEvent, "LostFocus", Events.lost_focus, value);
			}
			remove {
				UnregisterEvent (LostFocusEvent, "LostFocus", Events.lost_focus, value);
			}
		}

		public event MouseEventHandler LostMouseCapture {
			add {
				RegisterEvent (LostMouseCaptureEvent, "LostMouseCapture", Events.lost_mouse_capture, value);
			}
			remove {
				UnregisterEvent (LostMouseCaptureEvent, "LostMouseCapture", Events.lost_mouse_capture, value);
			}
		}

		public event KeyEventHandler KeyDown {
			add {
				RegisterEvent (KeyDownEvent, "KeyDown", Events.key_down, value);
			}
			remove {
				UnregisterEvent (KeyDownEvent, "KeyDown", Events.key_down, value);
			}
		}

		public event KeyEventHandler KeyUp {
			add {
				RegisterEvent (KeyUpEvent, "KeyUp", Events.key_up, value);
			}
			remove {
				UnregisterEvent (KeyUpEvent, "KeyUp", Events.key_up, value);
			}
		}

		public event MouseEventHandler MouseEnter {
			add {
				RegisterEvent (MouseEnterEvent, "MouseEnter", Events.mouse_enter, value);
			}
			remove {
				UnregisterEvent (MouseEnterEvent, "MouseEnter", Events.mouse_enter, value);
			}
		}

		public event MouseEventHandler MouseLeave {
			add {
				RegisterEvent (MouseLeaveEvent, "MouseLeave", Events.mouse_leave, value);
			}
			remove {
				UnregisterEvent (MouseLeaveEvent, "MouseLeave", Events.mouse_leave, value);
			}
		}

		public event MouseButtonEventHandler MouseLeftButtonDown {
			add {
				RegisterEvent (MouseLeftButtonDownEvent, "MouseLeftButtonDown", Events.mouse_left_button_down, value);
			}
			remove {
				UnregisterEvent (MouseLeftButtonDownEvent, "MouseLeftButtonDown", Events.mouse_left_button_down, value);
			}
		}

		public event MouseButtonEventHandler MouseLeftButtonUp {
			add {
				RegisterEvent (MouseLeftButtonUpEvent, "MouseLeftButtonUp", Events.mouse_left_button_up, value);
			}
			remove {
				UnregisterEvent (MouseLeftButtonUpEvent, "MouseLeftButtonUp", Events.mouse_left_button_up, value);
			}
		}

#if NET_3_0
		public event MouseButtonEventHandler MouseRightButtonDown {
			add {
				RegisterEvent (MouseRightButtonDownEvent, "MouseRightButtonDown", Events.mouse_right_button_down, value);
			}
			remove {
				UnregisterEvent (MouseRightButtonDownEvent, "MouseRightButtonDown", Events.mouse_right_button_down, value);
			}
		}

		public event MouseButtonEventHandler MouseRightButtonUp {
			add {
				RegisterEvent (MouseRightButtonUpEvent, "MouseRightButtonUp", Events.mouse_right_button_up, value);
			}
			remove {
				UnregisterEvent (MouseRightButtonUpEvent, "MouseRightButtonUp", Events.mouse_right_button_up, value);
			}
		}

		public event MouseWheelEventHandler MouseWheel {
			add {
				RegisterEvent (MouseWheelEvent, "MouseWheel", Events.mouse_wheel, value);
			}
			remove {
				UnregisterEvent (MouseWheelEvent, "MouseWheel", Events.mouse_wheel, value);
			}
		}
#endif


		public event MouseEventHandler MouseMove {
			add {
				RegisterEvent (MouseMoveEvent, "MouseMove", Events.mouse_motion, value);
			}
			remove {
				UnregisterEvent (MouseMoveEvent, "MouseMove", Events.mouse_motion, value);
			}
		}


		internal virtual void InvokeGotFocus (RoutedEventArgs r)
		{
			RoutedEventHandler h = (RoutedEventHandler) EventList [GotFocusEvent];
			if (h != null)
				h (this, r);
		}

		internal virtual void InvokeLostFocus (RoutedEventArgs r)
		{
			RoutedEventHandler h = (RoutedEventHandler) EventList [LostFocusEvent];
			if (h != null)
				h (this, r);
		}

		internal virtual void InvokeLostMouseCapture (MouseEventArgs m)
		{
			MouseEventHandler h = (MouseEventHandler) EventList [LostMouseCaptureEvent];
			if (h != null)
				h (this, m);
		}
		
		internal virtual void InvokeMouseMove (MouseEventArgs m)
		{
			MouseEventHandler h = (MouseEventHandler) EventList [MouseMoveEvent];
			if (h != null)
				h (this, m);
		}

		internal virtual void InvokeMouseLeftButtonDown (MouseButtonEventArgs e)
		{
			MouseButtonEventHandler h = (MouseButtonEventHandler) EventList [MouseLeftButtonDownEvent];
			if (h != null)
				h (this, e);
		}

		internal virtual void InvokeMouseLeftButtonUp (MouseButtonEventArgs e)
		{
			MouseButtonEventHandler h = (MouseButtonEventHandler) EventList [MouseLeftButtonUpEvent];
			if (h != null)
				h (this, e);
		}

#if NET_3_0
		internal virtual void InvokeMouseRightButtonDown (MouseButtonEventArgs e)
		{
			MouseButtonEventHandler h = (MouseButtonEventHandler) EventList [MouseRightButtonDownEvent];
			if (h != null)
				h (this, e);
		}

		internal virtual void InvokeMouseRightButtonUp (MouseButtonEventArgs e)
		{
			MouseButtonEventHandler h = (MouseButtonEventHandler) EventList [MouseRightButtonUpEvent];
			if (h != null)
				h (this, e);
		}

		internal virtual void InvokeMouseWheel (MouseWheelEventArgs e)
		{
			MouseWheelEventHandler h = (MouseWheelEventHandler) EventList [MouseWheelEvent];
			if (h != null)
				h (this, e);
		}
#endif
		internal virtual void InvokeKeyDown (KeyEventArgs k)
		{
			
		}
		
		internal virtual void RaiseKeyDown (KeyEventArgs k)
		{
			if (k.Handled)
				return;

			KeyEventHandler h = (KeyEventHandler) EventList [KeyDownEvent];
			if (h != null)
				h (this, k);
		}

		internal virtual void InvokeKeyUp (KeyEventArgs k)
		{
			KeyEventHandler h = (KeyEventHandler) EventList [KeyUpEvent];
			if (h != null)
				h (this, k);
		}

		internal virtual void InvokeMouseLeave (MouseEventArgs e)
		{
			MouseEventHandler h = (MouseEventHandler) EventList [MouseLeaveEvent];
			if (h != null)
				h (this, e);
		}

		internal virtual void InvokeMouseEnter (MouseEventArgs m)
		{
			MouseEventHandler h = (MouseEventHandler) EventList [MouseEnterEvent];
			if (h != null)
				h (this, m);
		}
	}
}
