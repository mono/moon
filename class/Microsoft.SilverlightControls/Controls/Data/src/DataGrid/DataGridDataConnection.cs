// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System.Collections;
using System.Collections.Specialized; 
using System.Diagnostics; 
using System.Reflection;
 
namespace System.Windows.Controlsb1
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

        public int Count 
        {
            get
            { 
                IList list = this.List;
                if (list != null)
                { 
                    return list.Count; 
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
                object dataItem = this.GetFirstDataItem();

                return DataItemIsPrimitive(dataItem); 
            } 
        }
 
        public PropertyInfo[] DataProperties
        {
            get 
            {
                if (_dataProperties == null && this.DataSource != null)
                { 
                    object dataItem = this.GetFirstDataItem(); 

                    if (dataItem != null && !DataItemIsPrimitive(dataItem)) 
                    {
                        _dataProperties = dataItem.GetType().GetProperties(BindingFlags.Public | BindingFlags.Instance);
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
            get 
            {
                return this._owner.ItemsSource; 
            }
            set
            { 
                this._owner.ItemsSource = value;
            }
        } 
 
        public Type DataType
        { 
            get
            {
                object dataItem = this.GetFirstDataItem(); 

                if (dataItem != null)
                { 
                    return dataItem.GetType(); 
                }
                else 
                {
                    return null;
                } 
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
                editableDataItem.EndEdit();
                return true;
            } 
 
            return true;
        } 

        public object GetDataItem(int index)
        { 
            IList list = this.List;
            if (list != null)
            { 
                return list[index]; 
            }
            IEnumerable enumerable = this.DataSource; 
            if (enumerable != null)
            {
                IEnumerator enumerator = enumerable.GetEnumerator(); 
                for (int indexTmp = 0; indexTmp <= index; indexTmp++)
                {
                    enumerator.MoveNext(); 
                } 
                return enumerator.Current;
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
 
        internal void UnWireEvents(IEnumerable value)
        {
            INotifyCollectionChanged notifyingDataSource = value as INotifyCollectionChanged; 
            if (notifyingDataSource != null) 
            {
                notifyingDataSource.CollectionChanged -= new NotifyCollectionChangedEventHandler(NotifyingDataSource_CollectionChanged); 
            }
        }
 
        internal void WireEvents(IEnumerable value)
        {
            INotifyCollectionChanged notifyingDataSource = value as INotifyCollectionChanged; 
            if (notifyingDataSource != null) 
            {
                notifyingDataSource.CollectionChanged += new NotifyCollectionChangedEventHandler(NotifyingDataSource_CollectionChanged); 
            }
        }
 
        #endregion Internal Methods

        #region Private methods 
 
        private static bool DataItemIsPrimitive(object dataItem)
        { 
            if (dataItem != null)
            {
                Type type = dataItem.GetType(); 

                return type.IsPrimitive || type == typeof(string) || type == typeof(DateTime) || type == typeof(Decimal);
            } 
            else 
            {
                return false; 
            }
        }
 
        private object GetFirstDataItem()
        {
            if (DataSource == null) 
            { 
                return null;
            } 

            // Get the first data item in the DataSource
            IEnumerator enumerator = DataSource.GetEnumerator(); 
            enumerator.MoveNext();

            // Get properties based on the first object 
            return enumerator.Current; 
        }
 
        private void NotifyingDataSource_CollectionChanged(object sender, NotifyCollectionChangedEventArgs e)
        {
            if (this._owner.ColumnsItemsInternal.Count == 0) 
            {
                return;
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
                    this._owner.InsertRows(e.NewStartingIndex, e.NewItems.Count); 
                    break;
                case NotifyCollectionChangedAction.Remove: 
                    IList removedItems = e.OldItems;
                    if (removedItems == null)
                    { 
                        Debug.Assert(false, "Unexpected NotifyCollectionChangedAction.Remove notification");
                        return;
                    } 
                    if (e.OldStartingIndex != -1) 
                    {
                        for (int rowCount = 0; rowCount < removedItems.Count; rowCount++) 
                        {
                            this._owner.RemoveRowAt(e.OldStartingIndex);
                        } 
                    }
                    break;
                case NotifyCollectionChangedAction.Replace: 
                    throw new NotSupportedException(); // 

                case NotifyCollectionChangedAction.Reset: 
                    this._owner.RefreshRows();
                    break;
            } 
        }

        #endregion Private Methods 
    } 
}
