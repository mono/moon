// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Windows.Automation.Peers;
using System.Windows.Media;

namespace System.Windows.Controls.Primitives
{
    public sealed class DataGridDetailsPresenter : Panel
    {
        #region Properties

        public double ContentHeight
        {
            get { return (double)GetValue(ContentHeightProperty); }
            set { SetValue(ContentHeightProperty, value); }
        }

        /// <summary>
        /// Identifies the ContentHeight dependency property.
        /// </summary>
        public static readonly DependencyProperty ContentHeightProperty =
            DependencyProperty.Register(
                "ContentHeight",
                typeof(double),
                typeof(DataGridDetailsPresenter),
                new PropertyMetadata(OnContentHeightPropertyChanged));

        /// <summary>
        /// ContentHeightProperty property changed handler.
        /// </summary>
        /// <param name="d">DataGridDetailsPresenter.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnContentHeightPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DataGridDetailsPresenter detailsPresenter = (DataGridDetailsPresenter)d;
            detailsPresenter.InvalidateMeasure();
        }

        private DataGrid OwningGrid
        {
            get
            {
                if (this.OwningRow != null)
                {
                    return this.OwningRow.OwningGrid;
                }
                return null;
            }
        }

        internal DataGridRow OwningRow
        {
            get;
            set;
        }

        #endregion Properties

        #region Methods

        protected override Size ArrangeOverride(Size finalSize)
        {
            if (this.OwningGrid == null)
            {
                return base.ArrangeOverride(finalSize);
            }
            double leftEdge = 0;
            double xClip = 0;
            double width;
            if (this.OwningGrid.AreRowDetailsFrozen)
            {
                leftEdge = this.OwningGrid.HorizontalOffset;
                width = this.OwningGrid.CellsWidth;
            }
            else
            {
                xClip = this.OwningGrid.HorizontalOffset;
                width = Math.Max(this.OwningGrid.CellsWidth, this.OwningGrid.ColumnsInternal.VisibleEdgedColumnsWidth);
            }
            double height = double.IsNaN(this.ContentHeight) ? 0 : this.ContentHeight;

            foreach (UIElement child in this.Children)
            {
                child.Arrange(new Rect(leftEdge, 0, width, height));
            }

            if (this.OwningGrid.AreRowDetailsFrozen)
            {
                // Frozen Details should not be clipped, similar to frozen cells
                this.Clip = null;
            }
            else
            {
                // Clip so Details doesn't obstruct elements to the left (the RowHeader by default) as we scroll to the right
                RectangleGeometry rg = new RectangleGeometry();
                rg.Rect = new Rect(xClip, 0, width - xClip, height);
                this.Clip = rg;
            }

            return finalSize;
        }

        protected override Size MeasureOverride(Size availableSize)
        {
            if (this.OwningGrid == null || this.Children.Count == 0)
            {
                return Size.Empty;
            }

            double desiredWidth = this.OwningGrid.AreRowDetailsFrozen ?
                this.OwningGrid.CellsWidth :
                Math.Max(this.OwningGrid.CellsWidth, this.OwningGrid.ColumnsInternal.VisibleEdgedColumnsWidth);

            foreach (UIElement child in this.Children)
            {
                child.Measure(new Size(desiredWidth, double.PositiveInfinity));
            }
            
            double desiredHeight = double.IsNaN(this.ContentHeight) ? 0 : this.ContentHeight;

            return new Size(desiredWidth, desiredHeight);
        }

        /// <summary>
        /// Creates AutomationPeer (<see cref="UIElement.OnCreateAutomationPeer"/>)
        /// </summary>
        protected override AutomationPeer OnCreateAutomationPeer()
        {
            return new DataGridDetailsPresenterAutomationPeer(this);
        }

        #endregion Methods
    }
}
