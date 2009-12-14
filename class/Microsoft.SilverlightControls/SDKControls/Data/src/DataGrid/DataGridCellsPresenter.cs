// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Diagnostics;
using System.Windows.Media;

namespace System.Windows.Controls.Primitives
{
    public sealed class DataGridCellsPresenter : Panel
    {
        private double _fillerLeftEdge;

        #region Properties

        private double DesiredHeight
        {
            get;
            set;
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
            if (this.OwningGrid != null)
            {
                double frozenLeftEdge = 0;
                double scrollingLeftEdge = -this.OwningGrid.HorizontalOffset;
                double cellLeftEdge;
                DataGridColumn column = this.OwningGrid.ColumnsInternal.FirstVisibleColumn;
                Debug.Assert(column == null || column.DisplayIndex >= 0);
                while (column != null)
                {
                    DataGridCell cell = this.OwningRow.Cells[column.Index];
                    Debug.Assert(cell.OwningColumn == column);
                    Debug.Assert(column.Visibility == Visibility.Visible);
                    
                    if (column.IsFrozen)
                    {
                        cellLeftEdge = frozenLeftEdge;
                        // This can happen before or after clipping because frozen cells aren't clipped
                        frozenLeftEdge += column.ActualWidth;
                    }
                    else
                    {
                        cellLeftEdge = scrollingLeftEdge;
                    }
                    if (cell.Visibility == Visibility.Visible)
                    {
                        cell.Arrange(new Rect(cellLeftEdge, 0, column.ActualWidth, finalSize.Height));
                        EnsureCellClip(cell, column.ActualWidth, finalSize.Height, frozenLeftEdge, scrollingLeftEdge);
                    }
                    scrollingLeftEdge += column.ActualWidth;

                    column = this.OwningGrid.ColumnsInternal.GetNextVisibleColumn(column);
                }

                _fillerLeftEdge = scrollingLeftEdge;
                // FillerColumn.Width == 0 when the filler column is not active
                this.OwningRow.FillerCell.Arrange(new Rect(_fillerLeftEdge, 0, this.OwningGrid.ColumnsInternal.FillerColumn.Width, finalSize.Height));

                return finalSize;
            }
            return base.ArrangeOverride(finalSize);
        }

        private static void EnsureCellClip(DataGridCell cell, double width, double height, double frozenLeftEdge, double cellLeftEdge)
        {
            // Clip the cell only if it's scrolled under frozen columns.  Unfortunately, we need to clip in this case
            // because cells could be transparent
            if (!cell.OwningColumn.IsFrozen && frozenLeftEdge > cellLeftEdge)
            {
                RectangleGeometry rg = new RectangleGeometry();
                double xClip = Math.Min(width, frozenLeftEdge - cellLeftEdge);
                rg.Rect = new Rect(xClip, 0, width - xClip, height);
                cell.Clip = rg;
            }
            else
            {
                cell.Clip = null;
            }
        }

        private static void EnsureCellDisplay(DataGridCell cell, bool displayColumn)
        {
            if (cell.IsCurrent)
            {
                if (displayColumn)
                {
                    cell.Visibility = Visibility.Visible;
                    cell.Clip = null;
                }
                else
                {
                    // Clip
                    RectangleGeometry rg = new RectangleGeometry();
                    rg.Rect = Rect.Empty;
                    cell.Clip = rg;
                }
            }
            else
            {
                cell.Visibility = displayColumn ? Visibility.Visible : Visibility.Collapsed;
            }
        }

        internal void EnsureFillerVisibility()
        {
            DataGridFillerColumn fillerColumn = this.OwningGrid.ColumnsInternal.FillerColumn;
            Visibility newVisibility = fillerColumn.IsActive ? Visibility.Visible : Visibility.Collapsed;
            if (this.OwningRow.FillerCell.Visibility != newVisibility)
            {
                this.OwningRow.FillerCell.Visibility = newVisibility;
                if (newVisibility == Visibility.Visible)
                {
                    this.OwningRow.FillerCell.Arrange(new Rect(_fillerLeftEdge, 0, fillerColumn.Width, this.ActualHeight));
                }
            }

            // This must be done after the Filler visibility is determined.  This also must be done
            // regardless of whether or not the filler visibility actually changed values because
            // we could scroll in a cell that didn't have EnsureGridLine called yet
            DataGridColumn lastVisibleColumn = this.OwningGrid.ColumnsInternal.LastVisibleColumn;
            if (lastVisibleColumn != null)
            {
                DataGridCell cell = this.OwningRow.Cells[lastVisibleColumn.Index];
                cell.EnsureGridLine(lastVisibleColumn);
            }
        }

