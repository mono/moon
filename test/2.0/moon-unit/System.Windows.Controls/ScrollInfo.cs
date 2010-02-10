using System;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Windows.Controls.Primitives;

namespace MoonTest.System.Windows.Controls
{
	public class ScrollInfo : Panel, IScrollInfo
	{
		public bool CalledCanHorizontallyScroll { get; set; }
		public bool CalledCanVerticallyScroll { get; set; }
		bool canHorizontallyScroll;
		public bool CanHorizontallyScroll
		{
			get { CalledCanHorizontallyScroll = true; return canHorizontallyScroll; }
			set { CalledCanHorizontallyScroll = true; canHorizontallyScroll = value; }
		}

		bool canVerticallyScroll;
		public bool CanVerticallyScroll
		{
			get { CalledCanVerticallyScroll = true; return canVerticallyScroll; }
			set { CalledCanVerticallyScroll = true; canVerticallyScroll = value; }
		}

		double extentHeight;
		public double ExtentHeight
		{
			get { return extentHeight; }
			set { extentHeight = value; }
		}

		double extentWidth;
		public double ExtentWidth
		{
			get { return extentWidth; }
			set { extentWidth = value; }
		}

		double horizontalOffset;
		public double HorizontalOffset
		{
			get { return horizontalOffset; }
			set { horizontalOffset = value; }
		}

		ScrollViewer scrollOwner;
		public ScrollViewer ScrollOwner
		{
			get { return scrollOwner; }
			set { scrollOwner = value; }
		}

		double verticalOffset;
		public double VerticalOffset
		{
			get { return verticalOffset; }
			set { verticalOffset = value; }
		}

		double viewportHeight;
		public double ViewportHeight
		{
			get { return viewportHeight; }
			set { viewportHeight = value; }
		}

		double viewportWidth;
		public double ViewportWidth
		{
			get { return viewportWidth; }
			set { viewportWidth = value; }
		}


		public ScrollInfo ()
		{
			Children.Add (new Rectangle { Width = 1000, Height = 1000, Fill = new SolidColorBrush (Colors.Red) });
		}

		protected override Size MeasureOverride (Size availableSize)
		{
			Children [0].Measure (availableSize);
			ScrollOwner.InvalidateScrollInfo ();
			return Children [0].DesiredSize;
		}

		protected override Size ArrangeOverride (Size finalSize)
		{
			Children [0].Arrange (new Rect (0, 0, finalSize.Width, finalSize.Height));
			return finalSize;
		}

		public void LineDown ()
		{
			throw new NotImplementedException ();
		}

		public void LineLeft ()
		{
			throw new NotImplementedException ();
		}

		public void LineRight ()
		{
			throw new NotImplementedException ();
		}

		public void LineUp ()
		{
			throw new NotImplementedException ();
		}

		public Rect MakeVisible (UIElement visual, Rect rectangle)
		{
			throw new NotImplementedException ();
		}

		public void MouseWheelDown ()
		{
			throw new NotImplementedException ();
		}

		public void MouseWheelLeft ()
		{
			throw new NotImplementedException ();
		}

		public void MouseWheelRight ()
		{
			throw new NotImplementedException ();
		}

		public void MouseWheelUp ()
		{
			throw new NotImplementedException ();
		}

		public void PageDown ()
		{
			throw new NotImplementedException ();
		}

		public void PageLeft ()
		{
			throw new NotImplementedException ();
		}

		public void PageRight ()
		{
			throw new NotImplementedException ();
		}

		public void PageUp ()
		{
			throw new NotImplementedException ();
		}

		public void SetHorizontalOffset (double offset)
		{
			throw new NotImplementedException ();
		}

		public void SetVerticalOffset (double offset)
		{
			throw new NotImplementedException ();
		}

	}
}
