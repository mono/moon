// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Windows.Controls.Primitives;

namespace System.Windows.Controls
{
    [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1036:OverrideMethodsOnComparableTypes", Justification="IComparable in Jolt has only one method, probably a false positive in FxCop.")]
    public abstract class DataGridColumn : DependencyObject
    {
        #region Constants

        internal const int DATAGRIDCOLUMN_maximumWidth = 65536;
        private const bool DATAGRIDCOLUMN_defaultIsReadOnly = false;

        #endregion Constants

        #region Data

        private Style _cellStyle; // 
        private int _displayIndex;
        private double _desiredWidth; // Implicitly default to 0, keep it that way
        private object _header;
        private DataGridColumnHeader _headerCell;
        private Style _headerStyle; // 
        private double? _maxWidth;
        private double? _minWidth;
        private bool? _isReadOnly;
        private DataGridLength? _width; // Null by default, null means inherit the Width from the DataGrid
        private Visibility _visibility;

        #endregion Data

        protected internal DataGridColumn()
        {
            this._visibility = Visibility.Visible;
            this._displayIndex = -1;
        }

        #region Dependency Properties
        /* 




































































































































































































































































































































*/

        #endregion

        #region Public Properties

        /// <summary>
        /// Actual visible width after Width, MinWidth, and MaxWidth setting at the Column level and DataGrid level
        /// have been taken into account
        /// </summary>
        public double ActualWidth
        {
            get
            {
                if (this.OwningGrid == null)
                {
                    return 0;
                }
                double targetWidth = this.EffectiveWidth.IsAbsolute ? this.EffectiveWidth.Value : this.DesiredWidth;
                return Math.Min(Math.Max(targetWidth, this.ActualMinWidth), this.ActualMaxWidth);
            }
        }

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

        public bool CanUserReorder
        {
            get
            {
                if (this.CanUserReorderInternal.HasValue)
                {
                    return this.CanUserReorderInternal.Value;
                }
                else if (this.OwningGrid != null)
                {
                    return this.OwningGrid.CanUserReorderColumns;
                }
                else
                {
                    return DataGrid.DATAGRID_defaultCanUserResizeColumns;
                }
            }
            set
            {
                this.CanUserReorderInternal = value;
            }
        }

        public bool CanUserResize
        {
            get 
            {
                if (this.CanUserResizeInternal.HasValue)
                {
                    return this.CanUserResizeInternal.Value;
                }
                else if (this.OwningGrid != null)
                {
                    return this.OwningGrid.CanUserResizeColumns;
                }
                else
                {
                    return DataGrid.DATAGRID_defaultCanUserResizeColumns;
                }
            }
            set 
            { 
                this.CanUserResizeInternal = value; 
            }
        }

        public bool CanUserSort
        {
            get
            {
                if (this.CanUserSortInternal.HasValue)
                {
                    return this.CanUserSortInternal.Value;
                }
                else if (this.OwningGrid != null)
                {
                    string propertyPath = GetSortPropertyName();
                    Type propertyType = this.OwningGrid.DataConnection.DataType.GetNestedPropertyType(propertyPath);

                    // if the type is nullable, then we will compare the non-nullable type
                    if (TypeHelper.IsNullableType(propertyType))
                    {
                        propertyType = TypeHelper.GetNonNullableType(propertyType);
                    }

                    // return whether or not the property type can be compared
                    return (typeof(IComparable).IsAssignableFrom(propertyType)) ? true : false;
                }
                else
                {
                    return DataGrid.DATAGRID_defaultCanUserSortColumns;
                }
            }
            set
            {
                this.CanUserSortInternal = value;
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
                    if (_headerCell != null)
                    {
                        this._headerCell.Style = value;
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

        public bool IsAutoGenerated
        {
            get;
            internal set;
        }
        
        public bool IsFrozen
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
                    return this._isReadOnly ?? DATAGRIDCOLUMN_defaultIsReadOnly;
                }
                if (this._isReadOnly != null)
                {
                    return this._isReadOnly.Value || this.OwningGrid.IsReadOnly;
                }
                return this.OwningGrid.GetColumnReadOnlyState(this, DATAGRIDCOLUMN_defaultIsReadOnly);
            }
            set
            {
                if (value != this._isReadOnly)
                {
                    if (this.OwningGrid != null)
                    {
                        this.OwningGrid.OnColumnReadOnlyStateChanging(this, value);
                    }
                    this._isReadOnly = value;
                }
            }
        }

        public double MaxWidth
        {
            get
            {
                if (_maxWidth.HasValue)
                {
                    return _maxWidth.Value;
                }
                // 
                return double.PositiveInfinity;
            }
            set
            {
                if (value < 0)
                {
                    throw DataGridError.DataGrid.ValueMustBeGreaterThanOrEqualTo("value", "MaxWidth", 0);
                }
                if (value < this.ActualMinWidth)
                {
                    throw DataGridError.DataGrid.ValueMustBeGreaterThanOrEqualTo("value", "MaxWidth", "MinWidth");
                }
                if (!_maxWidth.HasValue || _maxWidth.Value != value)
                {
                    _maxWidth = value;
                    if (this.OwningGrid != null && _maxWidth.Value < this.ActualWidth)
                    {
                        this.OwningGrid.OnColumnWidthChanged(this);
                    }
                }
            }
        }

        public double MinWidth
        {
            get
            {
                if (_minWidth.HasValue)
                {
                    return _minWidth.Value;
                }
                return 0;
            }
            set
            {
                if (double.IsNaN(value))
                {
                    throw DataGridError.DataGrid.ValueCannotBeSetToNAN("MinWidth");
                }
                if (value < 0)
                {
                    throw DataGridError.DataGrid.ValueMustBeGreaterThanOrEqualTo("value", "MinWidth", 0);
                }
                if (double.IsPositiveInfinity(value))
                {
                    throw DataGridError.DataGrid.ValueCannotBeSetToInfinity("MinWidth");
                }
                if (value > this.ActualMaxWidth)
                {
                    throw DataGridError.DataGrid.ValueMustBeLessThanOrEqualTo("value", "MinWidth", "MaxWidth");
                }
                if (!_minWidth.HasValue || _minWidth.Value != value)
                {
                    _minWidth = value;
                    if (this.OwningGrid != null && _minWidth > this.ActualWidth)
                    {
                        this.OwningGrid.OnColumnWidthChanged(this);
                    }
                }
            }
        }

        /// <summary>
        /// Holds the name of the member to use for sorting, if not using the default.
        /// </summary>
        public string SortMemberPath
        {
            get;
            set;
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

                    if (this._headerCell != null)
                    {
                        this._headerCell.Visibility = this._visibility;
                    }

                    if (this.OwningGrid != null)
                    {
                        this.OwningGrid.OnColumnVisibleStateChanged(this);
                    }
                }
            }
        }

