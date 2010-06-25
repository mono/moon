//
// LayoutInformation.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2008 Novell, Inc.
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

#pragma warning disable 3001 // "Argument type of 'X' is not CLS-compliant" shown for the Dispatcher argument in GetLayoutExceptionElement
#pragma warning disable 169 // "The private method 'M' is never used" shown for GetRawLayoutData, which is accessed using reflection from the test harness

using Mono;
using System;
using System.Windows;
using System.Windows.Media;
using System.Windows.Threading;

namespace System.Windows.Controls.Primitives {

	public static class LayoutInformation {
		private static readonly DependencyProperty LayoutClipProperty = DependencyProperty.Lookup (Kind.LAYOUTINFORMATION, "LayoutClip", typeof (Geometry));
		private static readonly DependencyProperty LayoutSlotProperty = DependencyProperty.Lookup (Kind.LAYOUTINFORMATION, "LayoutSlot", typeof (Rect));
		private static readonly DependencyProperty LayoutExceptionElementProperty = DependencyProperty.Lookup (Kind.LAYOUTINFORMATION, "LayoutExceptionElement", typeof (UIElement));

		public static Rect GetLayoutSlot (FrameworkElement element)
		{
			return (Rect) element.GetValue (LayoutInformation.LayoutSlotProperty);
		}

		public static Geometry GetLayoutClip (FrameworkElement element)
		{
			return (Geometry) element.GetValue (LayoutInformation.LayoutClipProperty);
		}

		public static UIElement GetLayoutExceptionElement (Dispatcher dispatcher)
		{
			return (UIElement) Deployment.Current.GetValue (LayoutExceptionElementProperty);
		}


		internal static void SetLayoutExceptionElement (Dispatcher dispatcher, UIElement element)
		{
			Deployment.Current.SetValue (LayoutExceptionElementProperty, element);
		}

		private static float[] GetRawLayoutData (FrameworkElement element)
		{
			return null;
		}
	}
}
