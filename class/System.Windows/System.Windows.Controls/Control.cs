//
// System.Windows.Controls.Control
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2007 Novell, Inc (http://www.novell.com)
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
using System.Security;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Markup;

namespace System.Windows.Controls {
	public abstract partial class Control : FrameworkElement {

		private static Type ControlType = typeof (Control);
		
		protected object DefaultStyleKey {
			get { return (object) GetValue (DefaultStyleKeyProperty); }
			set {
				// feels weird but that's unit tested as such
				if (value == null)
					throw new ArgumentException ("DefaultStyleKey");
				SetValue (DefaultStyleKeyProperty, value);
			}
		}

		protected static readonly System.Windows.DependencyProperty DefaultStyleKeyProperty = 
			DependencyProperty.Register (
				"DefaultStyleKey",
				typeof (object),
				typeof (Control),
				new PropertyMetadata (OnDefaultStyleKeyPropertyChanged)
			);

		private static void OnDefaultStyleKeyPropertyChanged (DependencyObject d, DependencyPropertyChangedEventArgs e) 
		{
			if (e.NewValue == null)
				return;
			// expected to be a Type
			Type nv = (e.NewValue as Type);
			if ((nv == null) || (nv == ControlType) || !nv.IsSubclassOf (ControlType))
				throw new ArgumentException ("DefaultStyleKey");
		}

		public bool IsEnabled {
			get { return (bool) GetValue (IsEnabledProperty); }
			set { SetValue (IsEnabledProperty, value); }
		}

		public static readonly DependencyProperty IsEnabledProperty = DependencyProperty.Register (
			"IsEnabled",
			typeof (bool),
			typeof (Control),
			new PropertyMetadata (true, OnIsEnabledPropertyChanged));

		private static void OnIsEnabledPropertyChanged (DependencyObject d, DependencyPropertyChangedEventArgs e) 
		{
			Control c = (d as Control);
			DependencyPropertyChangedEventHandler handler = c.IsEnabledChanged;
			if (handler != null)
				handler (d, e);

			c.OnIsEnabledChanged ((bool) e.NewValue);
		}
		
		public event DependencyPropertyChangedEventHandler IsEnabledChanged;


		// moved from ContentControl, needed to satisfy the beta1 controls
		// FIXME 1: needs to be internalized (corcompare) everywhere (including all controls)
		// FIXME 2: remove/update using the new event once we get the final controls 
		protected virtual void OnIsEnabledChanged (bool isEnabled)
		{
		}

		public bool ApplyTemplate()
		{
			return NativeMethods.control_apply_template (native);
		}
		
		public bool Focus()
		{
			return NativeMethods.surface_focus_element (Application.s_surface, native);
		}

		[MonoTODO]
		protected DependencyObject GetTemplateChild (string childName)
		{
			if (childName == null)
				throw new ArgumentException ("childName");

			// null is returned for unknown names (e.g. String.Empty) or when no templates are present
			// FIXME: return something when that something is present
			return null;
		}

		[MonoTODO]
		protected virtual void OnGotFocus (RoutedEventArgs e)
		{
		}

		[MonoTODO]
		protected virtual void OnLostFocus (RoutedEventArgs e)
		{
		}

		[MonoTODO]
		protected virtual void OnKeyDown (KeyEventArgs e)
		{
		}

		[MonoTODO]
		protected virtual void OnKeyUp (KeyEventArgs e)
		{
		}

		[MonoTODO]
		protected virtual void OnMouseEnter (MouseEventArgs e)
		{
		}

		[MonoTODO]
		protected virtual void OnMouseLeave (MouseEventArgs e)
		{
		}

		[MonoTODO]
		protected virtual void OnMouseLeftButtonDown (MouseButtonEventArgs e)
		{
		}

		[MonoTODO]
		protected virtual void OnMouseLeftButtonUp (MouseButtonEventArgs e)
		{
		}

		[MonoTODO]
		protected virtual void OnMouseMove (MouseEventArgs e)
		{
		}
	}
}