        public DataGridLength Width
        {
            get
            {
                if (_width.HasValue)
                {
                    return _width.Value;
                }
                else if (this.OwningGrid != null)
                {
                    return this.OwningGrid.ColumnWidth;
                }
                else
                {
                    
                    return DataGridLength.Auto;
                }
            }
            set
            {
                if (!_width.HasValue || _width.Value != value)
                {
                    _width = value;
                    if (value.IsAbsolute)
                    {
                        this.DesiredWidth = value.Value;
                    }
                    if (this.OwningGrid != null)
                    {
                        this.OwningGrid.OnColumnWidthChanged(this);
                    }
                }
            }
        }

        #endregion Public Properties

        #region Internal Properties

        internal bool ActualCanUserResize
        {
            get
            {
                if (this.OwningGrid == null || this.OwningGrid.CanUserResizeColumns == false || this is DataGridFillerColumn)
                {
                    return false;
                }
                return this.CanUserResizeInternal ?? true;
            }
        }

        // MaxWidth from local setting or DataGrid setting
        internal double ActualMaxWidth
        {
            get
            {
                return _maxWidth ?? (this.OwningGrid != null ? this.OwningGrid.MaxColumnWidth : double.PositiveInfinity);
            }
        }

        // MinWidth from local setting or DataGrid setting
        internal double ActualMinWidth
        {
            get
            {
                return _minWidth ?? (this.OwningGrid != null ? this.OwningGrid.MinColumnWidth : 0);
            }
        }

        internal bool? CanUserReorderInternal
        {
            get;
            set;
        }

        internal bool? CanUserResizeInternal
        {
            get;
            set;
        }

        internal bool? CanUserSortInternal
        {
            get;
            set;
        }

        // Desired pixel width of the Column without coercion
        internal double DesiredWidth
        {
            get
            {
                return _desiredWidth;
            }
            set
            {
                if (_desiredWidth != value)
                {
                    double oldActualWidth = this.ActualWidth;
                    _desiredWidth = value;
                    if (this.OwningGrid != null && oldActualWidth != this.ActualWidth)
                    {
                        this.OwningGrid.OnColumnWidthChanged(this);
                    }
                }
            }
        }

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

        // Width of the Column set locally or inherited from the DataGrid without any coercion
        internal DataGridLength EffectiveWidth
        {
            get
            {
                Debug.Assert(this.OwningGrid != null);
                return _width ?? this.OwningGrid.ColumnWidth;
            }
        }

        internal bool HasHeaderCell
        {
            get
            {
                return this._headerCell != null;
            }
        }

        internal DataGridColumnHeader HeaderCell
        {
            get
            {
                if (this._headerCell == null)
                {
                    this._headerCell = CreateHeader();
                }
                return this._headerCell;
            }
        }
        
