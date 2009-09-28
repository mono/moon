//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2007-2008 Novell, Inc (http://www.novell.com)
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
using Mono.Xaml;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Markup;

namespace System.Windows.Controls {
	public abstract partial class Control : FrameworkElement {
		static UnmanagedEventHandler template_applied = Events.SafeDispatcher (
			    (IntPtr target, IntPtr calldata, IntPtr closure) =>
			    	((Control) NativeDependencyObjectHelper.FromIntPtr (closure)).InvokeOnApplyTemplate ());

		static UnmanagedEventHandler on_got_focus = Events.SafeDispatcher (
			    (IntPtr target, IntPtr calldata, IntPtr closure) =>
			    	((Control) NativeDependencyObjectHelper.FromIntPtr (closure)).OnGotFocus (NativeDependencyObjectHelper.FromIntPtr (calldata) as RoutedEventArgs ?? new RoutedEventArgs (calldata, false)) );

		static UnmanagedEventHandler on_lost_focus = Events.SafeDispatcher (
			    (IntPtr target, IntPtr calldata, IntPtr closure) =>
			    	((Control) NativeDependencyObjectHelper.FromIntPtr (closure)).OnLostFocus (NativeDependencyObjectHelper.FromIntPtr (calldata) as RoutedEventArgs ?? new RoutedEventArgs (calldata, false)));

		static UnmanagedEventHandler on_key_down = Events.SafeDispatcher (
			    (IntPtr target, IntPtr calldata, IntPtr closure) =>
			    	((Control) NativeDependencyObjectHelper.FromIntPtr (closure)).OnKeyDown (NativeDependencyObjectHelper.FromIntPtr (calldata) as KeyEventArgs ?? new KeyEventArgs (calldata)));

		static UnmanagedEventHandler on_key_up = Events.SafeDispatcher (
			    (IntPtr target, IntPtr calldata, IntPtr closure) =>
			    	((Control) NativeDependencyObjectHelper.FromIntPtr (closure)).OnKeyUp (NativeDependencyObjectHelper.FromIntPtr (calldata) as KeyEventArgs ?? new KeyEventArgs (calldata)));

		static UnmanagedEventHandler on_mouse_enter = Events.SafeDispatcher (
			    (IntPtr target, IntPtr calldata, IntPtr closure) =>
			    	((Control) NativeDependencyObjectHelper.FromIntPtr (closure)).OnMouseEnter (NativeDependencyObjectHelper.FromIntPtr (calldata) as MouseEventArgs ?? new MouseEventArgs (calldata)));

		static UnmanagedEventHandler on_mouse_leave = Events.SafeDispatcher (
			    (IntPtr target, IntPtr calldata, IntPtr closure) =>
			    	((Control) NativeDependencyObjectHelper.FromIntPtr (closure)).OnMouseLeave (NativeDependencyObjectHelper.FromIntPtr (calldata) as MouseEventArgs ?? new MouseEventArgs (calldata)));

		static UnmanagedEventHandler on_mouse_move = Events.SafeDispatcher (
			    (IntPtr target, IntPtr calldata, IntPtr closure) =>
			    	((Control) NativeDependencyObjectHelper.FromIntPtr (closure)).OnMouseMove (NativeDependencyObjectHelper.FromIntPtr (calldata) as MouseEventArgs ?? new MouseEventArgs (calldata)));

		static UnmanagedEventHandler on_mouse_left_button_down = Events.SafeDispatcher (
			    (IntPtr target, IntPtr calldata, IntPtr closure) =>
			    	((Control) NativeDependencyObjectHelper.FromIntPtr (closure)).OnMouseLeftButtonDown (NativeDependencyObjectHelper.FromIntPtr (calldata) as MouseButtonEventArgs ?? new MouseButtonEventArgs (calldata)));

		static UnmanagedEventHandler on_mouse_left_button_up = Events.SafeDispatcher (
			    (IntPtr target, IntPtr calldata, IntPtr closure) =>
			    	((Control) NativeDependencyObjectHelper.FromIntPtr (closure)).OnMouseLeftButtonUp (NativeDependencyObjectHelper.FromIntPtr (calldata) as MouseButtonEventArgs ??new MouseButtonEventArgs (calldata)));

#if NET_3_0
		static UnmanagedEventHandler on_mouse_right_button_down = Events.SafeDispatcher (
			    (IntPtr target, IntPtr calldata, IntPtr closure) =>
			    	((Control) NativeDependencyObjectHelper.FromIntPtr (closure)).OnMouseRightButtonDown (NativeDependencyObjectHelper.FromIntPtr (calldata) as MouseButtonEventArgs ?? new MouseButtonEventArgs (calldata)));

