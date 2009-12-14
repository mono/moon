// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Diagnostics;
using System.Windows.Automation.Peers;
using System.Windows.Input;
using System.Windows.Shapes;

namespace System.Windows.Controls
{
    [TemplatePart(Name = DataGridCell.DATAGRIDCELL_elementRightGridLine, Type = typeof(Rectangle))]

    [TemplateVisualState(Name = VisualStates.StateNormal, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = VisualStates.StateMouseOver, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = VisualStates.StateUnselected, GroupName = VisualStates.GroupSelection)]
    [TemplateVisualState(Name = VisualStates.StateSelected, GroupName = VisualStates.GroupSelection)]
    [TemplateVisualState(Name = VisualStates.StateUnfocused, GroupName = VisualStates.GroupFocus)]
    [TemplateVisualState(Name = VisualStates.StateFocused, GroupName = VisualStates.GroupFocus)]
    [TemplateVisualState(Name = VisualStates.StateRegular, GroupName = VisualStates.GroupCurrent)]
    [TemplateVisualState(Name = VisualStates.StateCurrent, GroupName = VisualStates.GroupCurrent)]
    [TemplateVisualState(Name = VisualStates.StateDisplay, GroupName = VisualStates.GroupInteraction)]
    [TemplateVisualState(Name = VisualStates.StateEditing, GroupName = VisualStates.GroupInteraction)]
    public sealed partial class DataGridCell : ContentControl
    {
        #region Constants

        private const string DATAGRIDCELL_elementRightGridLine = "RightGridLine";

        #endregion Constants

        #region Data

        private Rectangle _rightGridLine;

        #endregion Data

        public DataGridCell()
        {
            this.MouseLeftButtonDown += new MouseButtonEventHandler(DataGridCell_MouseLeftButtonDown);
            this.MouseEnter += new MouseEventHandler(DataGridCell_MouseEnter);
            this.MouseLeave += new MouseEventHandler(DataGridCell_MouseLeave);

            DefaultStyleKey = typeof(DataGridCell);
        }

        #region Public Properties

        #endregion Public Properties


        #region Protected Properties

        #endregion Protected Properties


        #region Internal Properties

        internal double ActualRightGridLineWidth
        {
            get
            {
                if (_rightGridLine != null)
                {
                    return _rightGridLine.ActualWidth;
                }
                return 0;
            }
        }

        internal int ColumnIndex
        {
            get
            {
                if (this.OwningColumn == null)
                {
                    return -1;
                }
                return this.OwningColumn.Index;
            }
        }

        internal bool IsCurrent
        {
            get
            {
                Debug.Assert(this.OwningGrid != null && this.OwningColumn != null && this.OwningRow != null);
                return this.OwningGrid.CurrentColumnIndex == this.OwningColumn.Index &&
                       this.OwningGrid.CurrentRowIndex == this.OwningRow.Index;
            }
        }

        internal DataGridColumn OwningColumn
        {
            get;
            set;
        }

        internal DataGrid OwningGrid
        {
            get
            {
                if (this.OwningRow != null && this.OwningRow.OwningGrid != null)
                {
                    return this.OwningRow.OwningGrid;
                }
                if (this.OwningColumn != null)
                {
                    return this.OwningColumn.OwningGrid;
                }
                return null;
            }
        }

        internal DataGridRow OwningRow
        {
            get;
            set;
        }

        internal int RowIndex
        {
            get
            {
                if (this.OwningRow == null)
                {
                    return -1;
                }
                return this.OwningRow.Index;
            }
        }

        #endregion Internal Properties


        #region Private Properties

        private bool IsEdited
        {
            get
            {
                Debug.Assert(this.OwningGrid != null);
                return this.OwningGrid.EditingRowIndex == this.RowIndex &&
                       this.OwningGrid.EditingColumnIndex == this.ColumnIndex;
            }
        }

        private bool IsMouseOver
        {
            get
            {
                return this.OwningRow != null && this.OwningRow.MouseOverColumnIndex == this.ColumnIndex;
            }
            set
            {
                Debug.Assert(this.OwningRow != null);
                if (value != this.IsMouseOver)
                {
                    if (value)
                    {
                        this.OwningRow.MouseOverColumnIndex = this.ColumnIndex;
                    }
                    else
                    {
                        this.OwningRow.MouseOverColumnIndex = null;
                    }
                }
            }
        }

        #endregion Private Properties


        #region Public Methods

        #endregion Public Methods


        #region Protected Methods

        public override void OnApplyTemplate()
        {
            base.OnApplyTemplate();

            ApplyCellState(false /*animate*/);
            
            this._rightGridLine = GetTemplateChild(DATAGRIDCELL_elementRightGridLine) as Rectangle;
            if (_rightGridLine != null && this.OwningColumn == null)
            {
                // Turn off the right GridLine for filler cells
                _rightGridLine.Visibility = Visibility.Collapsed;
            }
            else
            {
                EnsureGridLine(null);
            }
        }

