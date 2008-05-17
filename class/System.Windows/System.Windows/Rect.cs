//
// System.Windows.Rect structure
//
// Authors:
//	Sebastien Pouliot  <sebastien@ximian.com>
//
// Copyright (C) 2007 Novell, Inc (http://www.novell.com)
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

	public struct Rect {

		private double x, y, w, h;

		public Rect (double x, double y, double width, double height)
		{
			this.x = x;
			this.y = y;
			w = width;
			h = height;
		}

		public Rect(Point point1, Point point2)
		{
			this.x = Math.Min (point1.X, point2.X);
			this.y = Math.Min (point1.Y, point2.Y);
			this.w = Math.Abs (point2.X - point1.X);
			this.h = Math.Abs (point2.Y - point1.Y);
		}
		
		public override string ToString ()
		{
			return String.Format ("{0},{1},{2},{3}", x, y, w, h);
		}

		public double X {
			get { return x; }
			set { x = value; }
		}

		public double Y {
			get { return y; }
			set { y = value; } 
		}

		public double Width {
			get { return w; }
			set { w = value; } 
		}

		public double Height {
			get { return h; }
			set { h = value; }
		}

		public bool Contains (Point point)
		{
			double px = point.X;
			if (px < x)
				return false;
			if (px > x+w)
				return false;
			
			double py = point.Y;
			if (py < y)
				return false;
			if (py > y + h)
				return false;

			return true;
		}
		
		public static Rect Empty { 
			get { return new Rect (0, 0, 0, 0); } 
		}
		
		public bool IsEmpty { 
			get {
				return w <= 0 || h <= 0;
			}
		}
		
		public double Left { 
			get { return x; }
		}
		
		public double Top { 
			get { return y; }
		}
		
		public double Right { 
			get { return x + w; }
		}
		
		public double Bottom { 
			get { return y + h; }
		}
		
		public void Intersect(Rect rect)
		{
			double new_x = Math.Max (x, rect.x);
			double new_y = Math.Max (y, rect.y);
			double new_w = Math.Min (Right, rect.Right) - new_x;
			double new_h = Math.Min (Bottom, rect.Bottom) - new_y; 

			x = new_x;
			y = new_y;
			w = new_w;
			h = new_h;

			if (w < 0 || h < 0) {
				x = y = w = h = 0;
			}
		}

		public void Union(Rect rect)
		{
			double new_x = Math.Min (x, rect.x);
			double new_y = Math.Min (y, rect.y);
			double new_w = Math.Max (Right, rect.Right) - x;
			double new_h = Math.Max (Bottom, rect.Bottom) - y;

			x = new_x;
			y = new_y;
			w = new_w;
			h = new_h;
		}
		
		public void Union(Point point)
		{
			Union (new Rect (point, point));
		}
		
		public static bool operator ==(Rect rect1, Rect rect2)
		{
			return rect1.x == rect2.x && rect1.y == rect2.y && rect1.w == rect2.w && rect1.h == rect2.h;
		}
		
		public static bool operator !=(Rect rect1, Rect rect2)
		{
			return !(rect1 == rect2);
		}
		
		public override bool Equals(object o)
		{
			if (o is Rect) {
				return this == (Rect) o;
			} else {
				return false;
			}
		}
		
		public bool Equals(Rect value)
		{
			return this == value;
		}
		
		public override int GetHashCode()
		{
			return base.GetHashCode ();
		}
	}
}
