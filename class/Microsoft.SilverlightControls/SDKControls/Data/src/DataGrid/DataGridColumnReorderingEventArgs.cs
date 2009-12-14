// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

namespace System.Windows.Controls
{
    public class DataGridColumnReorderingEventArgs : EventArgs
    {
        public DataGridColumnReorderingEventArgs(DataGridColumn dataGridColumn)
        {
            this.Column = dataGridColumn;
        }

        #region Public Properties

        /// <summary>
        /// The column being moved.
        /// </summary>
        public DataGridColumn Column
        {
            get;
            private set;
        }

        /// <summary>
        /// If true, then the event should be considered handled, and no further processing should be performed by event handlers.
        /// </summary>
        public bool Cancel
        {
            get;
            set;
        }

        /// <summary>
        /// The popup indicator displayed while dragging.  If null and Handled = true, then do not display a tooltip.
        /// </summary>
        public Control DragIndicator
        {
            get;
            set;
        }

        /// <summary>
        /// UIElement to display at the insertion position.  If null and Handled = true, then do not display an insertion indicator.
        /// </summary>
        public Control DropLocationIndicator
        {
            get;
            set;
        }

        #endregion Public Properties
    }
}
