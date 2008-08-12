//
// System.Windows.Controls.Control
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
using Mono.Xaml;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Markup;

namespace System.Windows.Controls {
	public abstract partial class Control : FrameworkElement {
		
		protected Control ()  : base (NativeMethods.control_new ())
		{
		}
		
		internal Control (IntPtr raw) : base (raw)
		{
		}
		
		internal override Kind GetKind ()
		{
			return Kind.CONTROL;
		}
		
		public Brush Background {
			get { return (Brush) GetValue (BackgroundProperty); }
			set { SetValue (BackgroundProperty, value); }
		}
		
		public Brush BorderBrush {
			get { return (Brush) GetValue (BorderBrushProperty); }
			set { SetValue (BorderBrushProperty, value); }
		}
		
		public Thickness BorderThickness {
			get { return (Thickness) GetValue (BorderThicknessProperty); }
			set { SetValue (BorderThicknessProperty, value); }
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
		
		public HorizontalAlignment HorizontalContentAlignment {
			get { return (HorizontalAlignment) GetValue (HorizontalContentAlignmentProperty); }
			set { SetValue (HorizontalContentAlignmentProperty, value); }
		}
		
		public bool IsTabStop {
			get { return (bool) GetValue (IsTabStopProperty); }
			set { SetValue (IsTabStopProperty, value); }
		}
		
		public Thickness Padding {
			get { return (Thickness) GetValue (PaddingProperty); }
			set { SetValue (PaddingProperty, value); }
		}
		
		public int TabIndex {
			get { return (int) GetValue (TabIndexProperty); }
			set { SetValue (TabIndexProperty, value); }
		}
		
		public KeyboardNavigationMode TabNavigation {
			get { return (KeyboardNavigationMode) GetValue (TabNavigationProperty); }
			set { SetValue (TabNavigationProperty, value); }
		}
		
		public ControlTemplate Template {
			get { return (ControlTemplate) GetValue (TemplateProperty); }
			set { SetValue (TemplateProperty, value); }
		}
		
		public VerticalAlignment VerticalContentAlignment {
			get { return (VerticalAlignment) GetValue (VerticalContentAlignmentProperty); }
			set { SetValue (VerticalContentAlignmentProperty, value); }
		}

		public Style Style {
			get { return (Style) GetValue (StyleProperty); }
			set { SetValue (StyleProperty, value); }
		}

		public virtual void OnApplyTemplate() {
			// FIXME
		}

		public bool Focus() {
			throw new NotImplementedException ();
		}

		public double ActualWidth {
			get {
				throw new NotImplementedException ();
			}
		}

		public double ActualHeight {
			get {
				throw new NotImplementedException ();
			}
		}

		protected DependencyObject GetTemplateChild(string childName) {
			throw new NotImplementedException ();
		}

		public bool ApplyTemplate() {
			throw new NotImplementedException ();
		}
		
	}
}
