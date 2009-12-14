// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Windows.Data;

namespace System.Windows.Controls
{
    public abstract class DataGridBoundColumn : DataGridColumn
    {
        #region Constants

        #endregion Constants

        #region Data

        private Binding _binding;
        private Style _elementStyle; // 
        private Style _editingElementStyle; // 

        #endregion Data

        #region Dependency Properties
        /* 


























































































*/

        #endregion

        #region Public Properties

        public virtual Binding Binding
        {
            get
            {
                return this._binding;
            }
            set
            {
                if (this._binding != value)
                {
                    if (this.OwningGrid != null && !this.OwningGrid.CommitEdit(DataGridEditingUnit.Row, true /*exitEditing*/))
                    {
                        // Edited value couldn't be committed or aborted
                        throw DataGridError.DataGrid.CommitFailedCannotCompleteOperation();
                    }

                    this._binding = value;

                    if (this._binding != null)
                    {
                        // Force the TwoWay binding mode if there is a Path present.  TwoWay binding requires a Path.
                        if (!String.IsNullOrEmpty(this._binding.Path.Path))
                        {
                            this._binding.Mode = BindingMode.TwoWay;
                        }

                        if (this._binding.Converter == null)
                        {
                            this._binding.Converter = new DataGridValueConverter();
                        }

                        // Apply the new Binding to existing rows in the DataGrid
                        if (this.OwningGrid != null)
                        {
                            // 


                            this.OwningGrid.OnColumnBindingChanged(this);
                        }
                    }

                    //
                }
            }
        }

        // 
        public Style EditingElementStyle
        {
            get
            {
                return _editingElementStyle;
            }
            set
            {
                if (_editingElementStyle != value)
                {
                    _editingElementStyle = value;
                    // We choose not to update the elements already editing in the Grid here.  They
                    // will get the EditingElementStyle next time they go into edit mode
                }
            }
        }

        // 
        public Style ElementStyle
        {
            get
            {
                return _elementStyle;
            }
            set
            {
                if (_elementStyle != value)
                {
                    _elementStyle = value;
                    if (this.OwningGrid != null)
                    {
                        this.OwningGrid.OnColumnElementStyleChanged(this);
                    }
                }
            }
        }

        #endregion Public Properties

    }
}
