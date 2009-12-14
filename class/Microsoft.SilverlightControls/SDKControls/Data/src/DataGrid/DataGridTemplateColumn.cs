// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

namespace System.Windows.Controls
{
    public class DataGridTemplateColumn : DataGridColumn
    {
        #region Constants

        #endregion Constants

        #region Data

        private DataTemplate _cellTemplate;
        private DataTemplate _cellEditingTemplate;

        #endregion Data

        public DataGridTemplateColumn()
        {
        }

        #region Dependency Properties
        /* 








































































































*/

        #endregion

        #region Properties
        public DataTemplate CellEditingTemplate
        {
            get
            {
                return this._cellEditingTemplate;
            }
            set
            {
                if (this._cellEditingTemplate != value)
                {
                    this._cellEditingTemplate = value;
                    // 




                }
            }
        }

        public DataTemplate CellTemplate
        {
            get
            {
                return this._cellTemplate;
            }
            set
            {
                if (this._cellTemplate != value)
                {
                    this._cellTemplate = value;
                    // 




                }
            }
        }

        internal bool HasDistinctTemplates
        {
            get
            {
                return this.CellTemplate != this.CellEditingTemplate;
            }
        }

        #endregion Properties

        #region Methods

        protected override FrameworkElement GenerateEditingElement(DataGridCell cell, object dataItem)
        {
            if (this.CellEditingTemplate == null)
            {
                throw DataGridError.DataGridTemplateColumn.MissingTemplateForType(typeof(DataGridTemplateColumn));
            }
            return this.CellEditingTemplate.LoadContent() as FrameworkElement;
        }

        protected override FrameworkElement GenerateElement(DataGridCell cell, object dataItem)
        {
            if (this.CellTemplate == null)
            {
                throw DataGridError.DataGridTemplateColumn.MissingTemplateForType(typeof(DataGridTemplateColumn));
            }
            return this.CellTemplate.LoadContent() as FrameworkElement;
        }

        protected override object PrepareCellForEdit(FrameworkElement editingElement, RoutedEventArgs editingEventArgs)
        {
            return null;
        }

        #endregion Methods
    }
}
