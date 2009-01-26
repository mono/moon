//
// System.Windows.Media.Color struct
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
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

	public struct Color : IFormattable {

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
			return String.Format ("#{0:X8}", argb);
		}

		public string ToString (IFormatProvider provider)
		{
			if (provider == null)
				return ToString ();

			if (provider != null) {
				ICustomFormatter cp = (ICustomFormatter) provider.GetFormat (typeof (ICustomFormatter));
				if (cp != null) {
					return String.Format ("#{0}{1}{2}{3}", 	cp.Format ("X2", A, provider),
						cp.Format ("X2", R, provider), cp.Format ("X2", G, provider),
						cp.Format ("X2", B, provider));
				}
			}

			return String.Format ("#{0}{1}{2}{3}", 	A.ToString ("X2", provider), R.ToString ("X2", provider), 
				G.ToString ("X2", provider), B.ToString ("X2", provider));
		}

		string IFormattable.ToString (string value, IFormatProvider formatProvider)
		{
			if (value == null)
				return ToString (formatProvider);

			if (formatProvider != null) {
				ICustomFormatter cp = (ICustomFormatter) formatProvider.GetFormat (typeof (ICustomFormatter));
				if (cp != null) {
					return String.Format ("sc#{0}{1} {2}{3} {4}{5} {6}", 
						cp.Format (value, A, formatProvider), cp.Format (value, ',', formatProvider),
						cp.Format (value, R, formatProvider), cp.Format (value, ',', formatProvider),
						cp.Format (value, G, formatProvider), cp.Format (value, ',', formatProvider),
						cp.Format (value, B, formatProvider));
				}
			}

			return String.Format ("sc#{0}{1} {2}{3} {4}{5} {6}", 
				A.ToString (value, formatProvider), ','.ToString (formatProvider),
				R.ToString (value, formatProvider), ','.ToString (formatProvider),
				G.ToString (value, formatProvider), ','.ToString (formatProvider),
				B.ToString (value, formatProvider));
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
