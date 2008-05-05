//
// FrameworkElement.cs
//
// Author:
//   Miguel de Icaza (miguel@novell.com)
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
using Mono;

namespace System.Windows {
	public abstract class FrameworkElement : UIElement {
		
		static FrameworkElement ()
		{
			WidthProperty = DependencyProperty.Lookup (Kind.FRAMEWORKELEMENT, "Width", typeof (double));
			HeightProperty = DependencyProperty.Lookup (Kind.FRAMEWORKELEMENT, "Height", typeof (double));
		}
		
		public FrameworkElement () : base (NativeMethods.framework_element_new ())
		{
		}
		
		internal FrameworkElement (IntPtr raw) : base (raw)
		{
		}
			
		public double Height {
			get {
				return (double) GetValue (HeightProperty);
			}

			set {
				SetValue (HeightProperty, value);
			}
		}

		public object Parent {
			get {
				IntPtr parent_handle = NativeMethods.uielement_get_parent (native);
				if (parent_handle == IntPtr.Zero)
					return null;

				Kind k = NativeMethods.dependency_object_get_object_type (parent_handle);
				return DependencyObject.Lookup (k, parent_handle);
			}
		}

		public double Width {
			get {
				return (double) GetValue (WidthProperty);
		}

		set {
				SetValue (WidthProperty, value);
			}
		}

		public static readonly DependencyProperty WidthProperty;
		public static readonly DependencyProperty HeightProperty;
		
		internal override Kind GetKind ()
		{
			return Kind.FRAMEWORKELEMENT;
		}
	}
}
