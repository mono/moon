//
// Size.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
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
	public struct Size  {
		double w, h;

		public Size (double width, double height)
		{
			Width = width;
			Height = height;
		}
		
		public override bool Equals (object o)
		{
			if (!(o is Size))
				return false;

			return Equals ((Size) o);
		}
		
		public bool Equals (Size value)
		{
			return value.w == w && value.h == h;
		}
		
		public override int GetHashCode ()
		{
			return ((int) w) ^ ((int) h);
		}
		
		public static bool operator == (Size size1, Size size2)
		{
			return size1.w == size2.w && size1.h == size2.h;
		}
			
		public static bool operator != (Size size1, Size size2)
		{
			return size1.w != size2.w || size1.h != size2.h;
		}
		
		public double Height {
			get { return h; } 
			set { 	
				if (value < 0)
					throw new ArgumentException ();

				h = value; 
			}
		}

		public double Width {
			get { return w; }
			set {
				if (value < 0)
					throw new ArgumentException ();

				w = value; 
			}
		}

		public bool IsEmpty {
			get { return w == Double.NegativeInfinity && h == Double.NegativeInfinity; }
		}
		
		public static Size Empty {
			get {
				Size s = new Size ();
				s.w = Double.NegativeInfinity;
				s.h = Double.NegativeInfinity;
				return s;
			}
		}

		internal Size Max (Size other)
		{
			return new Size (
				Math.Max (Width, other.Width),
				Math.Max (Height, other.Height)
			);
		}

		internal Size Min (Size other)
		{
			return new Size (
				Math.Min (Width, other.Width),
				Math.Min (Height, other.Height)
			);
		}

		public override string ToString ()
		{
			if (IsEmpty)
				return "Empty";

			return string.Format ("{0},{1}", w, h);
		}
	}
}
