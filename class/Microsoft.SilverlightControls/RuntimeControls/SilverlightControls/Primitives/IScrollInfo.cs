// (c) Copyright Microsoft Corporation. 
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved. 


using System; 
using System.Windows.Controls; 

namespace System.Windows.Controls.Primitives 
{
    /// <summary>
    /// An IScrollInfo serves as the main scrolling region inside a <see cref="System.Windows.Controls.ScrollViewer" /> 
    /// or derived class.  It exposes scrolling properties, methods for logical scrolling, computing
    /// which children are visible, and measuring/drawing/offsetting/clipping content.
    /// </summary> 
    public interface IScrollInfo 
    {
 
        #region Public Methods

        /// <summary> 
        /// Set the HorizontalOffset to the passed value.
        /// An implementation may coerce this value into a valid range, typically inclusively between 0 and <see cref="ExtentWidth" /> less <see cref="ViewportWidth" />.
        /// </summary> 
        void SetHorizontalOffset(double offset); 

        /// <summary> 
        /// Set the VerticalOffset to the passed value.
        /// An implementation may coerce this value into a valid range, typically inclusively between 0 and <see cref="ExtentHeight" /> less <see cref="ViewportHeight" />.
        /// </summary> 
        void SetVerticalOffset(double offset);

        Rect MakeVisible (UIElement visual, Rect rectangle);

        #endregion 
 
        #region Public Properties
 
        /// <summary>
        /// This property indicates to the IScrollInfo whether or not it can scroll in the vertical given dimension.
        /// </summary> 
        bool CanVerticallyScroll { get; set; }

        /// <summary> 
        /// This property indicates to the IScrollInfo whether or not it can scroll in the horizontal given dimension. 
        /// </summary>
        bool CanHorizontallyScroll { get; set; } 

        /// <summary>
        /// ExtentWidth contains the full horizontal range of the scrolled content. 
        /// </summary>
        double ExtentWidth { get; }
 
        /// <summary> 
        /// ExtentHeight contains the full vertical range of the scrolled content.
        /// </summary> 
        double ExtentHeight { get; }

        /// <summary> 
        /// ViewportWidth contains the currently visible horizontal range of the scrolled content.
        /// </summary>
        double ViewportWidth { get; } 
 
        /// <summary>
        /// ViewportHeight contains the currently visible vertical range of the scrolled content. 
        /// </summary>
        double ViewportHeight { get; }
 
        /// <summary>
        /// HorizontalOffset is the horizontal offset into the scrolled content that represents the first unit visible.
        /// </summary> 
        double HorizontalOffset { get; } 

        /// <summary> 
        /// VerticalOffset is the vertical offset into the scrolled content that represents the first unit visible.
        /// </summary>
        double VerticalOffset { get; } 

        /// <summary>
        /// ScrollOwner is the container that controls any scrollbars, headers, etc... that are dependant 
        /// on this IScrollInfo's properties.  Implementers of IScrollInfo should call InvalidateScrollInfo() 
        /// on this object when properties change.
        /// </summary> 
        ScrollViewer ScrollOwner { get; set; }

        #endregion 
    }
}
