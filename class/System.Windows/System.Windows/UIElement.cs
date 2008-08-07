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
using System.Security;
using System.Windows;
#if notyet
using System.Windows.Automation.Peers;
#endif
using System.Windows.Media;
using System.Windows.Input;
using Mono;

namespace System.Windows {
	public abstract class UIElement : DependencyObject {
		public static readonly DependencyProperty ClipProperty =
			DependencyProperty.Lookup (Kind.UIELEMENT, "Clip", typeof (Geometry));
		
		public static readonly DependencyProperty IsHitTestVisibleProperty =
			DependencyProperty.Lookup (Kind.UIELEMENT, "IsHitTestVisible", typeof (bool));
		
		public static readonly DependencyProperty OpacityMaskProperty =
			DependencyProperty.Lookup (Kind.UIELEMENT, "OpacityMask", typeof (Brush));
		
		public static readonly DependencyProperty OpacityProperty =
			DependencyProperty.Lookup (Kind.UIELEMENT, "Opacity", typeof (double));
		
		public static readonly DependencyProperty RenderTransformOriginProperty =
			DependencyProperty.Lookup (Kind.UIELEMENT, "RenderTransformOrigin", typeof (Point));
		
		public static readonly DependencyProperty RenderTransformProperty =
			DependencyProperty.Lookup (Kind.UIELEMENT, "RenderTransform", typeof (Transform));
		
		public static readonly DependencyProperty VisibilityProperty =
			DependencyProperty.Lookup (Kind.UIELEMENT, "Visibility", typeof (Visibility));
		
		public static readonly DependencyProperty ZIndexProperty =
			DependencyProperty.Lookup (Kind.UIELEMENT, "ZIndex", typeof (int));
		
		internal UIElement () : base (NativeMethods.uielement_new ())
		{
		}
		
		internal UIElement (IntPtr raw) : base (raw)
		{
		}
		
		internal override Kind GetKind ()
		{
			return Kind.UIELEMENT;
		}
		
#if NET_2_1
		[SecuritySafeCritical]
#endif
		public bool CaptureMouse ()
		{
			return NativeMethods.uielement_capture_mouse (native);
		}

#if NET_2_1
		[SecuritySafeCritical]
#endif
		public void ReleaseMouseCapture ()
		{
			NativeMethods.uielement_release_mouse_capture (native);
		}

#if NET_2_1
		[SecuritySafeCritical]
#endif
		public void Arrange (Rect r)
		{
			throw new NotImplementedException ();
		}

#if NET_2_1
		[SecuritySafeCritical]
#endif
		public void InvalidateArrange ()
		{
			throw new NotImplementedException ();
		}

#if NET_2_1
		[SecuritySafeCritical]
#endif
		public void Measure (Size s)
		{
			throw new NotImplementedException ();
		}

#if NET_2_1
		[SecuritySafeCritical]
#endif
		public void InvalidateMeasure ()
		{
			throw new NotImplementedException ();
		}

#if NET_2_1
		[SecuritySafeCritical]
#endif
		public void UpdateLayout ()
		{
			throw new NotImplementedException ();
		}

#if NET_2_1
		[SecuritySafeCritical]
#endif
		public GeneralTransform TransformToVisual (UIElement el)
		{
			throw new NotImplementedException ();
		}


#if NET_2_1
		[SecuritySafeCritical]
#endif
		public IEnumerable<UIElement> HitTest (Point p)
		{
			throw new NotImplementedException ();
		}

#if NET_2_1
		[SecuritySafeCritical]
#endif
		public IEnumerable<UIElement> HitTest (Rect r)
		{
			throw new NotImplementedException ();
		}

#if notyet
#if NET_2_1
		[SecuritySafeCritical]
#endif
		public AutomationPeer OnCreateAutomationPeer ()
		{
			throw new NotImplementedException ();
		}
#endif

		public Geometry Clip {
			get {
				return (Geometry) GetValue (ClipProperty);
			}
			set {
				SetValue (ClipProperty, value);
			}
		}
		
		public bool IsHitTestVisible {
			get {
				return (bool) GetValue (IsHitTestVisibleProperty);
			}
			set {
				SetValue (IsHitTestVisibleProperty, value);
			}
		}
		
		public double Opacity {
			get {
				return (double) GetValue (OpacityProperty);
			}
			set {
				SetValue (OpacityProperty, value);
			}
		}
		
		public Brush OpacityMask {
			get {
				return (Brush) GetValue (OpacityMaskProperty);
			}
			set {
				SetValue (OpacityMaskProperty, value);
			}
		}
		
		public Transform RenderTransform {
			get {
				return (Transform) GetValue (RenderTransformProperty);
			}
			set {
				SetValue (RenderTransformProperty, value);
			}
		}
		
		public Point RenderTransformOrigin {
			get {
				return (Point) GetValue (RenderTransformOriginProperty);
			}
			set {
				SetValue (RenderTransformOriginProperty, value);
			}
		}
		
		public Visibility Visibility {
			get {
				return (Visibility) GetValue (VisibilityProperty);
			}
			set {
				SetValue (VisibilityProperty, value);
			}
		}

		/* MS has removed this from the public API as of 1.0 RC1 / 1.1 refresh... not sure why */
		internal int ZIndex {
			get {
				return (int) GetValue (ZIndexProperty);
			}
			set {
				SetValue (ZIndexProperty, value);
			}
		}

		public Size DesiredSize {
#if NET_2_1
			[SecuritySafeCritical]
#endif
			get {
				return System.Windows.NativeMethods.uielement_get_desired_size (native);
			}
		}

		public Size RenderSize {
			get {
				throw new NotImplementedException ();
			}
		}

		static object GotFocusEvent = new object ();
		static object LostFocusEvent = new object ();
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


		internal void InvokeGotFocus ()
		{
			EventHandler h = (EventHandler)events[GotFocusEvent];
			if (h != null)
				h (this, EventArgs.Empty);
		}

		internal void InvokeLostFocus ()
		{
			EventHandler h = (EventHandler)events[LostFocusEvent];
			if (h != null)
				h (this, EventArgs.Empty);
		}

		internal void InvokeMouseMove (MouseEventArgs m)
		{
			MouseEventHandler h = (MouseEventHandler)events[MouseMoveEvent];
			if (h != null)
				h (this, m);
		}

		internal void InvokeMouseButtonDown (MouseButtonEventArgs m)
		{
			MouseButtonEventHandler h = (MouseButtonEventHandler)events[MouseLeftButtonDownEvent];
			if (h != null)
				h (this, m);
		}

		internal void InvokeMouseButtonUp (MouseButtonEventArgs m)
		{
			MouseButtonEventHandler h = (MouseButtonEventHandler)events[MouseLeftButtonUpEvent];
			if (h != null)
				h (this, m);
		}

		internal void InvokeKeyDown (KeyEventArgs k)
		{
			KeyEventHandler h = (KeyEventHandler)events[KeyDownEvent];
			if (h != null)
				h (this, k);
		}

		internal void InvokeKeyUp (KeyEventArgs k)
		{
			KeyEventHandler h = (KeyEventHandler)events[KeyUpEvent];
			if (h != null)
				h (this, k);
		}

		internal void InvokeMouseLeave ()
		{
			EventHandler h = (EventHandler)events[MouseLeaveEvent];
			if (h != null)
				h (this, null);
		}

		internal void InvokeMouseEnter (MouseEventArgs m)
		{
			MouseEventHandler h = (MouseEventHandler)events[MouseEnterEvent];
			if (h != null)
				h (this, m);
		}
	}
}
