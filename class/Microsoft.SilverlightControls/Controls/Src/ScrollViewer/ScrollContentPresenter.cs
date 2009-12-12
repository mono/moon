// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System;
using System.Diagnostics; 
using System.Windows; 
using System.Windows.Controls.Primitives;
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
        RectangleGeometry _clippingRectangle;
        Point cachedOffset;
        Size extents;
        Size viewport;

        public ScrollViewer ScrollOwner { get; set; }
 
        public bool CanHorizontallyScroll { get; set; }

        public bool CanVerticallyScroll { get; set; }

        public double HorizontalOffset
        {
            get; private set;
        }

        public void SetHorizontalOffset (double offset)
        {
           if (!CanHorizontallyScroll || cachedOffset.X == offset)
                return;

            InvalidateArrange();
            cachedOffset.X = offset;
        }

        public double VerticalOffset
        { 
            get; private set;
        } 

        public void SetVerticalOffset (double offset)
        {
            if (!CanVerticallyScroll || cachedOffset.Y == offset)
                return;

            InvalidateArrange();
            cachedOffset.Y = offset;
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
            _clippingRectangle = new RectangleGeometry(); 
            Clip = _clippingRectangle; 
        }

        void ClampOffsets ()
        {
            double result = CanHorizontallyScroll ? Math.Min (cachedOffset.X, ExtentWidth - ViewportWidth) : 0;
            HorizontalOffset = Math.Max (0, result);

            result = CanVerticallyScroll ? Math.Min (cachedOffset.Y, ExtentHeight - ViewportHeight) : 0;
            VerticalOffset = Math.Max (0, result);
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
            ClampOffsets ();
            return availableSize.Min (extents);
        } 

        protected override Size ArrangeOverride(Size finalSize)
        { 
            if (null == ScrollOwner || _contentRoot == null) 
                return base.ArrangeOverride(finalSize); 

            ClampOffsets ();
            Size desired = _contentRoot.DesiredSize;
            Point start = new Point (
                -HorizontalOffset,
                -VerticalOffset
            );

            _contentRoot.Arrange(new Rect (start, desired.Max (finalSize))); 
            _clippingRectangle.Rect = new Rect (new Point (0, 0), finalSize); 
            UpdateExtents (finalSize, extents);
            return finalSize;
        } 
        
        void UpdateExtents (Size viewport, Size extents)
        {
            bool changed = this.viewport != viewport || this.extents != extents;
            this.viewport = viewport;
            this.extents = extents;
            ClampOffsets ();
            if (changed)
                ScrollOwner.InvalidateScrollInfo ();
        }

        [MonoTODO]
        public Rect MakeVisible (UIElement visual, Rect rectangle)
        {
            throw new NotImplementedException ();
        }
    }
}
