// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Diagnostics;
using System.Windows.Automation.Peers;
using System.Windows.Media;

namespace System.Windows.Controls.Primitives
{
    public sealed class DataGridColumnHeadersPresenter : Panel
    {
        #region Properties

        internal DataGrid OwningGrid
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
            double frozenLeftEdge = 0;
            double scrollingLeftEdge = -this.OwningGrid.HorizontalOffset;
            DataGridColumn dataGridColumn = this.OwningGrid.ColumnsInternal.FirstVisibleColumn;
            Debug.Assert(dataGridColumn == null || dataGridColumn.DisplayIndex >= 0);
            while (dataGridColumn != null)
            {
                DataGridColumnHeader columnHeader = dataGridColumn.HeaderCell;
                Debug.Assert(columnHeader.OwningColumn == dataGridColumn);
                if (dataGridColumn.IsFrozen)
                {
                    columnHeader.Arrange(new Rect(frozenLeftEdge, 0, dataGridColumn.ActualWidth, finalSize.Height));
                    columnHeader.Clip = null; // The layout system could have clipped this becaues it's not aware of our render transform
                    frozenLeftEdge += dataGridColumn.ActualWidth;
                }
                else
                {
                    columnHeader.Arrange(new Rect(scrollingLeftEdge, 0, dataGridColumn.ActualWidth, finalSize.Height));
                    EnsureColumnHeaderClip(columnHeader, dataGridColumn.ActualWidth, finalSize.Height, frozenLeftEdge, scrollingLeftEdge);
                }
                scrollingLeftEdge += dataGridColumn.ActualWidth;
                
                dataGridColumn = this.OwningGrid.ColumnsInternal.GetNextVisibleColumn(dataGridColumn);
            }

            // Arrange filler
            this.OwningGrid.OnFillerColumnWidthNeeded(finalSize.Width);
            DataGridFillerColumn fillerColumn = this.OwningGrid.ColumnsInternal.FillerColumn;
            if (fillerColumn.Width > 0)
            {
                fillerColumn.HeaderCell.Visibility = Visibility.Visible;
                fillerColumn.HeaderCell.Arrange(new Rect(scrollingLeftEdge, 0, fillerColumn.Width, finalSize.Height));
            }
            else
            {
                fillerColumn.HeaderCell.Visibility = Visibility.Collapsed;
            }

            // This needs to be updated after the filler column is configured
            DataGridColumn lastVisibleColumn = this.OwningGrid.ColumnsInternal.LastVisibleColumn;
            if (lastVisibleColumn != null)
            {
                lastVisibleColumn.HeaderCell.UpdateSeparatorVisibility(lastVisibleColumn);
            }
            return finalSize;
        }

        private static void EnsureColumnHeaderClip(DataGridColumnHeader columnHeader, double width, double height, double frozenLeftEdge, double columnHeaderLeftEdge)
        {
            // Clip the cell only if it's scrolled under frozen columns.  Unfortunately, we need to clip in this case
            // because cells could be transparent
            if (frozenLeftEdge > columnHeaderLeftEdge)
            {
                RectangleGeometry rg = new RectangleGeometry();
                double xClip = Math.Min(width, frozenLeftEdge - columnHeaderLeftEdge);
                rg.Rect = new Rect(xClip, 0, width - xClip, height);
                columnHeader.Clip = rg;
            }
            else
            {
                columnHeader.Clip = null;
            }
        }

        protected override Size MeasureOverride(Size availableSize)
        {
            if (this.OwningGrid == null)
            {
                return base.MeasureOverride(availableSize);
            }
            if (!this.OwningGrid.AreColumnHeadersVisible)
            {
                return Size.Empty;
            }
            double height = this.OwningGrid.ColumnHeaderHeight;
            bool autoSizeHeight;
            if (double.IsNaN(height))
            {
                // No explicit height values were set so we can autosize
                height = 0;
                autoSizeHeight = true;
            }
            else
            {
                autoSizeHeight = false;
            }

            DataGridColumn lastVisibleColumn = this.OwningGrid.ColumnsInternal.LastVisibleColumn;
            DataGridColumn column = this.OwningGrid.ColumnsInternal.FirstVisibleColumn;
            double measureWidth;
            while (column != null)
            {
                DataGridLength columnWidth = column.EffectiveWidth;
                measureWidth = columnWidth.IsAbsolute ? column.ActualWidth : column.ActualMaxWidth;
                bool autoGrowWidth = columnWidth.IsAuto || columnWidth.IsSizeToHeader;

                DataGridColumnHeader columnHeader = column.HeaderCell;
                if (column != lastVisibleColumn)
                {
                    columnHeader.UpdateSeparatorVisibility(lastVisibleColumn);
                }
                columnHeader.Measure(new Size(measureWidth, double.PositiveInfinity));
                if (autoSizeHeight)
                {
                    height = Math.Max(height, columnHeader.DesiredSize.Height);
                }
                if (autoGrowWidth && columnHeader.DesiredSize.Width > column.DesiredWidth)
                {
                    column.DesiredWidth = columnHeader.DesiredSize.Width;
                }

                column = this.OwningGrid.ColumnsInternal.GetNextVisibleColumn(column);
            }

            // Add the filler column if it's not represented.  We won't know whether we need it or not until Arrange
            DataGridFillerColumn fillerColumn = this.OwningGrid.ColumnsInternal.FillerColumn;
            if (!fillerColumn.IsRepresented)
            {
                Debug.Assert(!this.Children.Contains(fillerColumn.HeaderCell));
                fillerColumn.HeaderCell.SeparatorVisibility = Visibility.Collapsed;
                this.Children.Insert(this.OwningGrid.ColumnsInternal.Count, fillerColumn.HeaderCell);
                fillerColumn.IsRepresented = true;
                this.OwningGrid.OnFillerColumnRepresentationChanged();
                // Optimize for the case where we don't need the filler cell 
                fillerColumn.HeaderCell.Visibility = Visibility.Collapsed;
            }
            fillerColumn.HeaderCell.Measure(new Size(double.PositiveInfinity, double.PositiveInfinity));

            return new Size(this.OwningGrid.ColumnsInternal.VisibleEdgedColumnsWidth, height);
        }

        /// <summary>
        /// Creates AutomationPeer (<see cref="UIElement.OnCreateAutomationPeer"/>)
        /// </summary>
        protected override AutomationPeer OnCreateAutomationPeer()
        {
            return new DataGridColumnHeadersPresenterAutomationPeer(this);
        }

        #endregion Methods
    }
}
