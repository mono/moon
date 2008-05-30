//
// Grid.cs
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

	public class Grid : Panel {
		public static readonly DependencyProperty ColumnProperty;
		public static readonly DependencyProperty ColumnSpanProperty;
		public static readonly DependencyProperty RowProperty;
		public static readonly DependencyProperty RowSpanProperty;
		public static readonly DependencyProperty ShowGridLinesProperty;

		//
		// These are not exposed to the public but they do exist
		//
		public static readonly DependencyProperty ColumnDefinitionsProperty;
		public static readonly DependencyProperty RowDefinitionsProperty;

		static Grid ()
		{
			ColumnProperty = DependencyProperty.Lookup (Kind.GRID, "Column", typeof (int));
			RowProperty = DependencyProperty.Lookup (Kind.GRID, "Row", typeof (int));
			ColumnSpanProperty = DependencyProperty.Lookup (Kind.GRID, "ColumnSpan", typeof (int));
			RowSpanProperty = DependencyProperty.Lookup (Kind.GRID, "RowSpan", typeof (int));

			//
			// These seem internal, no public field, but they are available to the user
			//
			ColumnDefinitionsProperty = DependencyProperty.Lookup (Kind.GRID, "ColumnDefinitions", typeof(ColumnDefinitionCollection));
			RowDefinitionsProperty    = DependencyProperty.Lookup (Kind.GRID, "RowDefinitions", typeof(RowDefinitionCollection));
		}

		public Grid () : base (NativeMethods.grid_new ())
		{
		}

		internal Grid (IntPtr raw) : base (raw)
		{
		}

		internal override Kind GetKind ()
		{
			return Kind.GRID;
		}

		public static int GetColumn (FrameworkElement element)
		{
			return (int) element.GetValue (ColumnProperty);
		}

		public static int GetColumnSpan (FrameworkElement element)
		{
			return (int) element.GetValue (ColumnSpanProperty);
		}

		public static int GetRow (FrameworkElement element)
		{
			return (int) element.GetValue (RowProperty);
		}

		public static int GetRowSpan (FrameworkElement element)
		{
			return (int) element.GetValue (RowSpanProperty);
		}

		public static void SetColumn (FrameworkElement element, int value)
		{
			element.SetValue (ColumnProperty, value);
		}

		public static void SetColumnSpan (FrameworkElement element, int value)
		{
			element.SetValue (ColumnSpanProperty, value);
		}

		public static void SetRow (FrameworkElement element, int value)
		{
			element.SetValue (RowProperty, value);
		}

		public static void SetRowSpan (FrameworkElement element, int value)
		{
			element.SetValue (RowSpanProperty, value);
		}

		public bool ShowGridLines {
			get {
				return (bool) GetValue (ShowGridLinesProperty);
			}

			set {
				SetValue (ShowGridLinesProperty, value);
			}
		}

		public ColumnDefinitionCollection ColumnDefinitions {
			get {
				return (ColumnDefinitionCollection) GetValue (ColumnDefinitionsProperty);
			}
		}

		public RowDefinitionCollection RowDefinitions {
			get {
				return (RowDefinitionCollection) GetValue (RowDefinitionsProperty);
			}
		}
	}
}
