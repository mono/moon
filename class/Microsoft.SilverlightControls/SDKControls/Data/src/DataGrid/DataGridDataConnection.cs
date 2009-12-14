// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Collections;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Diagnostics;
using System.Reflection;

namespace System.Windows.Controls
{
    internal class DataGridDataConnection
    {
        #region Data

        private PropertyInfo[] _dataProperties;
        private DataGrid _owner;

        #endregion Data

        public DataGridDataConnection(DataGrid owner)
        {
            this._owner = owner;
        }

        #region Public Properties

        // 














        public bool AllowEdit
        {
            get
            {
                // 




                if (this.List == null)
                {
                    return true;
                }
                else
                {
                    return !this.List.IsReadOnly;
                }
            }
        }

        /// <summary>
        /// True if the collection view says it can sort.
        /// </summary>
        public bool AllowSort
        {
            get
            {
                if (this.CollectionView == null)
                {
                    return false;
                }
                else
                {
                    return this.CollectionView.CanSort;
                }
            }
        }

        public ICollectionView CollectionView
        {
            get
            {
                return this.DataSource as ICollectionView;
            }
        }

        public int Count
        {
            get
            {
                IList list = this.List;
                if (list != null)
                {
                    return list.Count;
                }

                ListCollectionView lcv = this.DataSource as ListCollectionView;
                if (lcv != null)
                {
                    return lcv.Count;
                }

                int count = 0;
                IEnumerable enumerable = this.DataSource;
                if (enumerable != null)
                {
                    IEnumerator enumerator = enumerable.GetEnumerator();
                    if (enumerator != null)
                    {
                        while (enumerator.MoveNext())
                        {
                            count++;
                        }
                    }
                }
                return count;
            }
        }

        public bool DataIsPrimitive
        {
            get
            {
                return DataTypeIsPrimitive(this.DataType);
            }
        }

        public PropertyInfo[] DataProperties
        {
            get
            {
                if (_dataProperties == null && this.DataSource != null)
                {
                    Type dataType = this.DataType;

                    if (dataType != null && !DataTypeIsPrimitive(dataType))
                    {
                        _dataProperties = dataType.GetProperties(BindingFlags.Public | BindingFlags.Instance);
                        Debug.Assert(_dataProperties != null);
                    }
                    else
                    {
                        _dataProperties = null;
                    }
                }
                return _dataProperties;
            }
        }

        public IEnumerable DataSource
        {
            get;
            set;
        }

        public Type DataType
        {
            get
            {
                // We need to use the raw ItemsSource as opposed to DataSource because DataSource
                // may be the ItemsSource wrapped in a ListCollectionView, in which case we wouldn't
                // be able to take T to be the type if we're given IEnumerable<T>
                if (_owner.ItemsSource == null)
                {
                    return null;
                }
                return _owner.ItemsSource.GetItemType();
            }
        }

        // 








        public IList List
        {
            get
            {
                return this.DataSource as IList;
            }
        }

        public SortDescriptionCollection SortDescriptions
        {
            get
            {
                if (this.CollectionView != null && this.CollectionView.CanSort)
                {
                    return this.CollectionView.SortDescriptions;
                }
                else
                {
                    return (SortDescriptionCollection)null;
                }
            }
        }

        #endregion Public Properties

        #region Internal Properties
        #endregion Internal Properties

        #region Public Methods

        // 







        /// <summary>
        /// Puts the entity into editing mode if possible
        /// </summary>
        /// <param name="dataItem">The entity to edit</param>
        /// <returns>True if editing was started</returns>
        //
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Performance", "CA1822:MarkMembersAsStatic")]
        public bool BeginEdit(object dataItem)
        {
            if (dataItem == null)
            {
                return false;
            }

            // 







            IEditableObject editableDataItem = dataItem as IEditableObject;
            if (editableDataItem != null)
            {
                editableDataItem.BeginEdit();
                return true;
            }

            //

            return true;
        }

        /* 
















*/

