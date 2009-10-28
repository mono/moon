//
// Window.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2009 Novell, Inc.
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

using System;
using System.Windows;
using System.Windows.Controls;

namespace Moon.Windows.Desktop {

	[TemplateVisualState(Name = "Inactive", GroupName = "CommonStates")] 
	[TemplateVisualState(Name = "Normal", GroupName = "CommonStates")] 
	public class Window : ContentControl {

		public Window ()
		{
			DefaultStyleKey = typeof (Window);
		}


		public static readonly DependencyProperty IconProperty
			= DependencyProperty.Register ("Icon", typeof (Image), typeof (Window),
						       new PropertyMetadata (IconChanged));

		public Image Icon {
			get { return (Image)GetValue (IconProperty); }
			set { SetValue (IconProperty, value); }
		}

		static void IconChanged (DependencyObject sender, DependencyPropertyChangedEventArgs e)
		{
			((Window)sender).OnIconChanged (e);
		}


		public static readonly DependencyProperty TitleProperty
			= DependencyProperty.Register ("Title", typeof (string), typeof (Window),
						       new PropertyMetadata ("Moonlight",
									     (sender, e) => {
										     ((Window)sender).OnTitleChanged (e);
									     }));

		public string Title {
			get { return (string)GetValue (TitleProperty); }
			set { SetValue (TitleProperty, value); }
		}

		public static readonly DependencyProperty WindowOpacityProperty
			= DependencyProperty.Register ("WindowOpacity", typeof (double), typeof (Window),
						       new PropertyMetadata (1.0,
									     (sender, e) => {
										     ((Window)sender).OnWindowOpacityChanged (e);
									     }));

		public double WindowOpacity {
			get { return (double)GetValue (WindowOpacityProperty); }
			set { SetValue (WindowOpacityProperty, value); }
		}

		public static readonly DependencyProperty PositionProperty
			= DependencyProperty.Register ("Position", typeof (Point), typeof (Window),
						       new PropertyMetadata (new Point (),
									     (sender, e) => {
										     ((Window)sender).OnPositionChanged (e);
									     }));


		bool changing_position = false;

		public Point Position {
			get { return (Point)GetValue (PositionProperty); }
			set { changing_position = true; SetValue (PositionProperty, value); changing_position = false; }
		}

		public static readonly DependencyProperty SizeProperty
			= DependencyProperty.Register ("Size", typeof (Point), typeof (Window),
						       new PropertyMetadata (new Point (),
									     (sender, e) => {
										     ((Window)sender).OnSizeChanged (e);
									     }));


		bool changing_size = false;

		public Size Size {
			get { Point p = (Point)GetValue (SizeProperty); return new Size (p.X, p.Y); }
			set { changing_size = true; SetValue (SizeProperty, new Point (value.Width, value.Height)); changing_size = false; }
		}

		public static readonly DependencyProperty IsActiveProperty
			= DependencyProperty.Register ("IsActive", typeof (bool), typeof (Window),
						       new PropertyMetadata (false,
									     IsActiveChanged));

		public bool IsActive {
			get { return (bool)GetValue (IsActiveProperty); }
			set { SetValue (IsActiveProperty, value); }
		}

		static void IsActiveChanged (DependencyObject sender, DependencyPropertyChangedEventArgs e)
		{
			((Window)sender).OnIsActiveChanged (e);
		}

#if notyet
		public static readonly DependencyProperty AllowsTransparencyProperty;
		public static readonly DependencyProperty ResizeModeProperty;
		public static readonly DependencyProperty ShowActivatedProperty;
		public static readonly DependencyProperty ShowInTaskbarProperty;
		public static readonly DependencyProperty SizeToContentProperty;
		public static readonly DependencyProperty TopmostProperty;
		public static readonly DependencyProperty WindowStateProperty;
		public static readonly DependencyProperty WindowStyleProperty;
#endif

		public override void OnApplyTemplate ()
		{
			UpdateVisualState (false);
		}

		public event EventHandler PositionChanged;
		public event EventHandler SizeChanged;
		public event EventHandler WindowOpacityChanged;

		void OnPositionChanged (DependencyPropertyChangedEventArgs e)
		{
			if (!changing_position && PositionChanged != null)
				PositionChanged (this, EventArgs.Empty);
		}

		void OnSizeChanged (DependencyPropertyChangedEventArgs e)
		{
			if (!changing_size && SizeChanged != null)
				SizeChanged (this, EventArgs.Empty);
		}

		void OnWindowOpacityChanged (DependencyPropertyChangedEventArgs e)
		{
			if (WindowOpacityChanged != null)
				WindowOpacityChanged (this, EventArgs.Empty);
			Opacity = WindowOpacity;
		}

		void OnTitleChanged (DependencyPropertyChangedEventArgs e)
		{
		}

		void OnIconChanged (DependencyPropertyChangedEventArgs e)
		{
		}

		void OnIsActiveChanged (DependencyPropertyChangedEventArgs e)
		{
			UpdateVisualState ();
		}

		private void UpdateVisualState ()
		{
			UpdateVisualState (true);
		}

		private void UpdateVisualState (bool useTransitions)
		{
			if (!IsActive) {
				VisualStateManager.GoToState(this, "Inactive", useTransitions);
			}
			else {
				VisualStateManager.GoToState(this, "Normal", useTransitions);
			}
		}
		
	}

}