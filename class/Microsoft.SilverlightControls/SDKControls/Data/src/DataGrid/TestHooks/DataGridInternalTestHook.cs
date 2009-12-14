// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Windows.Controls.Primitives;
using System.Windows.Input;

namespace System.Windows.Controls
{
    public partial class DataGrid
    {
        private InternalTestHook _testHook;

        //Internal property to expose the testhook object
        internal InternalTestHook TestHook
        {
            get
            {
                if (_testHook == null)
                {
                    _testHook = new InternalTestHook(this);
                }
                return _testHook;
            }
        }
        /// <summary>
        /// Test hook class that exposes internal and private members of the datagrid
        /// </summary>
        internal class InternalTestHook
        {
            //Reference to the outer 'parent' datagrid
            private DataGrid _grid;

            internal InternalTestHook(DataGrid grid)
            {
                _grid = grid;
            }

            #region Internal Properties
            internal DataGridColumnCollection ColumnsInternal
            {
                get
                {
                    return _grid.ColumnsInternal;
                }
            }

            internal DataGridCellCoordinates CurrentCellCoordinates
            {
                get
                {
                    return _grid.CurrentCellCoordinates;
                }
            }

            internal DataGridDataConnection DataConnection
            {
                get
                {
                    return _grid.DataConnection;
                }
            }

            internal static double DATAGRID_defaultMinColumnWidth
            {
                get
                {
                    return DataGrid.DATAGRID_defaultMinColumnWidth;
                }
            }

            internal static double DATAGRID_minimumRowHeadersWidth
            {
                get 
                {
                    return DataGrid.DATAGRID_minimumRowHeaderWidth; 
                }
            }

            internal static double DATAGRID_maxHeadersThickness
            {
                get
                {
                    return DataGrid.DATAGRID_maxHeadersThickness;
                }
            }

            internal static double DATAGRID_minimumColumnHeaderHeight
            {
                get
                {
                    return DataGrid.DATAGRID_minimumColumnHeaderHeight;
                }
            }
           
            internal DataGridDisplayData DisplayData
            {
                get
                {
                    return _grid.DisplayData;
                }
            }

            internal int DisplayedRowCount
            {
                get
                {
                    return _grid.DisplayData.NumDisplayedScrollingRows;
                }
            }

            
            internal bool DelayAutogenerateColumns
            {
                set
                {
                    _grid._measured = !value;
                }
            }

            internal double HorizontalOffset
            {
                get
                {
                    return _grid.HorizontalOffset;
                }
            }

            internal DataGridRowsPresenter RowsPresenter
            {
                get
                {
                    return _grid._rowsPresenter;
                }
            }
            
            #endregion

            #region Internal Methods
            internal void ClearRowSelection(bool resetAnchorRowIndex)
            {
                _grid.ClearRowSelection(resetAnchorRowIndex);
            }

            internal DataGridRow GetRowFromItem(object dataItem)
            {
                return _grid.GetRowFromItem(dataItem);
            }

            internal bool IsColumnDisplayed(int columnIndex)
            {
                return _grid.IsColumnDisplayed(columnIndex);
            }

            internal bool IsRowDisplayed(int rowIndex)
            {
                return _grid.DisplayData.IsRowDisplayed(rowIndex);
            }

            //key processing methods
            internal void ProcessDataGridKey(KeyEventArgs e)
            {
                _grid.ProcessDataGridKey(e);
            }

            internal bool ProcessAKey()
            {
                return _grid.ProcessAKey();
            }

            internal bool ProcessDownKey(bool shift, bool ctrl, out bool moved)
            {
                return _grid.ProcessDownKeyInternal(shift, ctrl, out moved);
            }

            internal bool ProcessEndKey()
            {
                return _grid.ProcessEndKey();
            }

            internal bool ProcessEnterKey()
            {
                return _grid.ProcessEnterKey();
            }

            internal bool ProcessEscapeKey()
            {
                return _grid.ProcessEscapeKey();
            }

            internal bool ProcessHomeKey()
            {
                return _grid.ProcessHomeKey();
            }

            internal bool ProcessLeftKey()
            {
                return _grid.ProcessLeftKey();
            }

            internal bool ProcessNextKey()
            {
                return _grid.ProcessNextKey();
            }

            internal bool ProcessPriorKey()
            {
                return _grid.ProcessPriorKey();
            }

            internal bool ProcessRightKey()
            {
                return _grid.ProcessRightKey();
            }

            internal bool ProcessTabKey(KeyEventArgs e)
            {
                return _grid.ProcessTabKey(e);
            }

            internal bool ProcessUpKey()
            {
                return _grid.ProcessUpKey();
            }           

            internal void SetRowSelection(int rowIndex, bool isSelected, bool setAnchorRowIndex)
            {
                _grid.SetRowSelection(rowIndex, isSelected, setAnchorRowIndex);
            }

            internal void SetRowsSelection(int startRowIndex, int endRowIndex)
            {
                _grid.SetRowsSelection(startRowIndex, endRowIndex);
            }

            internal bool ScrollIntoView(int columnIndex, int rowIndex, bool forCurrentCellChange)
            {
                return _grid.ScrollIntoView(columnIndex, rowIndex, forCurrentCellChange);
            }
            #endregion
        }


    }
}
