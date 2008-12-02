// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System.Windows.Markup;
using System.Windows.Media; 
using System.Diagnostics; 

namespace System.Windows.Controls 
{
    /// <summary>
    /// Represents the control that shows a preview of the GridSplitter's redistribution of space between columns or rows of a Grid control 
    /// </summary>
    internal partial class PreviewControl : Control
    { 
        #region Control Instantiation 

        /// <summary> 
        /// Instantiate the PreviewControl
        /// </summary>
        public PreviewControl() 
        {
            // TranslateTransform is required to reposition the preview
            this.RenderTransform = new TranslateTransform(); 
        } 

        #endregion 

        #region Public Members
 
        /// <summary>
        /// Bind the the dimensions of the preview control to the associated grid splitter
        /// </summary> 
        /// <param name="gridSplitter">GridSplitter instance to target</param> 
        public void Bind(GridSplitter gridSplitter)
        { 
            Debug.Assert(gridSplitter != null);
            Debug.Assert(gridSplitter.Parent != null);
 
            this.Style = gridSplitter.PreviewStyle;
            this.Height = gridSplitter.ActualHeight;
            this.Width = gridSplitter.ActualWidth; 
            Matrix locationMatrix = ((MatrixTransform)gridSplitter.TransformToVisual((UIElement)gridSplitter.Parent)).Matrix; 
            SetValue(Canvas.LeftProperty, locationMatrix.OffsetX);
            SetValue(Canvas.TopProperty, locationMatrix.OffsetY); 
        }

        /// <summary> 
        /// Gets or sets the x-axis offset for the underlying render transform
        /// </summary>
        public double OffsetX 
        { 
            get
            { 
                Debug.Assert(this.RenderTransform is TranslateTransform);
                return ((TranslateTransform)this.RenderTransform).X;
            } 
            set
            {
                Debug.Assert(this.RenderTransform is TranslateTransform); 
                ((TranslateTransform)this.RenderTransform).X = value; 
            }
        } 

        /// <summary>
        /// Gets or sets the y-axis offset for the underlying render transform 
        /// </summary>
        public double OffsetY
        { 
            get 
            {
                Debug.Assert(this.RenderTransform is TranslateTransform); 
                return ((TranslateTransform)this.RenderTransform).Y;
            }
            set 
            {
                Debug.Assert(this.RenderTransform is TranslateTransform);
                ((TranslateTransform)this.RenderTransform).Y = value; 
            } 
        }
 
        #endregion
    }
} 
