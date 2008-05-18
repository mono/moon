//
// Point.cs
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

namespace System.Windows {
	public struct Point  {
		double x, y;

		public Point (double x, double y)
		{
			this.x = x;
			this.y = y;
		}

		public override bool Equals (object o)
		{
			if (!(o is Point))
				return false;

			return Equals ((Point) o);
		}
		
		public bool Equals (Point value)
		{
			return value.x == x && value.y == y;
		}
			
		public override int GetHashCode ()
		{
			return ((int) x) ^ ((int) y);
		}
			
		public override string ToString ()
		{
			return String.Format ("{0},{1}", x, y);
		}

		public static bool operator == (Point point1, Point point2)
		{
			return (point1.x == point2.x) && (point1.y == point2.y);
		}
		
		public static bool operator != (Point point1, Point point2)
		{
			return point1.x != point2.x || point1.y != point2.y;
		}
		
		public double X {
			get { return x; } 
			set { x = value; } 
		}
		
		public double Y {
			get { return y; }
			set { y = value; } 
		}

	}
}
