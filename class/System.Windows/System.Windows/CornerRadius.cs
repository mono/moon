////
//// CornerRadius.cs
////
//// Authors:
////   Rolf Bjarne Kvinge (rkvinge@novell.com)
////
//// Copyright 2008 Novell, Inc.
////
//// Permission is hereby granted, free of charge, to any person obtaining
//// a copy of this software and associated documentation files (the
//// "Software"), to deal in the Software without restriction, including
//// without limitation the rights to use, copy, modify, merge, publish,
//// distribute, sublicense, and/or sell copies of the Software, and to
//// permit persons to whom the Software is furnished to do so, subject to
//// the following conditions:
//// 
//// The above copyright notice and this permission notice shall be
//// included in all copies or substantial portions of the Software.
//// 
//// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
//// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
//// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
//// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
////
//

using System;
using System.Runtime.InteropServices;

namespace System.Windows
{
	[StructLayout(LayoutKind.Sequential)]
	public struct CornerRadius
	{
		private double top_left;
		private double top_right;
		private double bottom_right;
		private double bottom_left;
		
		public CornerRadius (double uniformRadius)
			: this (uniformRadius, uniformRadius, uniformRadius, uniformRadius)
		{
		}
		
		public CornerRadius (double topLeft, double topRight, double bottmRight, double bottomLeft)
		{
			top_left = topLeft;
			top_right = topRight;
			bottom_right = bottmRight; // Typo??
			bottom_left = bottomLeft;
		}
		
		[MonoTODO ()]
		public override string ToString ()
		{
			return base.ToString ();
		}
		
		public override bool Equals (object obj)
		{
			if (obj == null)
				return false;
			
			if (!(obj is CornerRadius))
				return false;
			
			return this == (CornerRadius) obj;
		}
		
		public bool Equals (CornerRadius cornerRadius)
		{
			return this == cornerRadius;
		}
		
		[MonoTODO ("We need a hash code algorithm based on the values in the struct")]
		public override int GetHashCode()
		{
			return base.GetHashCode ();
		}
		
		public static bool operator == (CornerRadius cr1, CornerRadius cr2)
		{
			return cr1.bottom_left == cr2.bottom_left &&
				cr1.bottom_right == cr2.bottom_right &&
				cr1.top_left == cr2.top_left &&
				cr1.top_right == cr2.top_right;
		}
		
		public static bool operator != (CornerRadius cr1, CornerRadius cr2)
		{
			return !(cr1 == cr2);
		}
		
		public double TopLeft {
			get { return top_left; }
			set { top_left = value; }
		}
		
		public double TopRight { 
			get { return top_right; }
			set { top_right = value; }
		}
			
		public double BottomRight {
			get { return bottom_right; }
			set { bottom_right = value; }
		}
		
		public double BottomLeft {
			get { return bottom_left; }
			set { bottom_left = value; }
		}
	}
}