		static UnmanagedEventHandler on_mouse_right_button_up = Events.SafeDispatcher (
			    (IntPtr target, IntPtr calldata, IntPtr closure) =>
			    	((Control) NativeDependencyObjectHelper.FromIntPtr (closure)).OnMouseRightButtonUp (NativeDependencyObjectHelper.FromIntPtr (calldata) as MouseButtonEventArgs ?? new MouseButtonEventArgs (calldata)));

		static UnmanagedEventHandler on_mouse_wheel = Events.SafeDispatcher (
			    (IntPtr target, IntPtr calldata, IntPtr closure) =>
			    	((Control) NativeDependencyObjectHelper.FromIntPtr (closure)).OnMouseWheel (NativeDependencyObjectHelper.FromIntPtr (calldata) as MouseWheelEventArgs ?? new MouseWheelEventArgs (calldata)));
#endif

		static UnmanagedEventHandler on_isenabledproperty_changed = Events.SafeDispatcher (
			    (IntPtr target, IntPtr calldata, IntPtr closure) =>
				((Control) NativeDependencyObjectHelper.FromIntPtr (closure)).InvokeIsEnabledPropertyChanged ());

		internal virtual void Initialize ()
		{
			// FIXME these two events should not be handled using Events.AddHandler, since those handlers are removable via the plugin

			// hook up the TemplateApplied callback so we
			// can notify controls when their template has
			// been instantiated as a visual tree.
			Events.AddHandler (this, EventIds.Control_TemplateAppliedEvent, template_applied);

			// this needs to be handled like the OnEventHandlers below, where it's called before the event
			// is raised.  the eventargs is a problem, though, since OnEventHandlers need to pass the native handle
			// down to the unmanaged layer which then emits the event (using that native handle as the eventargs
			// parameter).
			Events.AddHandler (this, EventIds.Control_IsEnabledChangedEvent, on_isenabledproperty_changed);

			Events.AddOnEventHandler (this, EventIds.UIElement_GotFocusEvent, on_got_focus);
			Events.AddOnEventHandler (this, EventIds.UIElement_LostFocusEvent, on_lost_focus);
			Events.AddOnEventHandler (this, EventIds.UIElement_KeyDownEvent, on_key_down);
			Events.AddOnEventHandler (this, EventIds.UIElement_KeyUpEvent, on_key_up);
			Events.AddOnEventHandler (this, EventIds.UIElement_MouseEnterEvent, on_mouse_enter);
			Events.AddOnEventHandler (this, EventIds.UIElement_MouseLeaveEvent, on_mouse_leave);
			Events.AddOnEventHandler (this, EventIds.UIElement_MouseMoveEvent, on_mouse_move);
			Events.AddOnEventHandler (this, EventIds.UIElement_MouseLeftButtonDownEvent, on_mouse_left_button_down);
			Events.AddOnEventHandler (this, EventIds.UIElement_MouseLeftButtonUpEvent, on_mouse_left_button_up);
#if NET_3_0
			Events.AddOnEventHandler (this, EventIds.UIElement_MouseRightButtonDownEvent, on_mouse_right_button_down);
			Events.AddOnEventHandler (this, EventIds.UIElement_MouseRightButtonUpEvent, on_mouse_right_button_up);
			Events.AddOnEventHandler (this, EventIds.UIElement_MouseWheelEvent, on_mouse_wheel);
#endif
		}

		private static Type ControlType = typeof (Control);
		private static Type UserControlType = typeof (UserControl);
		