        protected override Size MeasureOverride(Size availableSize)
        {
            if (this.OwningGrid == null)
            {
                return base.MeasureOverride(availableSize);
            }
            bool autoSizeHeight;
            double measureHeight;
            if (double.IsNaN(this.OwningGrid.RowHeight))
            {
                // No explicit height values were set so we can autosize
                autoSizeHeight = true;
                measureHeight = double.PositiveInfinity;
            }
            else
            {
                this.DesiredHeight = this.OwningGrid.RowHeight;
                measureHeight = this.DesiredHeight;
                autoSizeHeight = false;
            }

            DataGridColumn lastVisibleColumn = this.OwningGrid.ColumnsInternal.LastVisibleColumn;
            double totalCellsWidth = this.OwningGrid.ColumnsInternal.VisibleEdgedColumnsWidth;
            double measureWidth;

            double frozenLeftEdge = 0;
            double scrollingLeftEdge = -this.OwningGrid.HorizontalOffset;
            DataGridColumn column = this.OwningGrid.ColumnsInternal.FirstVisibleColumn;
            Debug.Assert(column == null || column.DisplayIndex >= 0);
            while (column != null)
            {
                DataGridCell cell = this.OwningRow.Cells[column.Index];
                // 
                bool shouldDisplayCell = ShouldDisplayCell(column, frozenLeftEdge, scrollingLeftEdge) || this.OwningRow.Index == 0;
                EnsureCellDisplay(cell, shouldDisplayCell);
                if (shouldDisplayCell)
                {
                    DataGridLength columnWidth = column.EffectiveWidth;
                    measureWidth = columnWidth.IsAbsolute ? column.ActualWidth : column.ActualMaxWidth;
                    bool autoGrowWidth = columnWidth.IsSizeToCells || columnWidth.IsAuto;
                    cell.Measure(new Size(measureWidth, measureHeight));
                    if (column != lastVisibleColumn)
                    {
                        cell.EnsureGridLine(lastVisibleColumn);
                    }
                    if (autoSizeHeight)
                    {
                        this.DesiredHeight = Math.Max(this.DesiredHeight, cell.DesiredSize.Height);
                    }
                    if (autoGrowWidth && cell.DesiredSize.Width > column.DesiredWidth)
                    {
                        column.DesiredWidth = cell.DesiredSize.Width;
                    }
                }
                if (column.IsFrozen)
                {
                    frozenLeftEdge += column.ActualWidth;
                }
                scrollingLeftEdge += column.ActualWidth;

                column = this.OwningGrid.ColumnsInternal.GetNextVisibleColumn(column);
            }

            // Measure FillerCell, we're doing it unconditionally here because we don't know if we'll need the filler
            // column and we don't want to cause another Measure if we do
            this.OwningRow.FillerCell.Measure(new Size(double.PositiveInfinity, this.DesiredHeight));

            return new Size(totalCellsWidth, this.DesiredHeight);
        }

        private bool ShouldDisplayCell(DataGridColumn column, double frozenLeftEdge, double scrollingLeftEdge)
        {
            Debug.Assert(this.OwningGrid != null);

            if (column.Visibility != Visibility.Visible)
            {
                return false;
            }
            double leftEdge = column.IsFrozen ? frozenLeftEdge : scrollingLeftEdge;
            double rightEdge = leftEdge + column.ActualWidth;
            return DoubleUtil.GreaterThan(rightEdge, 0) &&
                DoubleUtil.LessThanOrClose(leftEdge, this.OwningGrid.CellsWidth) &&
                DoubleUtil.GreaterThan(rightEdge, frozenLeftEdge); // scrolling column covered up by frozen column(s)
        }

        #endregion Methods
    }
}
