// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System.Diagnostics;
using System.Windows.Data; 
 
namespace System.Windows.Controlsb1
{ 
    public abstract class DataGridBoundColumnBase : DataGridColumnBase
    {
        #region Constants 

        #endregion Constants
 
        #region Data 

        private Binding _displayMemberBinding; 
        private Style _elementStyle; //
        private Style _editingElementStyle; //
 
        #endregion Data

        protected DataGridBoundColumnBase() 
        { 
        }
 
        #region Dependency Properties
        /*
 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


*/ 

        #endregion
 
        #region Public Properties 

        public Binding DisplayMemberBinding 
        {
            get
            { 
                return this._displayMemberBinding;
            }
            set 
            { 
                if (this._displayMemberBinding != value)
                { 
                    if (this.OwningGrid != null && !this.OwningGrid.EndEdit(true /*commitChanges*/, true /*exitEditingMode*/))
                    {
                        // Edited value couldn't be committed or aborted 
                        throw DataGridError.DataGrid.CommitFailedCannotCompleteOperation();
                    }
 
                    DataGridValueConverter dataGridValueConverter; 

                    // Stop listening to the old DataConversionError event 
                    if (this._displayMemberBinding != null)
                    {
                        dataGridValueConverter = this._displayMemberBinding.Converter as DataGridValueConverter; 
                        if (dataGridValueConverter != null)
                        {
                            dataGridValueConverter.DataConversionError -= new EventHandler<DataGridDataErrorEventArgs>(DataGridValueConverter_DataConversionError); 
                        } 
                    }
 
                    this._displayMemberBinding = value;

                    if (this._displayMemberBinding != null) 
                    {
                        // Force the TwoWay binding mode
                        this._displayMemberBinding.Mode = BindingMode.TwoWay; 
 
                        // Start listening to the new DataConversionError event
                        if (this._displayMemberBinding.Converter == null) 
                        {
                            dataGridValueConverter = new DataGridValueConverter(this);
                            dataGridValueConverter.DataConversionError += new EventHandler<DataGridDataErrorEventArgs>(DataGridValueConverter_DataConversionError); 
                            this._displayMemberBinding.Converter = dataGridValueConverter;
                            if (this._displayMemberBinding.ConverterCulture == null)
                            { 
                                this._displayMemberBinding.ConverterCulture = System.Globalization.CultureInfo.CurrentCulture; 
                            }
                        } 

                        // Apply the new Binding to existing rows in the DataGrid
                        if (this.OwningGrid != null) 
                        {
                            //
 
 
                            this.OwningGrid.OnColumnDisplayMemberBindingChanged(this);
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

        #region Internal Properties 

        //
 
        internal string DisplayMemberPath 
        {
            get; 
            set;
        }
 
        #endregion Internal Properties

        #region Public Methods 
 
        public abstract void CancelCellEdit(object uneditedValue);
        public abstract object PrepareCellEdit(DataGridEditingTriggerInfo editingTriggerInfo); 

        #endregion Public Methods
 
        #region Protected Methods

        protected abstract FrameworkElement GenerateEditingElement(); 
        protected abstract FrameworkElement GenerateElement(); 

        #endregion Protected Methods 

        #region Internal Methods
 
        internal FrameworkElement GenerateEditingElementInternal()
        {
            return GenerateEditingElement(); 
        } 

        internal FrameworkElement GenerateElementInternal() 
        {
            return GenerateElement();
        } 

        #endregion Internal Methods
 
        #region Private Methods 

        private void DataGridValueConverter_DataConversionError(object sender, DataGridDataErrorEventArgs dataError) 
        {
            Debug.Assert(dataError != null);
            Debug.Assert(sender is DataGridValueConverter); 

            if (this.OwningGrid != null)
            { 
                this.OwningGrid.OnDataConversionError(dataError); 
            }
        } 

        #endregion Private Methods
    } 
}
