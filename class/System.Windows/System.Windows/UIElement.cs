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

		static UIElement ()
		{
			UIElement.VisibilityProperty.AddPropertyChangeCallback (VisibilityPropertyChanged);
		}

		static void VisibilityPropertyChanged (DependencyObject sender,
		                                       DependencyPropertyChangedEventArgs args)
		{
			((UIElement) sender).RaiseUIAVisibilityChanged (args);
		}

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
		
		internal DependencyObject TemplateOwner {
			get { return (DependencyObject) NativeDependencyObjectHelper.Lookup (Mono.NativeMethods.dependency_object_get_template_owner (native)); }
			set {
				IntPtr owner = value == null ? IntPtr.Zero : value.native;
				Mono.NativeMethods.dependency_object_set_template_owner (native, owner);
			}
		}

#if NET_3_0
		public event MouseButtonEventHandler MouseRightButtonDown {
			add {
				RegisterEvent (EventIds.UIElement_MouseRightButtonDownEvent, value, Events.CreateMouseButtonEventHandlerDispatcher (value));
			}
			remove {
				UnregisterEvent (EventIds.UIElement_MouseRightButtonDownEvent, value);
			}
		}

		public event MouseButtonEventHandler MouseRightButtonUp {
			add {
				RegisterEvent (EventIds.UIElement_MouseRightButtonUpEvent, value, Events.CreateMouseButtonEventHandlerDispatcher (value));
			}
			remove {
				UnregisterEvent (EventIds.UIElement_MouseRightButtonUpEvent, value);
			}
		}
#endif

		public void AddHandler (RoutedEvent routedEvent, Delegate handler, bool handledEventsToo)
		{
			// FIXME: we don't handle handledEventsToo
			RegisterEvent (routedEvent.EventId, handler, Events.CreateDispatcherFromEventId (routedEvent.EventId, handler));
		}
		
		public void RemoveHandler (RoutedEvent routedEvent, Delegate handler)
		{
			UnregisterEvent (routedEvent.EventId, handler);
		}
		
		#region UIA Events

		internal event DependencyPropertyChangedEventHandler UIAVisibilityChanged;

		internal void RaiseUIAVisibilityChanged (DependencyPropertyChangedEventArgs args)
		{
			if (UIAVisibilityChanged != null)
				UIAVisibilityChanged (this, args);
		}

		#endregion
	}
}
