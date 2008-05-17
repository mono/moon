//
// System.Windows.Media.Color struct
//
// Authors:
//	Sebastien Pouliot  <sebastien@ximian.com>
//
// Copyright (C) 2007-2008 Novell, Inc (http://www.novell.com)
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

namespace System.Windows.Media {

	public struct Color {

		// maybe we should keep the 4 floats value and compute the argb on them
		// but that would require 4 times the memory

		// Its internal for now, but we should turn Color into a binary
		// compatible format so we can pass that directly to Value, right
		// now we are using a hack that creates the value out of the argb
		// to be able to call C++'s uint32 constructor, but we should be
		// passing all doubles instead.
		internal uint argb;


		private Color (uint value)
		{
			argb = value;
		}

		public static Color FromArgb (byte a, byte r, byte g, byte b)
		{
			return new Color ((uint)(a << 24 | r << 16 | g << 8 | b));
		}

		public byte A {
			get { return (byte)(argb >> 24); }
			set { argb = ((uint)(value << 24) | (argb & 0x00FFFFFF)); }
		}

		public byte R {
			get { return (byte)((argb >> 16) & 0xFF); }
			set { argb = ((uint)(value << 16) | (argb & 0xFF00FFFF)); }
		}

		public byte G {
			get { return (byte)((argb >> 8) & 0xFF); }
			set { argb = ((uint)(value << 8) | (argb & 0xFFFF00FF)); }
		}

		public byte B {
			get { return (byte)(argb & 0xFF); }
			set { argb = (uint)(value | (argb & 0xFFFFFF00)); }
		}

		public override int GetHashCode ()
		{
			return (int) argb;
		}

		public override bool Equals (object o)
		{
			return (o is Color) ? Equals ((Color)o) : false;
		}

		public bool Equals (Color color)
		{
			return (argb == color.argb);
		}

		public override string ToString ()
		{
			return String.Format ("#{0,8:X}", argb);
		}

		public static Color Add (Color color1, Color color2)
		{
			return (color1 + color2);
		}

		public static Color Multiply (Color color, float coefficient)
		{
			return color * coefficient;
		}

		public static Color operator + (Color color1, Color color2)
		{
			return Color.FromArgb ((byte)(color1.A + color2.A), (byte)(color1.R + color2.R), 
				(byte)(color1.G + color2.G), (byte)(color1.B + color2.B));
		}

		public static Color operator * (Color color, float coefficient)
		{
			return Color.FromArgb ((byte)(color.A * coefficient),
					       (byte)(color.R * coefficient),
					       (byte)(color.G * coefficient),
					       (byte)(color.B * coefficient));
		}

		public static bool operator == (Color color1, Color color2)
		{
			return (color1.argb == color2.argb);
		}

		public static bool operator != (Color color1, Color color2)
		{
			return (color1.argb != color2.argb);
		}
	}
}
