//
// Viewbox.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2010 Novell, Inc.
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
using System.Windows.Media;
using System.Windows.Markup;

namespace System.Windows.Controls {

	[ContentProperty ("Child")]
	public sealed class Viewbox : FrameworkElement {
		internal static readonly DependencyProperty ChildProperty =
			DependencyProperty.RegisterCore ("Child", typeof (UIElement), typeof (Viewbox), new PropertyMetadata (ChildChanged));

		public static readonly DependencyProperty StretchDirectionProperty =
			DependencyProperty.Register ("StretchDirection", typeof (StretchDirection), typeof (Viewbox), new PropertyMetadata (StretchDirection.Both, StretchDirectionChanged));

		public static readonly DependencyProperty StretchProperty =
			DependencyProperty.Register ("Stretch", typeof (Stretch), typeof (Viewbox), new PropertyMetadata (Stretch.Uniform, StretchChanged));

		static void ChildChanged (object sender, DependencyPropertyChangedEventArgs args)
		{
			var viewbox = (Viewbox) sender;
			var oldElement = (UIElement) args.OldValue;
			var newElement = (UIElement) args.NewValue;

			// This breaks because Border tries to set the logical parent when it shouldn't do.
//			if (oldElement != null)
//				Mono.NativeMethods.framework_element_set_logical_parent (oldElement.native, IntPtr.Zero);
//			if (newElement != null)
//				Mono.NativeMethods.framework_element_set_logical_parent (newElement.native, viewbox.native);

			if (viewbox.ChildBorder != null)
				viewbox.ChildBorder.Child = newElement;
		}

		static void StretchChanged (object sender, DependencyPropertyChangedEventArgs args)
		{
			((Viewbox) sender).InvalidateMeasure ();
		}

		static void StretchDirectionChanged (object sender, DependencyPropertyChangedEventArgs args)
		{
			((Viewbox) sender).InvalidateMeasure ();
		}

		UIElement defaultTemplate;

		public UIElement Child {
			get { return (UIElement) GetValue (ChildProperty); }
			set { SetValue (ChildProperty, value); }
		}

		Border ChildBorder {
			get; set;
		}

		ScaleTransform Scale {
			get; set;
		}

		public UIElement DefaultTemplate {
			get {
				if (defaultTemplate == null) {
					var x = (ControlTemplate) XamlReader.Load (@"
<ControlTemplate
	xmlns=""http://schemas.microsoft.com/client/2007""
	xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
	<Border />
</ControlTemplate>");

					defaultTemplate = (UIElement) x.GetVisualTree ();
				}

				return defaultTemplate;
			}
		}

		public StretchDirection StretchDirection {
			get { return (StretchDirection) GetValue (StretchDirectionProperty); }
			set { SetValue (StretchDirectionProperty, value); }
		}

		public Stretch Stretch {
			get { return (Stretch) GetValue (StretchProperty); }
			set { SetValue (StretchProperty, value); }
		}

		public Viewbox ()
		{
		}

		public override void OnApplyTemplate ()
		{
			ChildBorder = (Border) VisualTreeHelper.GetChild (this, 0);
			if (ChildBorder == null)
				throw new Exception ("Argh!");

			Scale = new ScaleTransform ();
			ChildBorder.RenderTransform = Scale;
			ChildBorder.Child = Child;
		}

		protected override Size ArrangeOverride (Size finalSize)
		{
			// Code in this method has been copied from the implementation
			// in the Silverlight 3 toolkitunder the MS-PL license: http://silverlight.codeplex.com/
			if (Child != null)
			{
				// Determine the scale factor given the final size
				Size desiredSize = ChildBorder.DesiredSize;
				Size scale = ComputeScaleFactor(finalSize, desiredSize);

				// Scale the Border by the necessary factor
				Scale.ScaleX = scale.Width;
				Scale.ScaleY = scale.Height;

				// Position the Child to fill the ChildBorder
				Rect originalPosition = new Rect(0, 0, desiredSize.Width, desiredSize.Height);
				ChildBorder.Arrange(originalPosition);

				// Determine the final size used by the Viewbox
				finalSize.Width = scale.Width * desiredSize.Width;
				finalSize.Height = scale.Height * desiredSize.Height;
			}
			return finalSize;
		}

		protected override Size MeasureOverride(Size availableSize)
		{
			// Code in this method has been copied from the implementation
			// in the Silverlight 3 toolkitunder the MS-PL license: http://silverlight.codeplex.com/
			Size size = new Size();
			if (Child != null)
			{
				// Get the child's desired size
				ChildBorder.Measure(new Size(double.PositiveInfinity, double.PositiveInfinity));
				Size desiredSize = ChildBorder.DesiredSize;

				// Determine how much we should scale the child
				Size scale = ComputeScaleFactor(availableSize, desiredSize);

				// Determine the desired size of the Viewbox
				size.Width = scale.Width * desiredSize.Width;
				size.Height = scale.Height * desiredSize.Height;
			}
			return size;
		}

		Size ComputeScaleFactor (Size availableSize, Size contentSize)
		{
			// Code in this method has been copied from the implementation
			// in the Silverlight 3 toolkitunder the MS-PL license: http://silverlight.codeplex.com/
			double scaleX = 1.0;
			double scaleY = 1.0;

			bool isConstrainedWidth = !double.IsPositiveInfinity(availableSize.Width);
			bool isConstrainedHeight = !double.IsPositiveInfinity(availableSize.Height);
			Stretch stretch = Stretch;

			// Don't scale if we shouldn't stretch or the scaleX and scaleY are both infinity.
			if ((stretch != Stretch.None) && (isConstrainedWidth || isConstrainedHeight))
			{
				// Compute the individual scaleX and scaleY scale factors
				scaleX = contentSize.Width == 0 ? 0.0 : (availableSize.Width / contentSize.Width);
				scaleY = contentSize.Height == 0 ? 0.0 : (availableSize.Height / contentSize.Height);

				// Make the scale factors uniform by setting them both equal to
				// the larger or smaller (depending on infinite lengths and the
				// Stretch value)
				if (!isConstrainedWidth)
				{
					scaleX = scaleY;
				}
				else if (!isConstrainedHeight)
				{
					scaleY = scaleX;
				}
				else
				{
					// (isConstrainedWidth && isConstrainedHeight)
					switch (stretch)
					{
					case Stretch.Uniform:
						// Use the smaller factor for both
						scaleX = scaleY = Math.Min(scaleX, scaleY);
						break;
					case Stretch.UniformToFill:
						// Use the larger factor for both
						scaleX = scaleY = Math.Max(scaleX, scaleY);
						break;
					case Stretch.Fill:
					default:
						break;
					}
				}

				// Prevent scaling in an undesired direction
				switch (StretchDirection)
				{
				case StretchDirection.UpOnly:
					scaleX = Math.Max(1.0, scaleX);
					scaleY = Math.Max(1.0, scaleY);
					break;
				case StretchDirection.DownOnly:
					scaleX = Math.Min(1.0, scaleX);
					scaleY = Math.Min(1.0, scaleY);
					break;
				case StretchDirection.Both:
				default:
					break;
				}
			}

			return new Size(scaleX, scaleY);
		}

		internal override UIElement GetDefaultTemplate ()
		{
			return DefaultTemplate;
		}
	}
}

