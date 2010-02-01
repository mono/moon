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
using System.Windows.Automation;
using System.Windows.Automation.Peers;

namespace System.Windows.Controls {
	public abstract partial class Control : FrameworkElement {
		static UnmanagedEventHandler on_got_focus = Events.SafeDispatcher (
			   (IntPtr target, IntPtr calldata, IntPtr closure) => {
				   Control control = (Control) NativeDependencyObjectHelper.FromIntPtr (closure);
				   RoutedEventArgs args = NativeDependencyObjectHelper.FromIntPtr (calldata) as RoutedEventArgs ?? new RoutedEventArgs (calldata, false);
				   control.PreOnGotFocus(args);
				   control.OnGotFocus (args);
				   control.PostOnGotFocus (args);
			   });

		static UnmanagedEventHandler on_lost_focus = Events.SafeDispatcher (
			   (IntPtr target, IntPtr calldata, IntPtr closure) => {
				   Control control = (Control) NativeDependencyObjectHelper.FromIntPtr (closure);
				   RoutedEventArgs args = NativeDependencyObjectHelper.FromIntPtr (calldata) as RoutedEventArgs ?? new RoutedEventArgs (calldata, false);
				   control.PreOnLostFocus(args);
				   control.OnLostFocus (args);
				   control.PostOnLostFocus (args);
			   });

		static UnmanagedEventHandler on_lost_mouse_capture = Events.SafeDispatcher (
			   (IntPtr target, IntPtr calldata, IntPtr closure) => {
				   Control control = (Control) NativeDependencyObjectHelper.FromIntPtr (closure);
				   MouseEventArgs args = NativeDependencyObjectHelper.FromIntPtr (calldata) as MouseEventArgs ?? new MouseEventArgs (calldata);
				   control.PreOnLostMouseCapture(args);
				   control.OnLostMouseCapture (args);
				   control.PostOnLostMouseCapture (args);
			   });

		static UnmanagedEventHandler on_key_down = Events.SafeDispatcher (
			  (IntPtr target, IntPtr calldata, IntPtr closure) => {
				  KeyEventArgs args = NativeDependencyObjectHelper.FromIntPtr (calldata) as KeyEventArgs ?? new KeyEventArgs (calldata);
				  Control control = (Control) NativeDependencyObjectHelper.FromIntPtr (closure);
				  control.PreOnKeyDown (args);
				  if (!args.Handled)
					  control.OnKeyDown (args);
				  if (!args.Handled)
					  control.PostOnKeyDown (args);
			  });

		static UnmanagedEventHandler on_key_up = Events.SafeDispatcher (
			  (IntPtr target, IntPtr calldata, IntPtr closure) => {
				  KeyEventArgs args = NativeDependencyObjectHelper.FromIntPtr (calldata) as KeyEventArgs ?? new KeyEventArgs (calldata);
				  Control control = (Control) NativeDependencyObjectHelper.FromIntPtr (closure);
				  control.PreOnKeyUp (args);
				  if (!args.Handled)
					  control.OnKeyUp (args);
				  if (!args.Handled)
					  control.PostOnKeyUp (args);
			  });

		static UnmanagedEventHandler on_mouse_enter = Events.SafeDispatcher (
			   (IntPtr target, IntPtr calldata, IntPtr closure) => {
				   Control control = (Control) NativeDependencyObjectHelper.FromIntPtr (closure);
				   MouseEventArgs args = NativeDependencyObjectHelper.FromIntPtr (calldata) as MouseEventArgs ?? new MouseEventArgs (calldata);
				   control.PreOnMouseEnter (args);
				   control.OnMouseEnter (args);
				   control.PostOnMouseEnter (args);
			   });

