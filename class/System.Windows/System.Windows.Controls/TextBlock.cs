//
// System.Windows.Controls.TextBlock.cs
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
using System.Windows;
using System.Windows.Media;
using System.Windows.Documents;

namespace System.Windows.Controls {
	public sealed partial class TextBlock : FrameworkElement {

		public TextBlock ()  : base (NativeMethods.text_block_new ())
		{
		}
		
		internal TextBlock (IntPtr raw) : base (raw)
		{
		}
		
		internal override Kind GetKind ()
		{
			return Kind.TEXTBLOCK;
		}
		
		public double ActualHeight {
			get { return (double) GetValue (ActualHeightProperty); }
		}
		
		public double ActualWidth {
			get { return (double) GetValue (ActualWidthProperty); }
		}
		
		public FontFamily FontFamily {
			get { return (FontFamily) GetValue (FontFamilyProperty); }
			set { SetValue (FontFamilyProperty, value); }
		}
		
		public double FontSize {
			get { return (double) GetValue (FontSizeProperty); }
			set { SetValue (FontSizeProperty, value); }
		}
		
		public FontStretch FontStretch {
			get { return (FontStretch) GetValue (FontStretchProperty); }
			set { SetValue (FontStretchProperty, value); }
		}
		
		public FontStyle FontStyle {
			get { return (FontStyle) GetValue (FontStyleProperty); }
			set { SetValue (FontStyleProperty, value); }
		}
		
		public FontWeight FontWeight {
			get { return (FontWeight) GetValue (FontWeightProperty); }
			set { SetValue (FontWeightProperty, value); }
		}
		
		public Brush Foreground {
			get { return (Brush) GetValue (ForegroundProperty); }
			set { SetValue (ForegroundProperty, value); }
		}
		
		public InlineCollection Inlines {
			get { return (InlineCollection) GetValue (InlinesProperty); }
		}
		
		public TextDecorationCollection TextDecorations {
			get { return (TextDecorationCollection) GetValue (TextDecorationsProperty); }
			set { SetValue (TextDecorationsProperty, value); }
		}
		
		public TextWrapping TextWrapping {
			get { return (TextWrapping) GetValue (TextWrappingProperty); }
			set { SetValue (TextWrappingProperty, value); }
		}
		
		public string Text {
			get { return (string) GetValue (TextProperty); }
			set { SetValue (TextProperty, value); }
		}
	}
}
