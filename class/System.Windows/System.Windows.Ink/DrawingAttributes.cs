// Author:
//   Rolf Bjarne Kvinge  (RKvinge@novell.com)
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
using System.Windows;
using System.Windows.Media;
using System.Windows.Input;
using System.Runtime.InteropServices;
using Mono;

namespace System.Windows.Ink
{
	public sealed class DrawingAttributes : DependencyObject
	{
		public static readonly DependencyProperty ColorProperty = 
			DependencyProperty.Lookup (Kind.DRAWINGATTRIBUTES, "Color", typeof (Color));
		
		public static readonly DependencyProperty HeightProperty = 
			DependencyProperty.Lookup (Kind.DRAWINGATTRIBUTES, "Height", typeof (double));
		
		public static readonly DependencyProperty OutlineColorProperty = 
			DependencyProperty.Lookup (Kind.DRAWINGATTRIBUTES, "OutlineColor", typeof (Color));
		
		public static readonly DependencyProperty WidthProperty = 
			DependencyProperty.Lookup (Kind.DRAWINGATTRIBUTES, "Width", typeof (double));

		public DrawingAttributes() : base (NativeMethods.drawing_attributes_new ())
		{
		}
		
		internal DrawingAttributes (IntPtr raw) : base (raw)
		{
		}
		
		public Color Color { 
			get {
				return (Color) GetValue (ColorProperty);
			}
			set {
				SetValue (ColorProperty, value);
			}
		}
		
		public double Height { 
			get {
				return (double) GetValue (HeightProperty);
			}
			set {
				SetValue (HeightProperty, value);
			}
		}
		
		public Color OutlineColor { 
			get {
				return (Color) GetValue (OutlineColorProperty);
			}
			set {
				SetValue (OutlineColorProperty, value);
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
		
		internal override Kind GetKind ()
		{
			return Kind.DRAWINGATTRIBUTES;
		}
	}

}


 
