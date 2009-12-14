// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Diagnostics;
using System.Windows.Controls.Primitives;

namespace System.Windows.Controls
{
    internal class DataGridFillerColumn : DataGridColumn
    {
        public DataGridFillerColumn(DataGrid owningGrid)
        {
            this.OwningGrid = owningGrid;
        }

        #region Properties

        // True if there is room for the filler column; otherwise, false
        internal bool IsActive
        {
            get
            {
                return this.Width > 0;
            }
        }

        // True if the FillerColumn's header cell is contained in the visual tree
        internal bool IsRepresented
        {
            get;
            set;
        }

        internal new double Width
        {
            get;
            set;
        }

        #endregion Properties

        #region Methods

        internal override DataGridColumnHeader CreateHeader()
        {
            DataGridColumnHeader headerCell = base.CreateHeader();
            if (headerCell != null)
            {
                headerCell.IsEnabled = false;
            }
            return headerCell;
        }

        protected override FrameworkElement GenerateElement(DataGridCell cell, object dataItem)
        {
            Debug.Assert(false);
            return null;
        }

        protected override FrameworkElement GenerateEditingElement(DataGridCell cell, object dataItem)
        {
            Debug.Assert(false);
            return null;
        }

        protected override object PrepareCellForEdit(FrameworkElement editingElement, RoutedEventArgs editingEventArgs)
        {
            Debug.Assert(false);
            return null;
        }

        #endregion Methods
    }
}
