//
// System.Windows.Media.Colors class
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
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

namespace System.Windows.Media {

	public sealed class Colors {

		internal Colors () { }

		public static Color Black {
			get { return Color.FromArgb (0xFF, 0x00, 0x00, 0x00); }
		}

		public static Color Blue {
			get { return Color.FromArgb (0xFF, 0x00, 0x00, 0xFF); }
		}

		public static Color Brown {
			get { return Color.FromArgb (0xFF, 0xA5, 0x2A, 0x2A); }
		}

		public static Color Cyan {
			get { return Color.FromArgb (0xFF, 0x00, 0xFF, 0xFF); }
		}

		public static Color DarkGray {
			get { return Color.FromArgb (0xFF, 0xA9, 0xA9, 0xA9); }
		}

		public static Color Gray {
			get { return Color.FromArgb (0xFF, 0x80, 0x80, 0x80); }
		}

		public static Color Green {
			get { return Color.FromArgb (0xFF, 0x00, 0x80, 0x00); }
		}

		public static Color LightGray {
			get { return Color.FromArgb (0xFF, 0xD3, 0xD3, 0xD3); }
		}

		public static Color Magenta {
			get { return Color.FromArgb (0xFF, 0xFF, 0x00, 0xFF); }
		}

		public static Color Orange {
			get { return Color.FromArgb (0xFF, 0xFF, 0xA5, 0x00); }
		}

		public static Color Purple {
			get { return Color.FromArgb (0xFF, 0x80, 0x00, 0x80); }
		}

		public static Color Red {
			get { return Color.FromArgb (0xFF, 0xFF, 0x00, 0x00); }
		}

		public static Color Transparent {
			get { return Color.FromArgb (0x00, 0xFF, 0xFF, 0xFF); }
		}

		public static Color White {
			get { return Color.FromArgb (0xFF, 0xFF, 0xFF, 0xFF); }
		}

		public static Color Yellow {
			get { return Color.FromArgb (0xFF, 0xFF, 0xFF, 0x00); }
		}
	}
}