		protected object DefaultStyleKey {
			get { return (object) GetValue (DefaultStyleKeyProperty); }
			set {
				Type t = value as Type;
				// feels weird but that's unit tested as such
				if (t == null || (t == ControlType)  || !t.IsSubclassOf (ControlType)
				    || (t == UserControlType) || (t.IsSubclassOf (UserControlType)))
					throw new ArgumentException ("DefaultStyleKey");

				if (this.GetType() == UserControlType || this.GetType().IsSubclassOf (UserControlType))
					throw new InvalidOperationException ("UserControls do not participate in templating, so setting the DefaultStyleKey is not allowed");

				SetValue (DefaultStyleKeyProperty, value);
			}
		}

		protected static readonly System.Windows.DependencyProperty DefaultStyleKeyProperty = 
			DependencyProperty.Lookup (Kind.CONTROL,
						   "DefaultStyleKey",
						   typeof (object));

		public bool ApplyTemplate()
		{
			return NativeMethods.control_apply_template (native);
		}
		
		public bool Focus()
		{
			return NativeMethods.uielement_focus (native, true);
		}

		protected DependencyObject GetTemplateChild (string childName)
		{
			if (childName == null)
				throw new ArgumentException ("childName");

			return NativeDependencyObjectHelper.FromIntPtr (NativeMethods.control_get_template_child (native, childName)) as DependencyObject;
		}

		internal virtual void InvokeIsEnabledPropertyChanged ()
		{
		}

		protected virtual void OnGotFocus (RoutedEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");

			NativeMethods.event_object_do_emit_current_context (native, EventIds.UIElement_GotFocusEvent, e.NativeHandle, false, -1);
		}

		protected virtual void OnLostFocus (RoutedEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");

			NativeMethods.event_object_do_emit_current_context (native, EventIds.UIElement_LostFocusEvent, e.NativeHandle, false, -1);
		}

		protected virtual void OnKeyDown (KeyEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");

			NativeMethods.event_object_do_emit_current_context (native, EventIds.UIElement_KeyDownEvent, e.NativeHandle, false, -1);

			if (!e.Handled && e.Key == Key.Tab) {
				// If the tab key is not handled by Control.OnKeyDown or by an eventhandler attached to the KeyDown event,
				// we handle it and tab to the next control here.
				e.Handled = true;
				NativeMethods.tab_navigation_walker_focus (native, (Keyboard.Modifiers & ModifierKeys.Shift) == ModifierKeys.None);
			}
		}

		protected virtual void OnKeyUp (KeyEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");

			NativeMethods.event_object_do_emit_current_context (native, EventIds.UIElement_KeyUpEvent, e.NativeHandle, false, -1);
		}

		protected virtual void OnMouseEnter (MouseEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");

			NativeMethods.event_object_do_emit_current_context (native, EventIds.UIElement_MouseEnterEvent, e.NativeHandle, false, -1);
		}

		protected virtual void OnMouseLeave (MouseEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");

			NativeMethods.event_object_do_emit_current_context (native, EventIds.UIElement_MouseLeaveEvent, e.NativeHandle, false, -1);
		}

		protected virtual void OnMouseLeftButtonDown (MouseButtonEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");

			NativeMethods.event_object_do_emit_current_context (native, EventIds.UIElement_MouseLeftButtonDownEvent, e.NativeHandle, false, -1);
		}

		protected virtual void OnMouseLeftButtonUp (MouseButtonEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");

			NativeMethods.event_object_do_emit_current_context (native, EventIds.UIElement_MouseLeftButtonUpEvent, e.NativeHandle, false, -1);
		}

#if NET_3_0
		protected virtual void OnMouseRightButtonDown (MouseButtonEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");

			NativeMethods.event_object_do_emit_current_context (native, EventIds.UIElement_MouseRightButtonDownEvent, e.NativeHandle, false, -1);
		}

		protected virtual void OnMouseRightButtonUp (MouseButtonEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");

			NativeMethods.event_object_do_emit_current_context (native, EventIds.UIElement_MouseRightButtonUpEvent, e.NativeHandle, false, -1);
		}

		protected virtual void OnMouseWheel (MouseWheelEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");

			NativeMethods.event_object_do_emit_current_context (native, EventIds.UIElement_MouseWheelEvent, e.NativeHandle, false, -1);
		}
#endif

		protected virtual void OnMouseMove (MouseEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");

			NativeMethods.event_object_do_emit_current_context (native, EventIds.UIElement_MouseMoveEvent, e.NativeHandle, false, -1);
		}
	}
}
