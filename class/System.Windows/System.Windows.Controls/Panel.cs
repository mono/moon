//
// Panel.cs
//
// Author:
//   Miguel de Icaza (miguel@novell.com)
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
using System.Windows.Media;
using System.Windows;
using MS.Internal;
using Mono;

namespace System.Windows.Controls {
	public abstract class Panel : FrameworkElement {
		public static readonly DependencyProperty ChildrenProperty;
		public static readonly DependencyProperty BackgroundProperty;

		static Panel ()
		{
			ChildrenProperty = DependencyProperty.Lookup (Kind.PANEL, "Children", typeof (VisualCollection));
			BackgroundProperty = DependencyProperty.Lookup (Kind.PANEL, "Background", typeof (Brush));
		}
		
		public Panel () : base (NativeMethods.panel_new ())
		{
		}
		
		internal Panel (IntPtr raw) : base (raw)
		{
		}

		public Brush Background {
			get {
				return (Brush) GetValue (BackgroundProperty);
			}
			
			set {
				SetValue (BackgroundProperty, value);
			}
		}
		
		public VisualCollection Children {
			get {
				return (VisualCollection) GetValue (ChildrenProperty);
			}
			
			set {
				SetValue (ChildrenProperty, value);
			}
		}
		
		internal override Kind GetKind ()
		{
			return Kind.PANEL;
		}
	}
}
