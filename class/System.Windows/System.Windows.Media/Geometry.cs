//
// Geometry.cs
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
	public abstract class Geometry : DependencyObject {
		public static readonly DependencyProperty FillRuleProperty;
		public static readonly DependencyProperty TransformProperty;

		static Geometry ()
		{
			FillRuleProperty = DependencyProperty.Lookup (Kind.GEOMETRY, "FillRule", typeof (FillRule));
			TransformProperty = DependencyProperty.Lookup (Kind.GEOMETRY, "Transform", typeof (Transform));
		}

		public Geometry () : base (NativeMethods.geometry_new ())
		{
		}
		
		internal Geometry (IntPtr raw) : base (raw)
		{
		}

		public FillRule FillRule {
			get {
				return (FillRule) GetValue (FillRuleProperty);
			}
			
			set {
				SetValue (FillRuleProperty, value);
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
			return Kind.GEOMETRY;
		}
		
	}
}
