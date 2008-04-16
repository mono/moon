//
// System.Windows.Documents.Glyphs.cs
//
// Authors:
//	Atsushi Enomoto  <atsushi@ximian.com>
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

using Mono;
using System.Windows;
using System.Windows.Media;

namespace System.Windows.Documents {

	public sealed class Glyphs : FrameworkElement {

		public static readonly DependencyProperty FillProperty =
			DependencyProperty.Lookup (Kind.GLYPHS, "Fill", typeof (Brush));
		public static readonly DependencyProperty FontRenderingEmSizeProperty =
			DependencyProperty.Lookup (Kind.GLYPHS, "FontRenderingEmSize", typeof (double));
		public static readonly DependencyProperty FontUriProperty =
			DependencyProperty.Lookup (Kind.GLYPHS, "FontUri", typeof (string));
		public static readonly DependencyProperty IndicesProperty=
			DependencyProperty.Lookup (Kind.GLYPHS, "Indices", typeof (string));
		public static readonly DependencyProperty OriginXProperty =
			DependencyProperty.Lookup (Kind.GLYPHS, "OriginX", typeof (double));
		public static readonly DependencyProperty OriginYProperty =
			DependencyProperty.Lookup (Kind.GLYPHS, "OriginY", typeof (double));
		public static readonly DependencyProperty StyleSimulationsProperty =
			DependencyProperty.Lookup (Kind.GLYPHS, "StyleSimulations", typeof (StyleSimulations));
		public static readonly DependencyProperty UnicodeStringProperty =
			DependencyProperty.Lookup (Kind.GLYPHS, "UnicodeString", typeof (string));


		public Glyphs ()  : base (NativeMethods.glyphs_new ())
		{
		}

		internal Glyphs (IntPtr raw) : base (raw)
		{
		}

		public Brush Fill {
			get { return (Brush) GetValue (FillProperty); }
			set { SetValue (FillProperty, value); }
		}

		public double FontRenderingEmSize {
			get { return (double) GetValue (FontRenderingEmSizeProperty); }
			set { SetValue (FontRenderingEmSizeProperty, value); }
		}

		public Uri FontUri {
			// Uri is not a DependencyProperty so we convert it to string.
			get { return new Uri ((string) GetValue (FontUriProperty)); }
			set { SetValue (FontUriProperty, value.OriginalString); }
		}

		public string Indices {
			get { return (string) GetValue (IndicesProperty); }
			set { SetValue (IndicesProperty, value); }
		}

		public double OriginX {
			get { return (double) GetValue (OriginXProperty); }
			set { SetValue (OriginXProperty, value); }
		}

		public double OriginY {
			get { return (double) GetValue (OriginYProperty); }
			set { SetValue (OriginYProperty, value); }
		}

		public StyleSimulations StyleSimulations {
			get { return (StyleSimulations) GetValue (StyleSimulationsProperty); }
			set { SetValue (StyleSimulationsProperty, value); }
		}

		public string UnicodeString {
			get { return (string) GetValue (UnicodeStringProperty); }
			set { SetValue (UnicodeStringProperty, value); }
		}

		internal override Kind GetKind ()
		{
			return Kind.GLYPHS;
		}
	}
}