		static UnmanagedEventHandler on_mouse_leave = Events.SafeDispatcher (
			   (IntPtr target, IntPtr calldata, IntPtr closure) => {
				   Control control = (Control) NativeDependencyObjectHelper.FromIntPtr (closure);
				   MouseEventArgs args = NativeDependencyObjectHelper.FromIntPtr (calldata) as MouseEventArgs ?? new MouseEventArgs (calldata);
				   control.PreOnMouseLeave (args);
				   control.OnMouseLeave (args);
				   control.PostOnMouseLeave (args);
			   });

		static UnmanagedEventHandler on_mouse_move = Events.SafeDispatcher (
			   (IntPtr target, IntPtr calldata, IntPtr closure) => {
				   Control control = (Control) NativeDependencyObjectHelper.FromIntPtr (closure);
				   MouseEventArgs args = NativeDependencyObjectHelper.FromIntPtr (calldata) as MouseEventArgs ?? new MouseEventArgs (calldata);
				   control.PreOnMouseMove (args);
				   control.OnMouseMove (args);
				   control.PostOnMouseMove (args);
			   });

		static UnmanagedEventHandler on_mouse_left_button_down = Events.SafeDispatcher (
			  (IntPtr target, IntPtr calldata, IntPtr closure) => {
				  MouseButtonEventArgs args = NativeDependencyObjectHelper.FromIntPtr (calldata) as MouseButtonEventArgs ?? new MouseButtonEventArgs (calldata);
				  Control control = (Control) NativeDependencyObjectHelper.FromIntPtr (closure);
				  control.PreOnMouseLeftButtonDown (args);
				  if (!args.Handled)
					  control.OnMouseLeftButtonDown (args);
				  if (!args.Handled)
					  control.PostOnMouseLeftButtonDown (args);
			  });

		static UnmanagedEventHandler on_mouse_left_button_up = Events.SafeDispatcher (
			  (IntPtr target, IntPtr calldata, IntPtr closure) => {
				  MouseButtonEventArgs args = NativeDependencyObjectHelper.FromIntPtr (calldata) as MouseButtonEventArgs ?? new MouseButtonEventArgs (calldata);
				  Control control = (Control) NativeDependencyObjectHelper.FromIntPtr (closure);
				  control.PreOnMouseLeftButtonUp (args);
				  if (!args.Handled)
					  control.OnMouseLeftButtonUp (args);
				  if (!args.Handled)
					  control.PostOnMouseLeftButtonUp (args);
			  });

#if NET_3_0
		static UnmanagedEventHandler on_mouse_right_button_down = Events.SafeDispatcher (
			  (IntPtr target, IntPtr calldata, IntPtr closure) => {
				  MouseButtonEventArgs args = NativeDependencyObjectHelper.FromIntPtr (calldata) as MouseButtonEventArgs ?? new MouseButtonEventArgs (calldata);
				  Control control = (Control) NativeDependencyObjectHelper.FromIntPtr (closure);
				  control.PreOnMouseRightButtonDown (args);
				  if (!args.Handled)
					  control.OnMouseRightButtonDown (args);
				  if (!args.Handled)
					  control.PostOnMouseRightButtonDown (args);
			  });

		static UnmanagedEventHandler on_mouse_right_button_up = Events.SafeDispatcher (
			  (IntPtr target, IntPtr calldata, IntPtr closure) => {
				  MouseButtonEventArgs args = NativeDependencyObjectHelper.FromIntPtr (calldata) as MouseButtonEventArgs ?? new MouseButtonEventArgs (calldata);
				  Control control = (Control) NativeDependencyObjectHelper.FromIntPtr (closure);
				  control.PreOnMouseRightButtonUp (args);
				  if (!args.Handled)
					  control.OnMouseRightButtonUp (args);
				  if (!args.Handled)
					  control.PostOnMouseRightButtonUp (args);
			  });
#endif

