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

		public Rect(Point location, Size size)
		{
			this.x = location.X;
			this.y = location.Y;
			this.w = size.Width;
			this.h = size.Height;
		}
		
		public Rect(Point point1, Point point2)
		{
			this.x = Math.Min (point1.X, point2.X);
			this.y = Math.Min (point1.Y, point2.Y);
			this.w = Math.Abs (point2.X - point1.X);
			this.h = Math.Abs (point2.Y - point1.Y);
		}
		
		public Rect(Size size)
		{
			this.x = 0;
			this.y = 0;
			this.w = size.Width;
			this.h = size.Height;
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
		
		public bool Contains(double x, double y)
		{
			return Contains (new Point (x, y));
		}
		
		public bool Contains(Rect rect)
		{
			return Contains (rect.TopLeft) && Contains (rect.BottomRight);
		}
		
		public static Rect Empty { 
			get { return new Rect (0, 0, 0, 0); } 
		}
		
		public bool IsEmpty { 
			get {
				return w <= 0 || h <= 0;
			}
		}
		
		public Point Location { 
			get {
				return new Point (x, y);
			}
			set {
				x = value.X;
				y = value.Y;
			}
		}
		
		public Size Size { 
			get { 
				return new Size (w, h);
			}
			set {
				w = value.Width;
				h = value.Height;
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
		
		public Point TopLeft { 
			get { return new Point (Top, Left); }
		}
		
		public Point TopRight { 
			get { return new Point (Top, Right); }
		}
		
		public Point BottomLeft { 
			get { return new Point (Bottom, Left); }
		}
		
		public Point BottomRight { 
			get { return new Point (Bottom, Right); }
		}
		
		public bool IntersectsWith(Rect rect)
		{
			return !((Left >= rect.Right) || (Right <= rect.Left) ||
			    (Top >= rect.Bottom) || (Bottom <= rect.Top));
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
		
		public static Rect Intersect(Rect rect1, Rect rect2)
		{
			Rect result = rect1;
			result.Intersect (rect2);
			return result;
		}
		
		public static Rect Union(Rect rect1, Rect rect2)
		{
			Rect result = rect1;
			result.Union (rect2);
			return result;
		}
		
		public static Rect Union(Rect rect, Point point)
		{
			Rect result = rect;
			result.Union (point);
			return result;
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
		
		public void Offset(double offsetX, double offsetY)
		{
			x += offsetX;
			y += offsetY;
		}
		
		public static Rect Offset(Rect rect, double offsetX, double offsetY)
		{
			Rect result = rect;
			result.Offset (offsetX, offsetY);
			return result;
		}
		
		public void Inflate(Size size)
		{
			Inflate (size.Width, size.Height);
		}
		
		public void Inflate(double width, double height)
		{
			x -= width;
			y -= height;
			w += width;
			h += height;	
		}
		
		public static Rect Inflate(Rect rect, Size size)
		{
			return Inflate (rect, size.Width, size.Height);
		}
		
		public static Rect Inflate(Rect rect, double width, double height)
		{
			Rect result = rect;
			result.Inflate (width, height);
			return result;
		}
		
		public void Scale(double scaleX, double scaleY)
		{
			x *= scaleX;
			y *= scaleY;
			w *= scaleX;
			h *= scaleY;
		}
		
		public static bool operator ==(Rect rect1, Rect rect2)
		{
			return rect1.x == rect2.x && rect1.y == rect2.y && rect1.w == rect2.w && rect1.h == rect2.h;
		}
		
		public static bool operator !=(Rect rect1, Rect rect2)
		{
			return !(rect1 == rect2);
		}
		
		public static bool Equals(Rect rect1, Rect rect2)
		{
			return rect1 == rect2;
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
