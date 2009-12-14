// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Diagnostics;
using System.Windows.Media;

namespace System.Windows.Controls
{
    /// <summary>
    /// Represents the control that shows a preview of the GridSplitter's redistribution of space between columns or rows of a Grid control
    /// </summary>
    [TemplatePart(Name = PreviewControl.ElementHorizontalTemplateName, Type = typeof(FrameworkElement))]
    [TemplatePart(Name = PreviewControl.ElementVerticalTemplateName, Type = typeof(FrameworkElement))]
    internal partial class PreviewControl : Control
    {
        #region TemplateParts

        internal const string ElementHorizontalTemplateName = "HorizontalTemplate";
        internal const string ElementVerticalTemplateName = "VerticalTemplate";

        internal FrameworkElement _elementHorizontalTemplateFrameworkElement;
        internal FrameworkElement _elementVerticalTemplateFrameworkElement;

        #endregion

        private GridSplitter.GridResizeDirection _currentGridResizeDirection; // Is Null until the PreviewControl is bound to a GridSplitter
        private Point _gridSplitterOrigin; // Tracks the bound GridSplitter's location for calculating the PreviewControl's offset

        #region Control Instantiation

        /// <summary>
        /// Instantiate the PreviewControl
        /// </summary>
        public PreviewControl()
        {
            _gridSplitterOrigin = new Point();
        }

        /// <summary>
        /// Called when template should be applied to the control
        /// </summary>
        public override void OnApplyTemplate()
        {
            base.OnApplyTemplate();

            _elementHorizontalTemplateFrameworkElement = this.GetTemplateChild(PreviewControl.ElementHorizontalTemplateName) as FrameworkElement;
            _elementVerticalTemplateFrameworkElement = this.GetTemplateChild(PreviewControl.ElementVerticalTemplateName) as FrameworkElement;

            if (_currentGridResizeDirection == GridSplitter.GridResizeDirection.Columns)
            {
                if (_elementHorizontalTemplateFrameworkElement != null)
                {
                    _elementHorizontalTemplateFrameworkElement.Visibility = Visibility.Collapsed;
                }
                if (_elementVerticalTemplateFrameworkElement != null)
                {
                    _elementVerticalTemplateFrameworkElement.Visibility = Visibility.Visible;
                }
            }
            else
            {
                if (_elementHorizontalTemplateFrameworkElement != null)
                {
                    _elementHorizontalTemplateFrameworkElement.Visibility = Visibility.Visible;
                }
                if (_elementVerticalTemplateFrameworkElement != null)
                {
                    _elementVerticalTemplateFrameworkElement.Visibility = Visibility.Collapsed;
                }
            }
        }

        #endregion

        #region Public Members

        /// <summary>
        /// Bind the dimensions of the preview control to the associated grid splitter
        /// </summary>
        /// <param name="gridSplitter">GridSplitter instance to target</param>
        public void Bind(GridSplitter gridSplitter)
        {
            Debug.Assert(gridSplitter != null);
            Debug.Assert(gridSplitter.Parent != null);

            this.Style = gridSplitter.PreviewStyle;
            this.Height = gridSplitter.ActualHeight;
            this.Width = gridSplitter.ActualWidth;

            if (gridSplitter._resizeData != null)
            {
                _currentGridResizeDirection = gridSplitter._resizeData.ResizeDirection;
            }

            Matrix locationMatrix = ((MatrixTransform)gridSplitter.TransformToVisual((UIElement)gridSplitter.Parent)).Matrix;
            _gridSplitterOrigin.X = locationMatrix.OffsetX;
            _gridSplitterOrigin.Y = locationMatrix.OffsetY;

            SetValue(Canvas.LeftProperty, _gridSplitterOrigin.X);
            SetValue(Canvas.TopProperty, _gridSplitterOrigin.Y);
        }

        /// <summary>
        /// Gets or sets the x-axis offset for the underlying render transform
        /// </summary>
        public double OffsetX
        {
            get
            {
                return (double)GetValue(Canvas.LeftProperty) - _gridSplitterOrigin.X;
            }
            set
            {
                SetValue(Canvas.LeftProperty, _gridSplitterOrigin.X + value);
            }
        }

        /// <summary>
        /// Gets or sets the y-axis offset for the underlying render transform
        /// </summary>
        public double OffsetY
        {
            get
            {
                return (double)GetValue(Canvas.TopProperty) - _gridSplitterOrigin.Y;
            }
            set
            {
                SetValue(Canvas.TopProperty, _gridSplitterOrigin.Y + value);
            }
        }

        #endregion
    }
}