		static UnmanagedEventHandler on_mouse_wheel = Events.SafeDispatcher (
			  (IntPtr target, IntPtr calldata, IntPtr closure) => {
				  MouseWheelEventArgs args = NativeDependencyObjectHelper.FromIntPtr (calldata) as MouseWheelEventArgs ?? new MouseWheelEventArgs (calldata);
				  Control control = (Control) NativeDependencyObjectHelper.FromIntPtr (closure);
				  control.PreOnMouseWheel (args);
				  if (!args.Handled)
					  control.OnMouseWheel (args);
				  if (!args.Handled)
					  control.PostOnMouseWheel (args);
			  });

		static UnmanagedEventHandler on_isenabledproperty_changed = Events.SafeDispatcher (
			    (IntPtr target, IntPtr calldata, IntPtr closure) =>
				((Control) NativeDependencyObjectHelper.FromIntPtr (closure)).InvokeIsEnabledPropertyChanged ());

		static Control ()
		{
			IsTabStopProperty.AddPropertyChangeCallback (IsTabStopPropertyChanged);
		}

		static void IsTabStopPropertyChanged (DependencyObject sender, DependencyPropertyChangedEventArgs args)
		{
			((Control) sender).RaiseUIAIsTabStopChanged (args);
		}

		void Initialize ()
		{
			// FIXME this should not be handled using Events.AddHandler, since those handlers are removable via the plugin

			// this needs to be handled like the OnEventHandlers below, where it's called before the event
			// is raised.  the eventargs is a problem, though, since OnEventHandlers need to pass the native handle
			// down to the unmanaged layer which then emits the event (using that native handle as the eventargs
			// parameter).
			Events.AddHandler (this, EventIds.Control_IsEnabledChangedEvent, on_isenabledproperty_changed);

			Events.AddOnEventHandler (this, EventIds.UIElement_GotFocusEvent, on_got_focus);
			Events.AddOnEventHandler (this, EventIds.UIElement_LostFocusEvent, on_lost_focus);
			Events.AddOnEventHandler (this, EventIds.UIElement_LostMouseCaptureEvent, on_lost_mouse_capture);
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
#endif
			Events.AddOnEventHandler (this, EventIds.UIElement_MouseWheelEvent, on_mouse_wheel);
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
			return NativeMethods.framework_element_apply_template (native);
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

		// centralize call to pinvoke to reduce [SecuritySafeCritical] methods
		private void EmitCurrentContext (int id, RoutedEventArgs e)
		{
			NativeMethods.event_object_do_emit_current_context (native, id, e.NativeHandle);
		}

		internal virtual void PreOnGotFocus (RoutedEventArgs e) { }
		protected virtual void OnGotFocus (RoutedEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");
		}
		internal virtual void PostOnGotFocus (RoutedEventArgs e)
		{
			EmitCurrentContext (EventIds.UIElement_GotFocusEvent, e);

			if (AutomationPeer != null)
				AutomationPeer.RaisePropertyChangedEvent (AutomationElementIdentifiers.HasKeyboardFocusProperty, 
					                                  false,
									  true);
		}

		internal virtual void PreOnLostFocus (RoutedEventArgs e) { }
		protected virtual void OnLostFocus (RoutedEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");
		}
		internal virtual void PostOnLostFocus (RoutedEventArgs e)
		{
			EmitCurrentContext (EventIds.UIElement_LostFocusEvent, e);

			if (AutomationPeer != null)
				AutomationPeer.RaisePropertyChangedEvent (AutomationElementIdentifiers.HasKeyboardFocusProperty, 
					                                  true,
									  false);
		}

		internal virtual void PreOnLostMouseCapture (MouseEventArgs e) { }
		protected virtual void OnLostMouseCapture (MouseEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");
		}
		internal virtual void PostOnLostMouseCapture (RoutedEventArgs e)
		{
			EmitCurrentContext (EventIds.UIElement_LostMouseCaptureEvent, e);
		}


		internal virtual void PreOnKeyDown (KeyEventArgs e) { }
		protected virtual void OnKeyDown (KeyEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");
		}
		internal virtual void PostOnKeyDown (KeyEventArgs e)
		{
			EmitCurrentContext (EventIds.UIElement_KeyDownEvent, e);

			if (!e.Handled && e.Key == Key.Tab) {
				// If the tab key is not handled by Control.OnKeyDown or by an eventhandler attached to the KeyDown event,
				// we handle it and tab to the next control here.
				e.Handled = true;
				NativeMethods.tab_navigation_walker_focus (native, (Keyboard.Modifiers & ModifierKeys.Shift) == ModifierKeys.None);
			}
		}

		internal virtual void PreOnKeyUp (KeyEventArgs e) { }
		protected virtual void OnKeyUp (KeyEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");
		}
		internal virtual void PostOnKeyUp (KeyEventArgs e)
		{
			EmitCurrentContext (EventIds.UIElement_KeyUpEvent, e);
		}

		internal virtual void PreOnMouseEnter (MouseEventArgs e) { }
		protected virtual void OnMouseEnter (MouseEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");
		}
		internal virtual void PostOnMouseEnter (MouseEventArgs e)
		{
			EmitCurrentContext (EventIds.UIElement_MouseEnterEvent, e);
		}

		internal virtual void PreOnMouseLeave (MouseEventArgs e) { }
		protected virtual void OnMouseLeave (MouseEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");
		}
		internal virtual void PostOnMouseLeave (MouseEventArgs e)
		{
			EmitCurrentContext (EventIds.UIElement_MouseLeaveEvent, e);
		}

		internal virtual void PreOnMouseLeftButtonDown (MouseButtonEventArgs e) { }
		protected virtual void OnMouseLeftButtonDown (MouseButtonEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");
		}
		internal virtual void PostOnMouseLeftButtonDown (MouseButtonEventArgs e)
		{
			EmitCurrentContext (EventIds.UIElement_MouseLeftButtonDownEvent, e);
		}

		internal virtual void PreOnMouseLeftButtonUp (MouseButtonEventArgs e) { }
		protected virtual void OnMouseLeftButtonUp (MouseButtonEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");
		}
		internal virtual void PostOnMouseLeftButtonUp (MouseButtonEventArgs e)
		{
			EmitCurrentContext (EventIds.UIElement_MouseLeftButtonUpEvent, e);
		}

#if NET_3_0
		internal virtual void PreOnMouseRightButtonDown (MouseButtonEventArgs e) { }
		protected virtual void OnMouseRightButtonDown (MouseButtonEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");
		}
		internal virtual void PostOnMouseRightButtonDown (MouseButtonEventArgs e)
		{
			EmitCurrentContext (EventIds.UIElement_MouseRightButtonDownEvent, e);
		}

		internal virtual void PreOnMouseRightButtonUp (MouseButtonEventArgs e) { }
		protected virtual void OnMouseRightButtonUp (MouseButtonEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");
		}
		internal virtual void PostOnMouseRightButtonUp (MouseButtonEventArgs e)
		{
			EmitCurrentContext (EventIds.UIElement_MouseRightButtonUpEvent, e);
		}
#endif

		internal virtual void PreOnMouseWheel (MouseWheelEventArgs e) { }
		protected virtual void OnMouseWheel (MouseWheelEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");
		}
		internal virtual void PostOnMouseWheel (MouseWheelEventArgs e)
		{
			EmitCurrentContext (EventIds.UIElement_MouseWheelEvent, e);
		}

		internal virtual void PreOnMouseMove (MouseEventArgs e) { }
		protected virtual void OnMouseMove (MouseEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");
		}
		internal virtual void PostOnMouseMove (MouseEventArgs e)
		{
			EmitCurrentContext (EventIds.UIElement_MouseMoveEvent, e);
		}
	
		#region UIA Events

		internal event DependencyPropertyChangedEventHandler UIAIsTabStopChanged;

		internal void RaiseUIAIsTabStopChanged (DependencyPropertyChangedEventArgs args)
		{
			if (UIAIsTabStopChanged != null)
				UIAIsTabStopChanged (this, args);
		}

		#endregion
	}
}
