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
using System.Windows.Controls.Primitives;

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
        RectangleGeometry _clippingRectangle;
        Size extents;
        double _horizontalOffset; 
        double _verticalOffset;
        Size viewport;

        RectangleGeometry ClippingRectangle {
            get {
                if (_clippingRectangle == null) {
                    _clippingRectangle = new RectangleGeometry ();
                    Clip = _clippingRectangle;
                }
                return _clippingRectangle;
            }
        }

        public ScrollViewer ScrollOwner { get; set; }
 
        public bool CanHorizontallyScroll { get; set; }

        public bool CanVerticallyScroll { get; set; }

        public double HorizontalOffset
        {
            get { return _horizontalOffset; } 
        }

        public void SetHorizontalOffset (double offset)
        {
            if (_horizontalOffset != offset)
                InvalidateArrange();
            _horizontalOffset = offset;
        }

        public double VerticalOffset
        { 
            get { return _verticalOffset; } 
        } 

        public void SetVerticalOffset (double offset)
        {
            if (_verticalOffset != offset)
                InvalidateArrange();

            _verticalOffset = offset;
        }

        public double ExtentWidth { 
            get { return extents.Width; }
        } 

        public double ExtentHeight {
            get { return extents.Height; }
        }

        public double ViewportWidth {
            get { return viewport.Width; }
        } 

        public double ViewportHeight {
            get { return viewport.Height; }
        } 

        public ScrollContentPresenter()
        {
            
        }

        protected override Size MeasureOverride(Size availableSize) 
        { 
            if (null == ScrollOwner || _contentRoot == null)
                return base.MeasureOverride(availableSize);
 
            Size ideal = new Size (
                CanHorizontallyScroll ? double.PositiveInfinity : availableSize.Width,
                CanVerticallyScroll ? double.PositiveInfinity : availableSize.Height
            );

            _contentRoot.Measure (ideal);
            UpdateExtents (availableSize, _contentRoot.DesiredSize);
            return availableSize.Min (extents);
        } 

        protected override Size ArrangeOverride(Size finalSize)
        { 
            if (null == ScrollOwner || _contentRoot == null) 
                return base.ArrangeOverride(finalSize); 

            Size desired = _contentRoot.DesiredSize;
            Point start = new Point (
                -Math.Max (0, Math.Min (HorizontalOffset, ExtentWidth - ViewportWidth)),
                -Math.Max (0, Math.Min (VerticalOffset, ExtentHeight - ViewportHeight))
            );

            _contentRoot.Arrange(new Rect (start, desired.Max (finalSize))); 
            ClippingRectangle.Rect = new Rect (new Point (0, 0), finalSize); 
            UpdateExtents (finalSize, extents);
            return finalSize;
        } 
        
        void UpdateExtents (Size viewport, Size extents)
        {
            bool changed = this.viewport != viewport || this.extents != extents;
            this.viewport = viewport;
            this.extents = extents;
            if (changed)
                ScrollOwner.InvalidateScrollInfo ();
        }
    }
}
