//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (c) 2008 Novell, Inc (http://www.novell.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

using System;
using Mono;
using System.Collections.ObjectModel;
using System.Windows.Media;


namespace System.Windows.Controls
{	
	public partial class MultiScaleSubImage : DependencyObject
	{
		public static readonly DependencyProperty AspectRatioProperty = DependencyProperty.Lookup (Kind.MULTISCALESUBIMAGE, "AspectRatio", typeof (double));
		public static readonly DependencyProperty OpacityProperty = DependencyProperty.Lookup (Kind.MULTISCALESUBIMAGE, "Opacity", typeof (double));
		public static readonly DependencyProperty ViewportOriginProperty = DependencyProperty.Lookup (Kind.MULTISCALESUBIMAGE, "ViewportOrigin", typeof (Point));
		public static readonly DependencyProperty ViewportWidthProperty = DependencyProperty.Lookup (Kind.MULTISCALESUBIMAGE, "ViewportWidth", typeof (double));
		public static readonly DependencyProperty ZIndexProperty = DependencyProperty.Lookup (Kind.MULTISCALESUBIMAGE, "ZIndex", typeof (int));

		public double AspectRatio {
			get { return (double) GetValue (AspectRatioProperty); }
		}

		public double Opacity {
			get { return (double) GetValue (OpacityProperty); }
			set { SetValue (OpacityProperty, value); }
		}

		public Point ViewportOrigin {
			get { return (Point) GetValue (ViewportOriginProperty); }
			set { NativeMethods.multi_scale_sub_image_set_viewport_origin (this.native, value); }
		}

		public double ViewportWidth {
			get { return (double) GetValue (ViewportWidthProperty); }
			set { NativeMethods.multi_scale_sub_image_set_viewport_width (this.native, value); }
		}

		public int ZIndex {
			get { return (int) GetValue (ZIndexProperty); }
			set { SetValue (ZIndexProperty, value); }
		}

	}

}
