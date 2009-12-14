// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Diagnostics;
using System.Windows.Automation.Peers;

namespace System.Windows.Controls.Primitives
{
    public sealed class DataGridRowsPresenter : Panel
    {
        #region Properties

        internal DataGrid OwningGrid
        {
            get;
            set;
        }

        #endregion

        #region Methods

        protected override Size ArrangeOverride(Size finalSize)
        {
            if (finalSize.Height == 0 || this.OwningGrid == null)
            {
                return base.ArrangeOverride(finalSize);
            }

            this.OwningGrid.OnFillerColumnWidthNeeded(finalSize.Width);

            double rowDesiredWidth = this.OwningGrid.ColumnsInternal.VisibleEdgedColumnsWidth + this.OwningGrid.ColumnsInternal.FillerColumn.Width;
            double topEdge = -this.OwningGrid.NegVerticalOffset;
            foreach (DataGridRow row in this.OwningGrid.DisplayData.GetScrollingRows())
            {
                row.Arrange(new Rect(-this.OwningGrid.HorizontalOffset, topEdge, rowDesiredWidth, row.DesiredSize.Height));

                
                
                row.EnsureFillerVisibility();

                Debug.Assert(row.Index != -1); // A displayed row should always have its index
                topEdge += row.DesiredSize.Height;
            }
            
            return new Size(finalSize.Width, Math.Max(topEdge + this.OwningGrid.NegVerticalOffset, finalSize.Height));
        }

        protected override Size MeasureOverride(Size availableSize)
        {
            if (availableSize.Height == 0 || this.OwningGrid == null)
            {
                return base.MeasureOverride(availableSize);
            }

            // The DataGrid uses the RowsPresenter available size in order to autogrow
            // and calculate the scrollbars
            this.OwningGrid.RowsPresenterAvailableSize = availableSize;

            this.OwningGrid.OnRowsMeasure();

            double totalHeight = -this.OwningGrid.NegVerticalOffset;
            double totalCellsWidth = this.OwningGrid.ColumnsInternal.VisibleEdgedColumnsWidth;

            double headerWidth = 0;
            foreach (DataGridRow row in this.OwningGrid.DisplayData.GetScrollingRows())
            {
                row.Measure(new Size(double.PositiveInfinity, double.PositiveInfinity));
                if (row.HeaderCell != null)
                {
                    headerWidth = Math.Max(headerWidth, row.HeaderCell.DesiredSize.Width);
                }
                totalHeight += row.DesiredSize.Height;
            }

            this.OwningGrid.RowHeadersDesiredWidth = headerWidth;
            // Could be positive infinity depending on the DataGrid's bounds
            this.OwningGrid.AvailableRowRoom = availableSize.Height - totalHeight;

            // 

            totalHeight = Math.Max(0, totalHeight);

            return new Size(totalCellsWidth + headerWidth, totalHeight);
        }

        /// <summary>
        /// Creates AutomationPeer (<see cref="UIElement.OnCreateAutomationPeer"/>)
        /// </summary>
        protected override AutomationPeer OnCreateAutomationPeer()
        {
            return new DataGridRowsPresenterAutomationPeer(this);
        }

        #endregion Methods
    }
}
