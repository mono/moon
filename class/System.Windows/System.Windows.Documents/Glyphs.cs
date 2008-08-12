//
// System.Windows.Documents.Glyphs.cs
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

using Mono;
using System;
using System.Windows;
using System.Windows.Media;

namespace System.Windows.Documents {
	public sealed partial class Glyphs : FrameworkElement {
		
		public Glyphs ()  : base (NativeMethods.glyphs_new ())
		{
		}
		
		internal Glyphs (IntPtr raw) : base (raw)
		{
		}
		
		internal override Kind GetKind ()
		{
			return Kind.GLYPHS;
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
			get { return (Uri) GetValue (FontUriProperty); }
			set { SetValue (FontUriProperty, value); }
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
	}
}
