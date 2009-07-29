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

		static Control ()
		{
			IsEnabledProperty.AddPropertyChangeCallback (OnIsEnabledPropertyChanged);
		}
		
		private void Initialize ()
		{
			// hook up the TemplateApplied callback so we
			// can notify controls when their template has
			// been instantiated as a visual tree.
			Events.AddHandler (this, "TemplateApplied", Events.template_applied);

			// regiser handlers so our Invoke methods will get hit
			GotFocus += delegate {};
			LostFocus += delegate {};

			KeyDown += delegate {};
			KeyUp += delegate {};

			MouseEnter += delegate {};
			MouseLeave += delegate {};

			MouseLeftButtonDown += delegate {};
			MouseLeftButtonUp += delegate {};

			MouseMove += delegate {};
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

		private static void OnIsEnabledPropertyChanged (DependencyObject d, DependencyPropertyChangedEventArgs e) 
		{
			Control c = (d as Control);

			DependencyPropertyChangedEventHandler handler = (DependencyPropertyChangedEventHandler) c.EventList [IsEnabledEvent];
			if (handler != null)
				handler (d, e);
		}
		
		static object IsEnabledEvent = new object ();

		public event DependencyPropertyChangedEventHandler IsEnabledChanged {
			add { EventList.AddHandler (IsEnabledEvent, value); }
			remove { EventList.RemoveHandler (IsEnabledEvent, value); }
		}

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

		internal override void InvokeGotFocus (RoutedEventArgs e)
		{
			OnGotFocus (e);
			base.InvokeGotFocus (e);
		}

		// called before the event
		protected virtual void OnGotFocus (RoutedEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");
		}

		internal override void InvokeLostFocus (RoutedEventArgs e)
		{
			OnLostFocus (e);
			base.InvokeLostFocus (e);
		}

		// called before the event
		protected virtual void OnLostFocus (RoutedEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");
		}

		internal override void InvokeKeyDown (KeyEventArgs k)
		{
			if (!k.Handled)
				OnKeyDown (k);
			if (!k.Handled)
				base.InvokeKeyDown (k);
			if (!k.Handled && k.Key == Key.Tab) {
				// If the tab key is not handled by Control.OnKeyDown or by an eventhandler attached to the KeyDown event,
				// we handle it and tab to the next control here.
				k.Handled = true;
				Mono.NativeMethods.tab_navigation_walker_focus (native, (Keyboard.Modifiers & ModifierKeys.Shift) == ModifierKeys.None);
			}
		}

		// called before the event
		protected virtual void OnKeyDown (KeyEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");
		}

		internal override void InvokeKeyUp (KeyEventArgs e)
		{
			OnKeyUp (e);
			if (!e.Handled)
				base.InvokeKeyUp (e);
		}

		// called before the event
		protected virtual void OnKeyUp (KeyEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");
		}

		internal override void InvokeMouseEnter (MouseEventArgs e)
		{
			OnMouseEnter (e);
			base.InvokeMouseEnter (e);
		}

		// called before the event
		protected virtual void OnMouseEnter (MouseEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");
		}

		internal override void InvokeMouseLeave (MouseEventArgs e)
		{
			OnMouseLeave (e);
			base.InvokeMouseLeave (e);
		}

		// called before the event
		protected virtual void OnMouseLeave (MouseEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");
		}

		internal override void InvokeMouseLeftButtonDown (MouseButtonEventArgs e)
		{
			OnMouseLeftButtonDown (e);
			if (!e.Handled)
				base.InvokeMouseLeftButtonDown (e);
		}

		// called before the event
		protected virtual void OnMouseLeftButtonDown (MouseButtonEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");
		}

		internal override void InvokeMouseLeftButtonUp (MouseButtonEventArgs e)
		{
			OnMouseLeftButtonUp (e);
			if (!e.Handled)
				base.InvokeMouseLeftButtonUp (e);
		}

		// called before the event
		protected virtual void OnMouseLeftButtonUp (MouseButtonEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");
		}

#if NET_3_0
		internal override void InvokeMouseRightButtonDown (MouseButtonEventArgs e)
		{
			OnMouseRightButtonDown (e);
			if (!e.Handled)
				base.InvokeMouseRightButtonDown (e);
		}

		// called before the event
		protected virtual void OnMouseRightButtonDown (MouseButtonEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");
		}

		internal override void InvokeMouseRightButtonUp (MouseButtonEventArgs e)
		{
			OnMouseRightButtonUp (e);
			if (!e.Handled)
				base.InvokeMouseRightButtonUp (e);
		}

		// called before the event
		protected virtual void OnMouseRightButtonUp (MouseButtonEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");
		}

		internal override void InvokeMouseWheel (MouseWheelEventArgs e)
		{
			OnMouseWheel (e);
			if (!e.Handled)
				base.InvokeMouseWheel (e);
		}

		// called before the event
		protected virtual void OnMouseWheel (MouseWheelEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");
		}
#endif

		internal override void InvokeMouseMove (MouseEventArgs e)
		{
			OnMouseMove (e);
			base.InvokeMouseMove (e);
		}

		// called before the event
		protected virtual void OnMouseMove (MouseEventArgs e)
		{
			if (e == null)
				throw new ArgumentNullException ("e");
		}
	}
}
