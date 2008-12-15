//
// Tickness.cs
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
	public struct Thickness
	{
		private double left;
		private double top;
		private double right;
		private double bottom;
		
		public Thickness (double uniformLength) : 
			this (uniformLength, uniformLength, uniformLength, uniformLength)
		{
		}
		
		public Thickness(double left, double top, double right, double bottom)
		{
			this.left = left;
			this.top = top;
			this.right = right;
			this.bottom = bottom;
		}
		
		public override string ToString ()
		{
			return string.Format ("{0},{1},{2},{3}", Double.IsNaN (left) ? "Auto" : left.ToString (),
				Double.IsNaN (top) ? "Auto" : top.ToString (), 
				Double.IsNaN (right) ? "Auto" : right.ToString (), 
				Double.IsNaN (bottom) ? "Auto" : bottom.ToString ()); 
		}

		public override bool Equals(object obj)
		{
			if (obj == null)
				return false;
			
			if (!(obj is Thickness))
				return false;
			
			return this == (Thickness) obj;
		}
		
		public bool Equals (Thickness thickness)
		{
			return this == thickness;
		}
		
		public override int GetHashCode()
		{
			return left.GetHashCode () ^ top.GetHashCode () ^ right.GetHashCode () ^ bottom.GetHashCode ();
		}
		
		public static bool operator == (Thickness t1, Thickness t2)
		{
			return t1.left == t2.left &&
				t1.right == t2.right &&
				t1.top == t2.top &&
				t1.bottom == t2.bottom;
		}
		
		public static bool operator != (Thickness t1, Thickness t2)
		{
			return !(t1 == t2);
		}
		
		public double Left {
			get { return left; }
			set { left = value; }
		}
		
		public double Top {
			get { return top; }
			set { top = value; }	
		}
		
		public double Right { 
			get { return right; }
			set { right = value; }
		}
		
		public double Bottom { 
			get { return bottom; }
			set { bottom = value; }
		}
	}
}
