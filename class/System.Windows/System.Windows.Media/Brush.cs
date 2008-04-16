//
// Brush.cs
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
using Mono;
namespace System.Windows.Media {
	public abstract class Brush : DependencyObject {
		
		public static readonly DependencyProperty OpacityProperty;
		public static readonly DependencyProperty TransformProperty;
		public static readonly DependencyProperty RelativeTransformProperty;

		static Brush ()
		{
			OpacityProperty = DependencyProperty.Lookup (Kind.BRUSH, "Opacity", typeof (double));
			TransformProperty = DependencyProperty.Lookup (Kind.BRUSH, "Transform", typeof (Transform));
			RelativeTransformProperty = DependencyProperty.Lookup (Kind.BRUSH, "RelativeTransform", typeof (Transform));
		}
		
		public Brush () : base (NativeMethods.brush_new ())
		{
		}
		
		internal Brush (IntPtr raw) : base (raw)
		{
		}

		public double Opacity {
			get {
				return (double) GetValue (OpacityProperty);
			}
			
			set {
				SetValue (OpacityProperty, value);
			}
		}
		
		public Transform RelativeTransform {
			get {
				return (Transform) GetValue (RelativeTransformProperty);
			}

			set {
				SetValue (RelativeTransformProperty, value);
			}
		}
		
		public Transform Transform {
			get {
				return (Transform) GetValue (TransformProperty);
			}

			set {
				SetValue (TransformProperty, value);
			}
		}
		internal override Kind GetKind ()
		{
			return Kind.BRUSH;
		}
	}
}
