//
// ColumnDefinition.cs
//
// Authors:
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
using System.Windows;
using Mono;

namespace System.Windows.Controls {

	public class ColumnDefinition : DependencyObject {
		public static readonly DependencyProperty WidthProperty;
		public static readonly DependencyProperty MaxWidthProperty;
		public static readonly DependencyProperty MinWidthProperty;

		static ColumnDefinition ()
		{
			WidthProperty = DependencyProperty.Lookup (Kind.GRID, "WidthProperty", typeof (GridLength));
			MaxWidthProperty = DependencyProperty.Lookup (Kind.GRID, "MaxWidth", typeof (double));
			MinWidthProperty = DependencyProperty.Lookup (Kind.GRID, "MinWidth", typeof (double));
		}

		public ColumnDefinition () : base (NativeMethods.column_definition_new ())
		{
		}

		internal ColumnDefinition (IntPtr raw) : base (raw)
		{
		}

		internal override Kind GetKind ()
		{
			return Kind.COLUMNDEFINITION;
		}

		public double ActualWidth {
			get {
				return NativeMethods.column_definition_get_actual_width (native);
			}
		}

		public GridLength Width {
			get {
				return (GridLength) GetValue (WidthProperty);
			}

			set {
				SetValue (WidthProperty, value);
			}
		}

		public double MaxWidth {
			get {
				return (double) GetValue (MaxWidthProperty);
			}

			set {
				SetValue (MaxWidthProperty, value);
			}
		}

		public double MinWidth {
			get {
				return (double) GetValue (MinWidthProperty);
			}

			set {
				SetValue (MinWidthProperty, value);
			}
		}
		

	}
}
