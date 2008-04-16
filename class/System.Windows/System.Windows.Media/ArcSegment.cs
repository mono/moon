//
// ArgSegment.cs
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
	public sealed class ArcSegment : PathSegment {

		static ArcSegment ()
		{
			PointProperty = DependencyProperty.Lookup (Kind.ARCSEGMENT, "Point", typeof (Point));
			SizeProperty = DependencyProperty.Lookup (Kind.ARCSEGMENT, "Size", typeof (Point));
			RotationAngleProperty = DependencyProperty.Lookup (Kind.ARCSEGMENT, "RotationAngle", typeof (double));
			IsLargeArcProperty = DependencyProperty.Lookup (Kind.ARCSEGMENT, "IsLargeArc", typeof (bool));
			SweepDirectionProperty = DependencyProperty.Lookup (Kind.ARCSEGMENT, "SweepDirection", typeof (SweepDirection));
			
		}
		
		public ArcSegment () : base (NativeMethods.arc_segment_new ())
		{
		}
		
		internal ArcSegment (IntPtr raw) : base (raw)
		{
		}

	        public bool IsLargeArc {
	                get {
				return (bool) GetValue (IsLargeArcProperty);
			}
	                set {
				SetValue (IsLargeArcProperty, value);
			}
	        }
		
	        public Point Point {
	                get {
				return (Point) GetValue (PointProperty);
			}
			
	                set {
				SetValue (PointProperty, value);
			}
	        }
		
	        public double RotationAngle {
	                get {
				return (double) GetValue (RotationAngleProperty);
			}
			
	                set {
				SetValue (RotationAngleProperty, value);
			}
	        }
		
	        public Point Size {
	                get {
				return (Point) GetValue (SizeProperty);
			}
			
	                set {
				SetValue (SizeProperty, value);
			}
	        }
		
	        public SweepDirection SweepDirection {
	                get {
				return (SweepDirection) GetValue (SweepDirectionProperty);
			}
			
	                set {
				SetValue (SweepDirectionProperty, value);
			}
	        }
	
	        public static readonly DependencyProperty PointProperty;
	        public static readonly DependencyProperty SizeProperty;
	        public static readonly DependencyProperty RotationAngleProperty;
	        public static readonly DependencyProperty IsLargeArcProperty;
	        public static readonly DependencyProperty SweepDirectionProperty;
			
		internal override Kind GetKind ()
		{
			return Kind.ARCSEGMENT;
		}
	}
}