        /// <summary>
        /// Creates AutomationPeer (<see cref="UIElement.OnCreateAutomationPeer"/>)
        /// </summary>
        protected override AutomationPeer OnCreateAutomationPeer()
        {
            if (this.OwningGrid != null &&
                this.OwningColumn != null &&
                this.OwningColumn != this.OwningGrid.ColumnsInternal.FillerColumn)
            {
                return new DataGridCellAutomationPeer(this);
            }
            return base.OnCreateAutomationPeer();
        }

        #endregion Protected Methods

        
        #region Internal Methods

        internal void ApplyCellState(bool animate)
        {
            if (this.OwningGrid == null || this.OwningColumn == null || this.OwningRow == null)
            {
                return;
            }

            // CommonStates
            if (this.IsMouseOver)
            {
                VisualStates.GoToState(this, animate, VisualStates.StateMouseOver, VisualStates.StateNormal);
            }
            else
            {
                VisualStates.GoToState(this, animate, VisualStates.StateNormal);
            }

            // SelectionStates
            if (this.OwningRow.IsSelected)
            {
                VisualStates.GoToState(this, animate, VisualStates.StateSelected, VisualStates.StateUnselected);
            }
            else
            {
                VisualStates.GoToState(this, animate, VisualStates.StateUnselected);
            }

            // FocusStates
            if (this.OwningGrid.ContainsFocus)
            {
                VisualStates.GoToState(this, animate, VisualStates.StateFocused, VisualStates.StateUnfocused);
            }
            else
            {
                VisualStates.GoToState(this, animate, VisualStates.StateUnfocused);
            }

            // CurrentStates
            if (this.IsCurrent)
            {
                VisualStates.GoToState(this, animate, VisualStates.StateCurrent, VisualStates.StateRegular);
            }
            else
            {
                VisualStates.GoToState(this, animate, VisualStates.StateRegular);
            }

            // Interaction states
            if (this.IsEdited)
            {
                VisualStates.GoToState(this, animate, VisualStates.StateEditing, VisualStates.StateDisplay);
            }
            else
            {
                VisualStates.GoToState(this, animate, VisualStates.StateDisplay);
            }
        }

        internal void EnsureCellStyle()
        {
            if (this.OwningColumn != null && this.OwningColumn.CellStyle != null)
            {
                // Set the cell's style if there is one defined for the owning column
                if (this.Style != this.OwningColumn.CellStyle)
                {
                    this.Style = this.OwningColumn.CellStyle;
                }
            }
            else if (this.OwningGrid != null && this.OwningGrid.CellStyle != null)
            {
                // Finally, set the style to the owning DataGrid's cell style if it is defined
                if (this.Style != this.OwningGrid.CellStyle)
                {
                    this.Style = this.OwningGrid.CellStyle;
                }
            }
        }

        // Makes sure the right gridline has the proper stroke and visibility. If lastVisibleColumn is specified, the 
        // right gridline will be collapsed if this cell belongs to the lastVisibileColumn and there is no filler column
        internal void EnsureGridLine(DataGridColumn lastVisibleColumn)
        {
            if (this.OwningGrid != null && _rightGridLine != null)
            {
                if (this.OwningGrid.VerticalGridLinesBrush != null && this.OwningGrid.VerticalGridLinesBrush != _rightGridLine.Fill)
                {
                    _rightGridLine.Fill = this.OwningGrid.VerticalGridLinesBrush;
                }

                Visibility newVisibility = (this.OwningGrid.GridLinesVisibility == DataGridGridLinesVisibility.Vertical || this.OwningGrid.GridLinesVisibility == DataGridGridLinesVisibility.All) &&
                    (this.OwningGrid.ColumnsInternal.FillerColumn.IsActive || this.OwningColumn != lastVisibleColumn) 
                    ? Visibility.Visible : Visibility.Collapsed;

                if (newVisibility != _rightGridLine.Visibility)
                {
                    _rightGridLine.Visibility = newVisibility;
                }
            }
        }

        #endregion Internal Methods


        #region Private Methods

        private void DataGridCell_MouseEnter(object sender, MouseEventArgs e)
        {
            if (this.OwningRow != null)
            {
                this.IsMouseOver = true;
            }
        }

        private void DataGridCell_MouseLeave(object sender, MouseEventArgs e)
        {
            if (this.OwningRow != null)
            {
                this.IsMouseOver = false;
            }
        }

        private void DataGridCell_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            if (!e.Handled)
            {
                // OwningGrid is null for TopLeftHeaderCell and TopRightHeaderCell because they have no OwningRow
                if (this.OwningGrid != null)
                {
                    if (this.OwningRow != null)
                    {
                        Debug.Assert(sender is DataGridCell);
                        Debug.Assert(sender == this);
                        e.Handled = this.OwningGrid.UpdateStateOnMouseLeftButtonDown(e, this.ColumnIndex, this.RowIndex);
                    }
                    if (this.OwningGrid.IsTabStop)
                    {
                        bool success = this.OwningGrid.Focus();
                        Debug.Assert(success);
                    }
                }
            }
        }

        #endregion Private Methods
    }
}