        /// <summary>
        /// Cancels the current entity editing and exits the editing mode.
        /// </summary>
        /// <param name="dataItem">The entity being edited</param>
        /// <returns>True if a cancellation operation was invoked.</returns>
        //
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Performance", "CA1822:MarkMembersAsStatic")]
        public bool CancelEdit(object dataItem)
        {
            // 







            IEditableObject editableDataItem = dataItem as IEditableObject;
            if (editableDataItem != null)
            {
                editableDataItem.CancelEdit();
                return true;
            }

            return true;
        }

        public static bool CanEdit(Type type)
        {
            Debug.Assert(type != null);

            type = type.GetNonNullableType();

            return
                type.IsEnum
                || type == typeof(System.String)
                || type == typeof(System.Char)
                || type == typeof(System.DateTime)
                || type == typeof(System.Boolean)
                || type == typeof(System.Byte)
                || type == typeof(System.SByte)
                || type == typeof(System.Single)
                || type == typeof(System.Double)
                || type == typeof(System.Decimal)
                || type == typeof(System.Int16)
                || type == typeof(System.Int32)
                || type == typeof(System.Int64)
                || type == typeof(System.UInt16)
                || type == typeof(System.UInt32)
                || type == typeof(System.UInt64);
        }

        /// <summary>
        /// Commits the current entity editing and exits the editing mode.
        /// </summary>
        /// <param name="dataItem">The entity being edited</param>
        /// <returns>True if a commit operation was invoked.</returns>
        //
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Performance", "CA1822:MarkMembersAsStatic")]
        public bool EndEdit(object dataItem)
        {
            // 







            IEditableObject editableDataItem = dataItem as IEditableObject;
            if (editableDataItem != null)
            {
                // BeginInvoke this so the Binding has a chance to update the item's value before EndEdit is called
                this._owner.Dispatcher.BeginInvoke(delegate
                {
                    editableDataItem.EndEdit();
                });
                return true;
            }

            return true;
        }

        // Assumes index >= 0, returns null if index >= Count
        public object GetDataItem(int index)
        {
            Debug.Assert(index >= 0);

            IList list = this.List;
            if (list != null)
            {
                return (index < list.Count) ? list[index] : null;
            }

            ListCollectionView lcv = this.DataSource as ListCollectionView;
            if (lcv != null)
            {
                return lcv.GetItemAt(index);
            }

            IEnumerable enumerable = this.DataSource;
            if (enumerable != null)
            {
                IEnumerator enumerator = enumerable.GetEnumerator();
                int i = -1;
                while (enumerator.MoveNext() && i < index)
                {
                    i++;
                    if (i == index)
                    {
                        return enumerator.Current;
                    }
                }
            }
            return null;
        }

        public bool GetPropertyIsReadOnly(string propertyName)
        {
            Debug.Assert(!string.IsNullOrEmpty(propertyName));
            if (this.DataProperties != null)
            {
                foreach (PropertyInfo propertyInfo in this._dataProperties)
                {
                    if (String.Compare(propertyName, propertyInfo.Name, StringComparison.OrdinalIgnoreCase) == 0)
                    {
                        return !propertyInfo.CanWrite || !this.AllowEdit;
                    }
                }
            }
            return false;
        }

        // 






        public int IndexOf(object dataItem)
        {
            IList list = this.List;
            if (list != null)
            {
                return list.IndexOf(dataItem);
            }

            ListCollectionView lcv = this.DataSource as ListCollectionView;
            if (lcv != null)
            {
                return lcv.IndexOf(dataItem);
            }

            IEnumerable enumerable = this.DataSource;
            if (enumerable != null)
            {
                int index = 0;
                foreach (object dataItemTmp in enumerable)
                {
                    if ((dataItem == null && dataItemTmp == null) ||
                        dataItem.Equals(dataItemTmp))
                    {
                        return index;
                    }
                    index++;
                }
            }
            return -1;
        }

        // 






        #endregion Public methods

        #region Internal Methods

        internal void ClearDataProperties()
        {
            this._dataProperties = null;
        }

        internal static bool DataTypeIsPrimitive(Type dataType)
        {
            if (dataType != null)
            {
                return dataType.IsPrimitive || dataType == typeof(string) || dataType == typeof(DateTime) || dataType == typeof(Decimal);
            }
            else
            {
                return false;
            }
        }

