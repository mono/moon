//
// CornerRadius.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
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
		
		public CornerRadius (double uniformRadius) : this (uniformRadius, uniformRadius, uniformRadius, uniformRadius)
		{
		}
		
		public CornerRadius (double topLeft, double topRight, double bottomRight, double bottomLeft)
		{
			TopLeft = topLeft;
			TopRight = topRight;
			BottomRight = bottomRight;
			BottomLeft = bottomLeft;
		}
		
		public override string ToString ()
		{
			return String.Format ("{0},{1},{2},{3}", top_left, top_right, bottom_right, bottom_left);
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
		
		public override int GetHashCode()
		{
			return top_left.GetHashCode () ^ top_right.GetHashCode () ^ 
				bottom_right.GetHashCode () ^ bottom_left.GetHashCode ();
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
		
		private double CheckValue (double value, string name)
		{
			if ((value < 0.0) || Double.IsNaN (value))
				throw new ArgumentException ("Invalid value", name);
			return value;
		}

		public double TopLeft {
			get { return top_left; }
			set { top_left = CheckValue (value, "TopLeft"); }
		}
		
		public double TopRight { 
			get { return top_right; }
			set { top_right = CheckValue (value, "TopRight"); }
		}
			
		public double BottomRight {
			get { return bottom_right; }
			set { bottom_right = CheckValue (value, "BottomRight"); }
		}
		
		public double BottomLeft {
			get { return bottom_left; }
			set { bottom_left = CheckValue (value, "BottomLeft"); }
		}
	}
}