        internal int Index
        {
            get;
            set;
        }

        internal DataGrid OwningGrid
        {
            get;
            set;
        }

        #endregion Internal Properties

        #region Public Methods

        public FrameworkElement GetCellContent(DataGridRow dataGridRow)
        {
            if (dataGridRow == null)
            {
                throw new ArgumentNullException("dataGridRow");
            }
            if (this.OwningGrid == null)
            {
                throw DataGridError.DataGrid.NoOwningGrid(this.GetType());
            }
            if (dataGridRow.OwningGrid == this.OwningGrid)
            {
                Debug.Assert(this.Index >= 0);
                Debug.Assert(this.Index < this.OwningGrid.Columns.Count);
                DataGridCell dataGridCell = dataGridRow.Cells[this.Index];
                if (dataGridCell != null)
                {
                    return dataGridCell.Content as FrameworkElement;
                }
            }
            return null;
        }

        public FrameworkElement GetCellContent(object dataItem)
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
                return null;
            }
            return GetCellContent(dataGridRow);
        }

        #endregion Public Methods

        #region Protected Methods

        protected virtual void CancelCellEdit(FrameworkElement editingElement, object uneditedValue)
        { }

        protected abstract FrameworkElement GenerateEditingElement(DataGridCell cell, object dataItem);
        protected abstract FrameworkElement GenerateElement(DataGridCell cell, object dataItem);

        /// <summary>
        /// Called by a specific column type when one of its properties changed, 
        /// and its current cells need to be updated.
        /// </summary>
        /// <param name="propertyName">Indicates which property changed and caused this call</param>
        protected void NotifyPropertyChanged(string propertyName)
        {
            if (this.OwningGrid == null)
            {
                return;
            }
            this.OwningGrid.RefreshColumnElements(this, propertyName);
        }

        protected abstract object PrepareCellForEdit(FrameworkElement editingElement, RoutedEventArgs editingEventArgs);

        /// <summary>
        /// Called by the DataGrid control when a column asked for its
        /// elements to be refreshed, typically because one of its properties changed.
        /// </summary>
        /// <param name="element">Indicates the element that needs to be refreshed</param>
        /// <param name="propertyName">Indicates which property changed and caused this call</param>
        protected internal virtual void RefreshCellContent(FrameworkElement element, string propertyName)
        {
        }

        #endregion Protected Methods

        #region Internal Methods

        internal void CancelCellEditInternal(FrameworkElement editingElement, object uneditedValue)
        {
            CancelCellEdit(editingElement, uneditedValue);
        }

        internal virtual DataGridColumnHeader CreateHeader()
        {
            DataGridColumnHeader result = new DataGridColumnHeader();

            // Take the style set on this column or the one defined at the DataGrid level
            Style style = this.HeaderStyle;
            if (style == null && this.OwningGrid != null)
            {
                style = this.OwningGrid.ColumnHeaderStyle;
            }
            if (style != null)
            {
                result.Style = style;
            }

            result.OwningColumn = this;
            result.Content = this._header;

            return result;
        }

        internal FrameworkElement GenerateEditingElementInternal(DataGridCell cell, object dataItem)
        {
            return GenerateEditingElement(cell, dataItem);
        }

        internal FrameworkElement GenerateElementInternal(DataGridCell cell, object dataItem)
        {
            return GenerateElement(cell, dataItem);
        }

        /// <summary>
        /// We get the sort description from the data source.  We don't worry whether we can modify sort -- perhaps the sort description
        /// describes an unchangeable sort that exists on the data.
        /// </summary>
        /// <returns></returns>
        internal SortDescription? GetSortDescription()
        {
            if (this.OwningGrid != null
                && this.OwningGrid.DataConnection != null
                && this.OwningGrid.DataConnection.SortDescriptions != null)
            {
                string propertyName = GetSortPropertyName();

                SortDescription sort = (new List<SortDescription>(this.OwningGrid.DataConnection.SortDescriptions))
                    .FirstOrDefault(s => s.PropertyName == propertyName);

                if (sort.PropertyName != null)
                {
                    return sort;
                }

                return null;
            }


            return null;
        }

        internal string GetSortPropertyName()
        {
            string result = this.SortMemberPath;

            if (String.IsNullOrEmpty(result))
            {
                DataGridBoundColumn boundColumn = this as DataGridBoundColumn;

                if (boundColumn != null && boundColumn.Binding != null && boundColumn.Binding.Path != null)
                {
                    result = boundColumn.Binding.Path.Path;
                }
            }

            return result;
        }

        internal object PrepareCellForEditInternal(FrameworkElement editingElement, RoutedEventArgs editingEventArgs)
        {
            return PrepareCellForEdit(editingElement, editingEventArgs);
        }

        #endregion Internal Methods

        #region Private Methods

        #endregion Private Methods

    }
}
