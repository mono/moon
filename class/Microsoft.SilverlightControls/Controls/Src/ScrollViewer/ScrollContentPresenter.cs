// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System;
using System.Diagnostics; 
using System.Windows; 
using System.Windows.Controls;
using System.Windows.Input; 
using System.Windows.Media;

#if WPF 
namespace WPF
#else
namespace System.Windows.Controls
#endif 
{
    /// <summary> 
    /// Displays the content of a ScrollViewer control.
    /// </summary>
    public sealed class ScrollContentPresenter : ContentPresenter, IScrollInfo 
    {
        /// <summary>
        /// Reference to the ScrollViewer parent control. 
        /// </summary> 
        public ScrollViewer ScrollOwner { get; set; }
 
	public bool CanHorizontallyScroll { get; set; }

	public bool CanVerticallyScroll { get; set; }
        /// <summary>
        /// Gets the horizontal offset of the scrolled content.
        /// </summary> 
        public double HorizontalOffset
        {
            get { return _horizontalOffset; } 
        }
        private double _horizontalOffset; 

	public void SetHorizontalOffset (double offset)
	{
		if (_horizontalOffset != offset)
				InvalidateArrange();
		_horizontalOffset = offset;
	}

        /// <summary>
        /// Gets the vertical offset of the scrolled content. 
        /// </summary>
        public double VerticalOffset
        { 
            get { return _verticalOffset; } 
        } 
        private double _verticalOffset;

	public void SetVerticalOffset (double offset)
	{
		if (_verticalOffset != offset)
			InvalidateArrange();

		_verticalOffset = offset;
	}

        /// <summary> 
        /// Gets the horizontal size of the extent.
        /// </summary>
        public double ExtentWidth { get; private set; } 
 
        /// <summary>
        /// Gets the vertical size of the extent. 
        /// </summary>
        public double ExtentHeight { get; private set; }
 
        /// <summary>
        /// Gets the horizontal size of the viewport for this content.
        /// </summary> 
        public double ViewportWidth { get; private set; } 

        /// <summary> 
        /// Gets the vertical size of the viewport for this content.
        /// </summary>
        public double ViewportHeight { get; private set; } 

        /// <summary>
        /// Clipping rectangle used to replace WPF's GetLayoutClip virtual 
        /// </summary> 
        private RectangleGeometry _clippingRectangle;
 
        /// <summary>
        /// Initializes a new instance of the ScrollContentPresenter class.
        /// </summary> 
        public ScrollContentPresenter()
        {
            _clippingRectangle = new RectangleGeometry(); 
            Clip = _clippingRectangle; 
        }
 
        /// <summary>
        /// Called to remeasure a control.
        /// </summary> 
        /// <param name="availableSize">Measurement constraints, a control cannot return a size larger than the constraint.</param>
        /// <returns>The size of the control.</returns>
        protected override Size MeasureOverride(Size availableSize) 
        { 
            if (null == ScrollOwner)
            { 
                return base.MeasureOverride(availableSize);
            }
 
            Size ideal = new Size(
                ScrollBarVisibility.Disabled != ScrollOwner.HorizontalScrollBarVisibility ? double.PositiveInfinity : availableSize.Width,
                ScrollBarVisibility.Disabled != ScrollOwner.VerticalScrollBarVisibility ? double.PositiveInfinity : availableSize.Height); 
            Size ExtentSize = base.MeasureOverride(ideal); 
            UpdateExtents (new Size (Math.Min(availableSize.Width, ExtentSize.Width),
                                     Math.Min(availableSize.Height, ExtentSize.Height)),
                           ExtentSize);
            SetHorizontalOffset (Math.Max (HorizontalOffset, 0)); 
            SetVerticalOffset (Math.Max (VerticalOffset, 0));
            ScrollOwner.UpdateFromChild ();
            return new Size(ViewportWidth, ViewportHeight); 
        } 

        /// <summary> 
        /// Called to arrange and size the content of a Control object.
        /// </summary>
        /// <param name="finalSize">The computed size that is used to arrange the content.</param> 
        /// <returns>The size of the control.</returns>
        protected override Size ArrangeOverride(Size finalSize)
        { 
            if (null == ScrollOwner || _contentRoot == null) 
            {
                return base.ArrangeOverride(finalSize); 
            }

            UIElement child = 
#if WPF
                GetVisualChild(0) as UIElement;
#else 
                // The base class implementation of ContentPresenter includes an 
                // additional element above what it returns for GetVisualChild. When
                // doing the work for ArrangeOverride, that additional element must be 
                // used instead.
                _contentRoot;
#endif 
            Debug.Assert(null != child);
            Rect desired = new Rect(
                0, 
                0, 
                child.DesiredSize.Width,
                child.DesiredSize.Height); 
            Rect arranged = new Rect(
                desired.X - Math.Max (0, Math.Min(HorizontalOffset, ExtentWidth - ViewportWidth)),
                desired.Y - Math.Max (0, Math.Min(VerticalOffset, ExtentHeight - ViewportHeight)), 
                Math.Max(desired.Width, finalSize.Width),
                Math.Max(desired.Height, finalSize.Height));
            child.Arrange(arranged); 
            _clippingRectangle.Rect = new Rect(0, 0, finalSize.Width, finalSize.Height); 
	    //FIXME isn't this call excessive, layout should pass the info up
	    //trough normal channels already
	    UpdateExtents (new Size (finalSize.Width, finalSize.Height), new Size (ExtentWidth, ExtentHeight));
            return finalSize;
        } 
        
        void UpdateExtents (Size viewport, Size extents)
        {
            ViewportWidth = viewport.Width;
            ViewportHeight = viewport.Height;
            ExtentWidth = extents.Width;
            ExtentHeight = extents.Height;

            ScrollOwner.UpdateFromChild ();
        }
    }
}