        internal void UnWireEvents(IEnumerable value)
        {
            INotifyCollectionChanged notifyingDataSource = value as INotifyCollectionChanged;
            if (notifyingDataSource != null)
            {
                notifyingDataSource.CollectionChanged -= new NotifyCollectionChangedEventHandler(NotifyingDataSource_CollectionChanged);
            }

            if (this.SortDescriptions != null)
            {
                ((INotifyCollectionChanged)this.SortDescriptions).CollectionChanged -= new NotifyCollectionChangedEventHandler(CollectionView_SortDescriptions_CollectionChanged);
            }
        }

        internal void WireEvents(IEnumerable value)
        {
            INotifyCollectionChanged notifyingDataSource = value as INotifyCollectionChanged;
            if (notifyingDataSource != null)
            {
                notifyingDataSource.CollectionChanged += new NotifyCollectionChangedEventHandler(NotifyingDataSource_CollectionChanged);
            }

            if (this.SortDescriptions != null)
            {
                ((INotifyCollectionChanged)this.SortDescriptions).CollectionChanged += new NotifyCollectionChangedEventHandler(CollectionView_SortDescriptions_CollectionChanged);
            }
        }

        #endregion Internal Methods

        #region Private methods

        private void CollectionView_SortDescriptions_CollectionChanged(object sender, NotifyCollectionChangedEventArgs e)
        {
            if (this._owner.ColumnsItemsInternal.Count == 0)
            {
                return;
            }

            // refresh sort description
            foreach (DataGridColumn column in this._owner.ColumnsItemsInternal)
            {
                column.HeaderCell.ApplyState();
            }
        }

        private void NotifyingDataSource_CollectionChanged(object sender, NotifyCollectionChangedEventArgs e)
        {
            if (this._owner.LoadingOrUnloadingRow)
            {
                throw DataGridError.DataGrid.CannotChangeItemsWhenLoadingRows();
            }
            switch (e.Action)
            {
                case NotifyCollectionChangedAction.Add:
                    IList addedItems = e.NewItems;
                    if (addedItems == null)
                    {
                        Debug.Assert(false, "Unexpected NotifyCollectionChangedAction.Add notification");
                        return;
                    }
                    if (this._owner.AutoGenerateColumns && this._owner.ColumnsInternal.AutogeneratedColumnCount == 0 && this.DataProperties.Length > 0)
                    {
                        this._owner.RefreshRowsAndColumns();
                    }
                    else
                    {
                        this._owner.InsertRows(e.NewStartingIndex, e.NewItems.Count);
                    }
                    break;
                case NotifyCollectionChangedAction.Remove:
                    IList removedItems = e.OldItems;
                    if (removedItems == null || e.OldStartingIndex < 0)
                    {
                        Debug.Assert(false, "Unexpected NotifyCollectionChangedAction.Remove notification");
                        return;
                    }
                    // According to WPF, Remove is a single item operation
                    foreach (object item in e.OldItems)
                    {
                        Debug.Assert(item != null);
                        this._owner.RemoveRowAt(e.OldStartingIndex, item);
                    }
                    break;
                case NotifyCollectionChangedAction.Replace:
                    throw new NotSupportedException(); // 

                case NotifyCollectionChangedAction.Reset:
                    // Did the data properties change during the reset?  If not, we can recycle
                    // the existing rows instead of having to clear them all
                    bool dataPropertiesAreEqual = false;
                    PropertyInfo[] previousDataProperties = this._dataProperties;
                    ClearDataProperties();
                    if (previousDataProperties != null && this.DataProperties != null &&
                        previousDataProperties.Length == this.DataProperties.Length)
                    {
                        dataPropertiesAreEqual = true;
                        for (int i = 0; i < previousDataProperties.Length; i++)
                        {
                            if (!previousDataProperties[i].Equals(this.DataProperties[i]))
                            {
                                dataPropertiesAreEqual = false;
                                break;
                            }
                        }
                    }
                    this._owner.RefreshRows(dataPropertiesAreEqual /*recycleRows*/);
                    break;
            }
        }

        #endregion Private Methods
    }
}
