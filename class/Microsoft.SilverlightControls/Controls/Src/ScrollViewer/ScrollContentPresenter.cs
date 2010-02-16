/* -*- Mode: C#; default-tab-width: 4; tab-width: 4; indent-tabs-mode: nil; c-basic-indent: 4; c-basic-offset: 4 -*- */
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
        static readonly double LineDelta = 16.0;

        RectangleGeometry _clippingRectangle;
        Point cachedOffset;
        Size viewport;
        Size extents;

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
            get; private set;
        }

        public void SetHorizontalOffset (double offset)
        {
            if (!CanHorizontallyScroll || cachedOffset.X == offset)
                return;

            cachedOffset.X = offset;
            InvalidateArrange();
        }

        public double VerticalOffset
        { 
            get; private set;
        } 

        public void SetVerticalOffset (double offset)
        {
            if (!CanVerticallyScroll || cachedOffset.Y == offset)
                return;

            cachedOffset.Y = offset;
            InvalidateArrange();
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

        void ClampOffsets ()
        {
            double result = CanHorizontallyScroll ? Math.Min (cachedOffset.X, ExtentWidth - ViewportWidth) : 0;
            HorizontalOffset = Math.Max (0, result);

            result = CanVerticallyScroll ? Math.Min (cachedOffset.Y, ExtentHeight - ViewportHeight) : 0;
            VerticalOffset = Math.Max (0, result);
        }

        public override void OnApplyTemplate ()
        {
            base.OnApplyTemplate ();
            ScrollViewer sv = TemplateOwner as ScrollViewer;
            if (sv == null)
                return;

            IScrollInfo info = Content as IScrollInfo;
            if (info == null) {
                var presenter = Content as ItemsPresenter;
                if (presenter != null) {
                    if (presenter._elementRoot == null) {
                        presenter.ApplyTemplate ();
                    }
                    info = presenter._elementRoot as IScrollInfo;
                }
            }
            info = info ?? this;
            info.CanHorizontallyScroll = sv.HorizontalScrollBarVisibility != ScrollBarVisibility.Disabled;
            info.CanVerticallyScroll = sv.VerticalScrollBarVisibility != ScrollBarVisibility.Disabled;
            info.ScrollOwner = sv;
            info.ScrollOwner.ScrollInfo = info;
            sv.InvalidateScrollInfo ();
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

            ClampOffsets ();
            Size desired = _contentRoot.DesiredSize;
            Point start = new Point (
                -HorizontalOffset,
                -VerticalOffset
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
            ClampOffsets ();
            if (changed)
                ScrollOwner.InvalidateScrollInfo ();
        }

        public void LineDown ()
        {
            SetVerticalOffset (VerticalOffset + LineDelta);
        }

        public void LineLeft ()
        {
            SetHorizontalOffset (HorizontalOffset - LineDelta);
        }

        public void LineRight ()
        {
            SetHorizontalOffset (HorizontalOffset + LineDelta);
        }

        public void LineUp ()
        {
            SetVerticalOffset (VerticalOffset - LineDelta);
        }
        
        // FIXME: how does one invoke MouseWheelUp/Down/etc? Need to figure out proper scrolling amounts
        public void MouseWheelDown ()
        {
            SetVerticalOffset (VerticalOffset + LineDelta);
        }

        public void MouseWheelLeft ()
        {
            SetHorizontalOffset (HorizontalOffset - LineDelta);
        }

        public void MouseWheelRight ()
        {
            SetHorizontalOffset (HorizontalOffset + LineDelta);
        }

        public void MouseWheelUp ()
        {
            SetVerticalOffset (VerticalOffset - LineDelta);
        }

        public void PageDown ()
        {
            SetVerticalOffset (VerticalOffset + ViewportHeight);
        }

        public void PageLeft ()
        {
            SetHorizontalOffset (HorizontalOffset - ViewportWidth);
        }

        public void PageRight ()
        {
            SetHorizontalOffset (HorizontalOffset + ViewportWidth);
        }

        public void PageUp ()
        {
            SetVerticalOffset (VerticalOffset - ViewportHeight);
        }

        [MonoTODO]
        public Rect MakeVisible (UIElement visual, Rect rectangle)
        {
            throw new NotImplementedException ();
        }
    }
}
