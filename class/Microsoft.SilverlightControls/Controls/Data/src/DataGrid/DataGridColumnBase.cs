// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System.Diagnostics;
 
namespace System.Windows.Controlsb1
{
    [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1036:OverrideMethodsOnComparableTypes", Justification="IComparable in Jolt has only one method, probably a false positive in FxCop.")] 
    public abstract class DataGridColumnBase : /* DependencyObject,*/ IComparable<DataGridColumnBase>
    {
        #region Constants 

        private const byte DATAGRIDCOLUMN_defaultMinWidth = 0;
        internal const int  DATAGRIDCOLUMN_maximumWidth = 65536; 
        internal const byte DATAGRIDCOLUMN_minMinWidth = 2; 

        #endregion Constants 

        #region Data
 
        private Style _cellStyle; //
        private int _displayIndex;
        private object _header; 
        private DataGridColumnHeader _headerCell; 
        private Style _headerStyle; //
        private bool _isFrozen; 
        private bool _isReadOnly;
        private Visibility _visibility;
 
        /*

*/ 
 
        #endregion Data
 
        protected DataGridColumnBase()
        {
            this._visibility = Visibility.Visible; 
            this.MinWidth = DATAGRIDCOLUMN_defaultMinWidth;
            this._displayIndex = -1;
        } 
 
        #region IComparable<DataGridColumnBase> Implementation
 
        public int CompareTo(DataGridColumnBase other)
        {
            return this._displayIndex - other._displayIndex; 
        }

        #endregion IComparable<DataGridColumnBase> Implementation 
 
        #region Dependency Properties
        /* 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
 

 


 


 
*/ 

        #endregion 

        #region Public Properties
 
        //
        public Style CellStyle
        { 
            get 
            {
                return _cellStyle; 
            }
            set
            { 
                if (_cellStyle != value)
                {
                    _cellStyle = value; 
                    if (this.OwningGrid != null) 
                    {
                        this.OwningGrid.OnColumnCellStyleChanged(this); 
                    }
                }
            } 
        }

        public bool? CanUserResize 
        { 
            get;
            set; 
        }

        // 
        public Style HeaderStyle
        {
            get 
            { 
                return _headerStyle;
            } 
            set
            {
                if (_headerStyle != value) 
                {
                    _headerStyle = value;
                    this.HeaderCell.Style = value; 
                } 
            }
        } 

        public int DisplayIndex
        { 
            get
            {
                return this._displayIndex; 
            } 
            set
            { 
                if (this._displayIndex != value)
                {
                    if (value == Int32.MaxValue) 
                    {
                        throw DataGridError.DataGrid.ValueMustBeLessThan("value", "DisplayIndex", Int32.MaxValue);
                    } 
                    if (this.OwningGrid != null) 
                    {
                        if (value < 0 || value >= this.OwningGrid.Columns.Count) 
                        {
                            throw DataGridError.DataGrid.ValueMustBeBetween("value", "DisplayIndex", 0, true, this.OwningGrid.Columns.Count, false);
                        } 
                        // Will throw an error if a visible frozen column is placed inside a non-frozen area or vice-versa.
                        this.OwningGrid.OnColumnDisplayIndexChanging(this, value);
                        this._displayIndex = value; 
                        try 
                        {
                            this.OwningGrid.InDisplayIndexAdjustments = true; 
                            this.OwningGrid.OnColumnDisplayIndexChanged_PreNotification();
                            this.OwningGrid.OnColumnDisplayIndexChanged(this);
                            this.OwningGrid.OnColumnDisplayIndexChanged_PostNotification(); 
                        }
                        finally
                        { 
                            this.OwningGrid.InDisplayIndexAdjustments = false; 
                        }
                    } 
                    else
                    {
                        if (value < -1) 
                        {
                            throw DataGridError.DataGrid.ValueMustBeGreaterThanOrEqualTo("value", "DisplayIndex", -1);
                        } 
                        this._displayIndex = value; 
                    }
                } 
            }
        }
 
        public object Header
        {
            get 
            { 
                return this._header;
            } 
            set
            {
                if (this._header != value) 
                {
                    this._header = value;
                    if (this._headerCell != null) 
                    { 
                        this._headerCell.Content = this._header;
                    } 
                }
            }
        } 

        public int Index
        { 
            get; 
            internal set;
        } 

        public bool IsReadOnly
        { 
            get
            {
                if (this.OwningGrid == null) 
                { 
                    return this._isReadOnly;
                } 
                return this.OwningGrid.GetColumnReadOnlyState(this, this._isReadOnly);
            }
            set 
            {
                if (value != this.IsReadOnly)
                { 
                    if (this.OwningGrid != null) 
                    {
                        this.OwningGrid.OnColumnReadOnlyStateChanging(this, value); 
                    }
                    this._isReadOnly = value;
                    // 


 
 
                }
            } 
        }

        // 
        public double MinWidth
        {
            get 
            { 
                return this.InheritedMinWidth;
            } 
            set
            {
                if (this.LocalMinWidth != value) 
                {
                    if (value < DATAGRIDCOLUMN_minMinWidth && value != 0)
                    { 
                        throw DataGridError.DataGrid.ValueMustBeGreaterThanOrEqualTo("value", "MinWidth", DATAGRIDCOLUMN_minMinWidth); 
                    }
                    double newEffectiveMinWidth; 
                    if (value == 0)
                    {
                        if (this.OwningGrid == null) 
                        {
                            newEffectiveMinWidth = DataGrid.DATAGRID_defaultMinColumnWidth;
                        } 
                        else 
                        {
                            newEffectiveMinWidth = this.OwningGrid.MinColumnWidth; 
                        }
                    }
                    else 
                    {
                        newEffectiveMinWidth = value;
                    } 
                    if (this.Width < newEffectiveMinWidth) 
                    {
                        this.Width = newEffectiveMinWidth; 
                    }
                    this.LocalMinWidth = value;
                    // 


 
                } 
            }
        } 

        //
        public Visibility Visibility 
        {
            get
            { 
                return this._visibility; 
            }
            set 
            {
                if (value != this.Visibility)
                { 
                    if (this.OwningGrid != null)
                    {
                        this.OwningGrid.OnColumnVisibleStateChanging(this); 
                    } 
                    this._visibility = value;
                    if (this.OwningGrid != null) 
                    {
                        this.OwningGrid.OnColumnVisibleStateChanged(this);
                    } 
                }
            }
        } 
 
        public double Width
        { 
            get
            {
                return this.InheritedWidth; 
            }
            set
            { 
                double minWidth = this.InheritedMinWidth; 
                if (value < minWidth)
                { 
                    // If it's 0, we inherit the width from the DataGrid; We don't need to check
                    // the inherited value here because it's already checked when it's set
                    if (value != 0) 
                    {
                        throw DataGridError.DataGrid.ValueMustBeGreaterThanOrEqualTo("value", "Width", minWidth);
                    } 
                } 
                if (value > DATAGRIDCOLUMN_maximumWidth)
                { 
                    throw DataGridError.DataGrid.ValueMustBeLessThanOrEqualTo("value", "Width", DATAGRIDCOLUMN_maximumWidth);
                }
                double oldInheritedWidth = this.InheritedWidth; 
                double newInheritedWidth;
                if (value == 0)
                { 
                    newInheritedWidth = (this.OwningGrid == null) ? DataGrid.DATAGRID_defaultColumnWidth : this.OwningGrid.ColumnWidth; 
                }
                else 
                {
                    newInheritedWidth = value;
                } 
                if (oldInheritedWidth != newInheritedWidth)
                {
                    // 
 

 
                    this.LocalWidth = value;
                    if (this.OwningGrid != null)
                    { 
                        this.OwningGrid.OnColumnWidthChanged(this);
                    }
                } 
            } 
        }
 
        #endregion Public Properties

        #region Internal Properties 

        internal bool DisplayIndexHasChanged
        { 
            get; 
            set;
        } 

        internal int DisplayIndexInternal
        { 
            set
            {
                Debug.Assert(value >= -1); 
                Debug.Assert(value < Int32.MaxValue); 

                this._displayIndex = value; 
            }
        }
 
        internal bool HasHeaderCell
        {
            get 
            { 
                return this._headerCell != null;
            } 
        }

        /* 


 
 

 


 


 
 

*/ 

        internal DataGridColumnHeader HeaderCell
        { 
            get
            {
                if (this._headerCell == null) 
                { 
                    this._headerCell = new DataGridColumnHeader();
 
                    // Take the style set on this column or the one defined at the DataGrid level
                    Style style = this.HeaderStyle;
                    if (style == null && this.OwningGrid != null) 
                    {
                        style = this.OwningGrid.ColumnHeaderStyle;
                    } 
                    if (style != null) 
                    {
                        this._headerCell.Style = style; 
                    }

                    this._headerCell.OwningColumn = this; 
                    this._headerCell.Content = this._header;
                }
                return this._headerCell; 
            } 
        }
 
        internal double InheritedMinWidth
        {
            get 
            {
                if (this.LocalMinWidth == 0)
                { 
                    return (this.OwningGrid == null) ? DataGrid.DATAGRID_defaultMinColumnWidth : this.OwningGrid.MinColumnWidth; 
                }
                return this.LocalMinWidth; 
            }
        }
 
        internal double InheritedWidth
        {
            get 
            { 
                if (this.LocalWidth == 0)
                { 
                    return this.OwningGrid == null ? DataGrid.DATAGRID_defaultColumnWidth : this.OwningGrid.ColumnWidth;
                }
                return this.LocalWidth; 
            }
        }
 
        // 
        internal bool IsFrozen
        { 
            get
            {
                return this._isFrozen; 
            }
            set
            { 
                if (value != this.IsFrozen) 
                {
                    if (this.OwningGrid != null) 
                    {
                        this.OwningGrid.OnColumnFrozenStateChanging(this);
                    } 
                    this._isFrozen = value;
                    if (this.OwningGrid != null)
                    { 
                        this.OwningGrid.OnColumnFrozenStateChanged(this); 
                    }
                } 
            }
        }
 
        /*

 
 

 


 


 
*/ 

        internal double LocalMinWidth 
        {
            get;
            private set; 
        }

        internal double LocalWidth 
        { 
            get;
            private set; 
        }

        internal DataGrid OwningGrid 
        {
            get;
            set; 
        } 

        /* 


 


 
 

 


 


 
*/ 

        internal Type Type 
        {
            get;
            set; 
        }

        #endregion Internal Properties 
 
        #region Public Methods
 
        public FrameworkElement GetElement(DataGridRow dataGridRow)
        {
            if (dataGridRow == null) 
            {
                throw new ArgumentNullException("dataGridRow");
            } 
            if (this.OwningGrid == null) 
            {
                throw DataGridError.DataGrid.NoOwningGrid(this.GetType()); 
            }
            Debug.Assert(this.Index >= 0);
            Debug.Assert(this.Index < this.OwningGrid.Columns.Count); 
            DataGridCell dataGridCell = dataGridRow.Cells[this.Index];
            Debug.Assert(dataGridCell != null);
            return dataGridCell.Content as FrameworkElement; 
        } 

        public FrameworkElement GetElement(object dataItem) 
        {
            if (dataItem == null)
            { 
                throw new ArgumentNullException("dataItem");
            }
            if (this.OwningGrid == null) 
            { 
                throw DataGridError.DataGrid.NoOwningGrid(this.GetType());
            } 
            Debug.Assert(this.Index >= 0);
            Debug.Assert(this.Index < this.OwningGrid.Columns.Count);
            DataGridRow dataGridRow = this.OwningGrid.GetRowFromItem(dataItem); 
            if (dataGridRow == null)
            {
                // 
                return null; 
            }
            return GetElement(dataGridRow); 
        }

        /// <summary> 
        /// Called by the DataGrid control when a column asked for its
        /// elements to be updated, typically because one of its properties changed.
        /// </summary> 
        /// <param name="element">Indicates the element that needs to be updated</param> 
        /// <param name="propertyName">Indicates which property changed and caused this call</param>
        public virtual void UpdateElement(FrameworkElement element, string propertyName) 
        {
        }
 
        #endregion Public Methods

        #region Protected Methods 
 
        /// <summary>
        /// Called by a specific column type when one of its properties changed, 
        /// and its current cells need to be updated.
        /// </summary>
        /// <param name="propertyName">Indicates which property changed and caused this call</param> 
        protected void UpdateElements(string propertyName)
        {
            if (this.OwningGrid == null) 
            { 
                return;
            } 
            this.OwningGrid.UpdateColumnElements(this, propertyName);
        }
 
        #endregion Protected Methods

        #region Internal Methods 
 
        /*
 


 
*/

        #endregion Internal Methods 
 
        #region Private Methods
 
        /*

 


 
 

 


 
*/

        #endregion Private Methods 
 
    }
} 
