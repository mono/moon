// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Collections;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Diagnostics;
using System.Globalization;
using System.Linq;
using System.Reflection;
using System.Threading;

namespace System.Windows.Controls
{
    internal class ListCollectionView : ICollectionView, INotifyCollectionChanged, INotifyPropertyChanged, IEnumerable
    {
        #region Constructors

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="list">Underlying IList</param>
        public ListCollectionView(IList list)
        {
            _dispatcherThread = Thread.CurrentThread;
            _sourceCollection = list;
            _internalList = list;
            _elementType = list.GetItemType();

            if (list.Count > 0)
            {
                _currentItem = list[0];
            }

            // forward collection change events from underlying collection to our listeners.
            INotifyCollectionChanged incc = list as INotifyCollectionChanged;
            if (incc != null)
            {
                incc.CollectionChanged += new NotifyCollectionChangedEventHandler(OnCollectionChanged);
            }
        }

        #endregion Constructors

        #region ICollectionView Members

        public bool CanFilter
        {
            get { return false; }
        }

        public bool CanGroup
        {
            get
            {
                return false;
            }
        }

        public bool CanSort
        {
            get
            {
                return (true);
            }
        }

        public bool Contains(object item)
        {
            return _internalList.Contains(item);
        }

        public virtual CultureInfo Culture
        {
            get { return _culture; }
            set
            {
                if (value == null)
                    throw new ArgumentNullException("value");

                if (_culture != value)
                {
                    _culture = value;
                    OnPropertyChanged(CulturePropertyName);
                }
            }
        }

        public event EventHandler CurrentChanged;

        public event CurrentChangingEventHandler CurrentChanging;

        public object CurrentItem
        {
            get
            {
                return _currentItem;
            }
        }

        public int CurrentPosition
        {
            get
            {
                return _currentPosition;
            }
        }

        public virtual IDisposable DeferRefresh()
        {
            throw new NotImplementedException();
        }

        public Predicate<object> Filter
        {
            get
            {
                return _filter;
            }
            set
            {
                _filter = value;
            }
        }

        public System.Collections.ObjectModel.ObservableCollection<GroupDescription> GroupDescriptions
        {
            get { throw new NotImplementedException(); }
        }

        public System.Collections.ObjectModel.ReadOnlyObservableCollection<object> Groups
        {
            get { throw new NotImplementedException(); }
        }

        public bool IsCurrentAfterLast
        {
            get { throw new NotImplementedException(); }
        }

        public bool IsCurrentBeforeFirst
        {
            get { throw new NotImplementedException(); }
        }

        public bool IsEmpty
        {
            get
            {
                return (_internalList.Count == 0);
            }
        }

        public bool MoveCurrentTo(object item)
        {
            throw new NotImplementedException();
        }

        public bool MoveCurrentToFirst()
        {
            throw new NotImplementedException();
        }

        public bool MoveCurrentToLast()
        {
            throw new NotImplementedException();
        }

        public bool MoveCurrentToNext()
        {
            throw new NotImplementedException();
        }

        public bool MoveCurrentToPosition(int position)
        {
            throw new NotImplementedException();
        }

        public bool MoveCurrentToPrevious()
        {
            throw new NotImplementedException();
        }

        public void Refresh()
        {
            // if there's no sort/filter, just use the collection's array
            if (UsesLocalArray)
            {
                MethodInfo mi = (typeof(ListCollectionView)).GetMethod("PrepareLocalArray", BindingFlags.Instance | BindingFlags.NonPublic);
                mi = mi.MakeGenericMethod(_elementType);
                try
                {
                    _internalList = (IList)mi.Invoke(this, new object[] { _internalList, _internalList != null });
                }
                catch (TargetInvocationException e)
                {
                    // If there's an exception while invoking PrepareLocalArray,
                    // we want to unwrap it and throw its inner exception
                    if (e.InnerException != null)
                    {
                        throw e.InnerException;
                    }
                    else
                    {
                        throw;
                    }
                }
            }

            OnCollectionChanged(new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Reset));

        }

        public SortDescriptionCollection SortDescriptions
        {
            get
            {
                if (_sort == null)
                {
                    SetSortDescriptions(new SortDescriptionCollection());
                }
                return _sort;
            }
        }

        public System.Collections.IEnumerable SourceCollection
        {
            get
            {
                return _sourceCollection;
            }
        }

        #endregion

        #region IEnumerable Members

        public System.Collections.IEnumerator GetEnumerator()
        {
            return _internalList.GetEnumerator();
        }

        #endregion

        #region INotifyCollectionChanged Members

        public event System.Collections.Specialized.NotifyCollectionChangedEventHandler CollectionChanged;

        #endregion

        #region INotifyPropertyChanged Members
        public virtual event PropertyChangedEventHandler PropertyChanged;
        #endregion

        #region Internal Fields
        internal const string CountPropertyName = "Count";
        internal const string CulturePropertyName = "Culture";
        internal const string CurrentItemPropertyName = "CurrentItem";
        internal const string CurrentPositionPropertyName = "CurrentPosition";
        internal const string IsCurrentAfterLastPropertyName = "IsCurrentAfterLast";
        internal const string IsCurrentBeforeFirstPropertyName = "IsCurrentBeforeFirst";
        internal const string IsEmptyPropertyName = "IsEmpty";
        internal const char PropertyNameSeparator = '.';
        #endregion

        #region Public Properties

        public virtual int Count
        {
            get
            {
                return _internalList.Count;
            }
        }

        #endregion Public Properties

        #region Public Methods

        public virtual object GetItemAt(int index)
        {
            if (index < 0 || index >= this._internalList.Count)
            {
                // 


                return null;
            }

            return _internalList[index];
        }

        public virtual int IndexOf(object item)
        {
            return _internalList.IndexOf(item);
        }

        #endregion Public Methods

        #region Protected Methods

        /// <summary>
        /// Protected accessor to private _activeComparer field.
        /// </summary>
        protected IComparer ActiveComparer
        {
            get { return _activeComparer; }
            set
            {
                _activeComparer = value;
            }
        }

        /// <summary>
        /// Protected accessor to private _internalList field.
        /// </summary>
        protected IList InternalList
        {
            get { return _internalList; }
        }

        /// <summary>
        ///     Notify listeners that this View has changed
        /// </summary>
        /// <remarks>
        ///     CollectionViews (and sub-classes) should take their filter/sort/grouping
        ///     into account before calling this method to forward CollectionChanged events.
        /// </remarks>
        /// <param name="args">
        ///     The NotifyCollectionChangedEventArgs to be passed to the EventHandler
        /// </param>
        protected virtual void OnCollectionChanged(NotifyCollectionChangedEventArgs args)
        {
            if (args == null)
                throw new ArgumentNullException("args");

            if (CollectionChanged != null)
                CollectionChanged(this, args);

            // Collection changes change the count unless an item is being
            // replaced or moved within the collection.
            if (args.Action != NotifyCollectionChangedAction.Replace)
            {
                OnPropertyChanged(CountPropertyName);
            }
        }

        ///<summary>
        ///     Handle CollectionChanged events.
        ///
        ///     Calls ProcessCollectionChanged() or
        ///     posts the change to the Dispatcher to process on the correct thread.
        ///</summary>
        /// <remarks>
        ///     User should override <see cref="ProcessCollectionChanged"/>
        /// </remarks>
        /// <param name="sender">
        /// </param>
        /// <param name="args">
        /// </param>
        protected void OnCollectionChanged(object sender, NotifyCollectionChangedEventArgs args)
        {
            if (!IsValidThreadAccess())
            {
                throw CollectionViewError.ListCollectionView.MultiThreadedCollectionChangeNotSupported();
            }

            ProcessCollectionChanged(args);
        }

        /// <summary>
        /// Raises the CurrentChanged event
        /// </summary>
        protected virtual void OnCurrentChanged()
        {
            if (CurrentChanged != null)
            {
                CurrentChanged(this, EventArgs.Empty);
            }
        }

        /// <summary>
        /// Raise a CurrentChanging event that is not cancelable.
        /// Internally, CurrentPosition is set to -1.
        /// This is called by CollectionChanges (Remove and Refresh) that affect the CurrentItem.
        /// </summary>
        /// <exception cref="InvalidOperationException">
        /// This CurrentChanging event cannot be canceled.
        /// </exception>
        protected void OnCurrentChanging()
        {
            _currentPosition = -1;
            OnCurrentChanging(uncancelableCurrentChangingEventArgs);
        }

        /// <summary>
        /// Raises the CurrentChanging event
        /// </summary>
        /// <param name="args">
        ///     CancelEventArgs used by the consumer of the event.  args.Cancel will
        ///     be true after this call if the CurrentItem should not be changed for
        ///     any reason.
        /// </param>
        /// <exception cref="InvalidOperationException">
        ///     This CurrentChanging event cannot be canceled.
        /// </exception>
        protected virtual void OnCurrentChanging(CurrentChangingEventArgs args)
        {
            if (args == null)
                throw new ArgumentNullException("args");

            if (CurrentChanging != null)
            {
                CurrentChanging(this, args);
            }
        }

        /// <summary>
        /// Raises a PropertyChanged event.
        /// </summary>
        protected virtual void OnPropertyChanged(PropertyChangedEventArgs e)
        {
            if (PropertyChanged != null)
            {
                PropertyChanged(this, e);
            }
        }

        /// <summary>
        /// Handle CollectionChange events
        /// </summary>
        protected void ProcessCollectionChanged(NotifyCollectionChangedEventArgs args)
        {
            if (args == null)
                throw new ArgumentNullException("args");

            ValidateCollectionChangedEventArgs(args);

            int adjustedOldIndex = -1;
            int adjustedNewIndex = -1;

            // If the Action is Reset then we do a Refresh.
            if (args.Action == NotifyCollectionChangedAction.Reset)
            {
                RefreshOrDefer();
                return; // the Refresh raises collection change event, so there's nothing left to do
            }

            #region check this for currency changes and specific add and deletes
            // If the Action is one that can be expected to have a valid NewItems[0] and NewStartingIndex then
            // adjust the index for filtering and sorting.
            if (args.Action != NotifyCollectionChangedAction.Remove)
            {
                adjustedNewIndex = AdjustBefore(NotifyCollectionChangedAction.Add, args.NewItems[0], args.NewStartingIndex);
            }

            // If the Action is one that can be expected to have a valid OldItems[0] and OldStartingIndex then
            // adjust the index for filtering and sorting.
            if (args.Action != NotifyCollectionChangedAction.Add)
            {
                adjustedOldIndex = AdjustBefore(NotifyCollectionChangedAction.Remove, args.OldItems[0], args.OldStartingIndex);

                // the new index needs further adjustment if the action removes (or moves)
                // something before it
                if (UsesLocalArray && adjustedOldIndex >= 0 && adjustedOldIndex < adjustedNewIndex)
                {
                    --adjustedNewIndex;
                }
            }

            // handle interaction with AddNew and EditItem
            switch (args.Action)
            {
                case NotifyCollectionChangedAction.Add:
                    if (args.NewStartingIndex <= _newItemIndex)
                    {
                        ++_newItemIndex;
                    }
                    break;

                case NotifyCollectionChangedAction.Remove:
                    if (args.OldStartingIndex < _newItemIndex)
                    {
                        --_newItemIndex;
                    }

                    break;
            }

            ProcessCollectionChangedWithAdjustedIndex(args, adjustedOldIndex, adjustedNewIndex);
            #endregion
        }
        protected void ProcessCollectionChangedWithAdjustedIndex(NotifyCollectionChangedEventArgs args, int adjustedOldIndex, int adjustedNewIndex)
        {
            // Finding out the effective Action after filtering and sorting.
            //
            NotifyCollectionChangedAction effectiveAction = args.Action;
            if (adjustedOldIndex == adjustedNewIndex && adjustedOldIndex >= 0)
            {
                effectiveAction = NotifyCollectionChangedAction.Replace;
            }
            else if (adjustedOldIndex == -1) // old index is unknown
            {
                // we weren't told the old index, but it may have been in the view.
                if (adjustedNewIndex < 0)
                {
                    // The new item will not be in the filtered view,
                    // so an Add is a no-op and anything else is a Remove.
                    if (args.Action == NotifyCollectionChangedAction.Add)
                        return;
                    effectiveAction = NotifyCollectionChangedAction.Remove;
                }
            }
            else if (adjustedOldIndex < -1) // old item is known to be NOT in filtered view
            {
                if (adjustedNewIndex < 0)
                {
                    // since the old item wasn't in the filtered view, and the new
                    // item would not be in the filtered view, this is a no-op.
                    return;
                }
                else
                {
                    effectiveAction = NotifyCollectionChangedAction.Add;
                }
            }
            else // old item was in view
            {
                if (adjustedNewIndex < 0)
                {
                    effectiveAction = NotifyCollectionChangedAction.Remove;
                }
            }

            // in the case of a replace that has a new adjustedPosition
            // (likely caused by sorting), the only way to effectively communicate
            // this change is through raising Remove followed by Insert.
            NotifyCollectionChangedEventArgs args2 = null;

            switch (effectiveAction)
            {
                case NotifyCollectionChangedAction.Add:
                    // insert into private view
                    // (unless it's a special item - placeholder or new item)
                    if (UsesLocalArray)
                    {
                        InternalList.Insert(adjustedNewIndex, args.NewItems[0]);
                    }

                    args = new NotifyCollectionChangedEventArgs(effectiveAction, args.NewItems[0], adjustedNewIndex);

                    break;

                case NotifyCollectionChangedAction.Remove:
                    // remove from private view, unless it's not there to start with
                    // (e.g. when CommitNew is applied to an item that fails the filter)
                    if (UsesLocalArray)
                    {
                        int localOldIndex = adjustedOldIndex;

                        if (localOldIndex < InternalList.Count &&
                            Object.Equals(InternalList[localOldIndex], args.OldItems[0]))
                        {
                            InternalList.RemoveAt(localOldIndex);
                        }
                    }

                    args = new NotifyCollectionChangedEventArgs(effectiveAction, args.OldItems[0], adjustedOldIndex);
                    break;
                case NotifyCollectionChangedAction.Replace:
                    // replace item in private view
                    if (UsesLocalArray)
                    {
                        InternalList[adjustedOldIndex] = args.NewItems[0];
                    }

                    args = new NotifyCollectionChangedEventArgs(effectiveAction, args.NewItems[0], args.OldItems[0], adjustedOldIndex);
                    break;

                default:
                    Debug.Assert(
                        false,
                        String.Format(
                            CultureInfo.CurrentCulture,
                            Resource.ListCollectionView_UnexpectedCollectionChangeAction,
                            effectiveAction));
                    break;
            }

            // we've already returned if (args.Action == NotifyCollectionChangedAction.Reset) above
            OnCollectionChanged(args);
            if (args2 != null)
                OnCollectionChanged(args2);

        }

        /// <summary>
        ///     Refresh, or mark that refresh is needed when defer cycle completes.
        /// </summary>
        protected void RefreshOrDefer()
        {
            Refresh();
        }

        protected bool UsesLocalArray
        {
            get { return (SortDescriptions != null && SortDescriptions.Count != 0); }
        }

        #endregion //Protected Methods

        #region Internal Methods

        /// <summary>
        /// Helper for SortList to handle nested properties (i.e. Address.Street)
        /// </summary>
        /// <param name="item">parent object</param>
        /// <param name="propertyPath">property names path</param>
        /// <returns>child object</returns>
        internal static object InvokePath(object item, string propertyPath)
        {
            object newItem = item;
            string[] propertyNames = propertyPath.Split(PropertyNameSeparator);
            for (int i = 0; i < propertyNames.Length; i++)
            {
                if (newItem == null)
                {
                    throw CollectionViewError.ListCollectionView.InvalidPropertyName(propertyNames[i]);
                }

                newItem = newItem.GetType().InvokeMember(propertyNames[i], System.Reflection.BindingFlags.GetProperty, null, newItem, null);
            }
            return newItem;
        }

        #endregion

        #region Private Methods
        // Convert the collection's index to an index into the view.
        // Return -1 if the index is unknown or moot (Reset events).
        // Return -2 if the event doesn't apply to this view.
        private int AdjustBefore(NotifyCollectionChangedAction action, object item, int index)
        {
            // index is not relevant to Reset events
            if (action == NotifyCollectionChangedAction.Reset)
                return -1;

            IList ilFull = SourceCollection as IList;

            // validate input
            if (index < -1 || index > ilFull.Count)
                throw CollectionViewError.ListCollectionView.CollectionChangeIndexOutOfRange(index, ilFull.Count);

            if (action == NotifyCollectionChangedAction.Add)
            {
                if (index >= 0)
                {
                    if (!Object.Equals(item, ilFull[index]))
                        throw CollectionViewError.ListCollectionView.AddedItemNotAtIndex(index);
                }
                else
                {
                    // event didn't specify index - determine it the hard way
                    index = ilFull.IndexOf(item);
                    if (index < 0)
                        throw CollectionViewError.ListCollectionView.AddedItemNotInCollection();
                }
            }

            // if there's no sort or filter, use the index into the full array
            if (!UsesLocalArray)
            {
                return index;
            }


            if (action == NotifyCollectionChangedAction.Add)
            {
                // search the local array.  If there's no sort order, use the index
                // in the full array as the sort key.
                IComparer comparer = (ActiveComparer != null) ? ActiveComparer
                                        : new ListOrdinalComparer(ilFull, item, index);

                // Treat _internalList as a List<_elementType> and do a BinarySearch on it
                MethodInfo mi = (typeof(ListCollectionView)).GetMethod("BinarySearch", BindingFlags.Instance | BindingFlags.NonPublic);
                mi = mi.MakeGenericMethod(_elementType);
                index = (int)mi.Invoke(this, new object[] { item, comparer });

                // If the item is not in the list, BinarySearch returns the bitwise complement
                // of the index where it would have been
                if (index < 0)
                {
                    index = ~index;
                }                
            }
            else if (action == NotifyCollectionChangedAction.Remove)
            {
                // a deleted item should already be in the local array
                index = InternalList.IndexOf(item);
            }
            else
            {
                index = -1;
            }

            return index;
        }

        /// <summary>
        /// Allows a BinarySearch to be performed on _internalList, an IList
        /// </summary>
        /// <typeparam name="T">type of elements in list</typeparam>
        /// <param name="item">item to be searched for</param>
        /// <param name="comparer">comparer</param>
        /// <returns>index of item or bitwise complement of the index of the next larger element</returns>
        private int BinarySearch<T>(T item, SortFieldComparer<T> comparer)
        {
            List<T> tempList = (List<T>)_internalList;
            return tempList.BinarySearch(item, comparer);
        }

        private bool IsValidThreadAccess()
        {
            return (Thread.CurrentThread == this._dispatcherThread);
        }

        /// <summary>
        /// Helper to raise a PropertyChanged event  />).
        /// </summary>
        private void OnPropertyChanged(string propertyName)
        {
            OnPropertyChanged(new PropertyChangedEventArgs(propertyName));
        }

        /// <summary>
        /// Create, filter and sort the local index array.
        /// called from Refresh(), override in derived classes as needed.
        /// </summary>
        /// <param name="list">new ILIst to associate this view with</param>
        /// <param name="createNewList">indicates whether a new list needs to be created</param>
        /// <returns>new local array to use for this view</returns>
        private IList PrepareLocalArray<T>(IList list, bool createNewList)
        {
            if (list == null)
                throw new ArgumentNullException("list");

            List<T> tempList;

            if (createNewList)
            {
                tempList = new List<T>();

                foreach (T obj in _sourceCollection)
                {
                    tempList.Add(obj);
                }
            }
            else
            {
                tempList = (List<T>)list;
            }

            // sort the local array based on the Comparer. 
            if (SortDescriptions != null && SortDescriptions.Count != 0)
            {
                tempList = SortList(tempList);
            }

            return tempList;
        }

        // set new SortDescription collection; rehook collection change notification handler
        private void SetSortDescriptions(SortDescriptionCollection descriptions)
        {
            if (_sort != null)
            {
                ((INotifyCollectionChanged)_sort).CollectionChanged -= new NotifyCollectionChangedEventHandler(SortDescriptionsChanged);
            }

            _sort = descriptions;

            if (_sort != null)
            {
                System.Diagnostics.Debug.Assert(_sort.Count == 0, "must be empty SortDescription collection");
                ((INotifyCollectionChanged)_sort).CollectionChanged += new NotifyCollectionChangedEventHandler(SortDescriptionsChanged);
            }
        }

        // SortDescription was added/removed, refresh CollectionView
        private void SortDescriptionsChanged(object sender, NotifyCollectionChangedEventArgs e)
        {
            if (_elementType == null && _internalList != null)
            {
                _elementType = _internalList.GetItemType();
            }

            //Ignore on reset when the collection is being cleared no reason to go back to default sort.
            if (!(e.Action == NotifyCollectionChangedAction.Reset &&
                e.NewItems == null && e.NewStartingIndex == -1 &&
                e.OldItems == null && e.OldStartingIndex == -1) &&
                _elementType != null)
            {
                Type comparerType = typeof(SortFieldComparer<>).MakeGenericType(new Type[] { _elementType });
                ConstructorInfo constructor = comparerType.GetConstructor(new Type[] { typeof(SortDescriptionCollection) });
                ActiveComparer = (IComparer)constructor.Invoke(new object[] { (SortDescriptionCollection)sender });
                Refresh();
            }
        }

        private List<T> SortList<T>(List<T> list)
        {
            //Need to create temporary storage the first time so that the bound list is not sorted.
            if (list == null)
                throw new ArgumentNullException("list");

            IEnumerable<T> seq = (IEnumerable<T>)list;

            foreach (System.ComponentModel.SortDescription sort in this.SortDescriptions)
            {
                Debug.Assert(!String.IsNullOrEmpty(sort.PropertyName), "PropertyName must exist");

                // get the type of the property we will be sorting on
                string propertyPath = sort.PropertyName;
                Type propertyType = typeof(T).GetNestedPropertyType(propertyPath);
                if (propertyType == null)
                {
                    throw CollectionViewError.ListCollectionView.InvalidPropertyName(propertyPath);
                }

                // if the type is Nullable, then we want the non-nullable type
                if (TypeHelper.IsNullableType(propertyType))
                {
                    propertyType = TypeHelper.GetNonNullableType(propertyType);
                }

                // make sure the property type can be compared
                if (!typeof(IComparable).IsAssignableFrom(propertyType))
                {
                    throw CollectionViewError.ListCollectionView.FailedToCompareElements();
                }

                IOrderedEnumerable<T> seqOrdered = seq as IOrderedEnumerable<T>;
                switch (sort.Direction)
                {
                    case System.ComponentModel.ListSortDirection.Ascending:
                        if (seqOrdered != null)
                        {
                            // thenby
                            seq = seqOrdered.ThenBy(item => InvokePath(item, propertyPath));
                        }
                        else
                        {
                            // orderby
                            seq = seq.OrderBy(item => InvokePath(item, propertyPath));
                        }
                        break;
                    case System.ComponentModel.ListSortDirection.Descending:
                        if (seqOrdered != null)
                        {
                            // thenby
                            seq = seqOrdered.ThenByDescending(item => InvokePath(item, propertyPath));
                        }
                        else
                        {
                            // orderby
                            seq = seq.OrderByDescending(item => InvokePath(item, propertyPath));
                        }
                        break;
                    default:
                        break;
                }
            }
            return seq.ToList();
        }

        private static void ValidateCollectionChangedEventArgs(NotifyCollectionChangedEventArgs e)
        {

            switch (e.Action)
            {
                case NotifyCollectionChangedAction.Add:
                    if (e.NewItems.Count != 1)
                        throw CollectionViewError.ListCollectionView.RangeActionsNotSupported();
                    break;

                case NotifyCollectionChangedAction.Remove:
                    if (e.OldItems.Count != 1)
                        throw CollectionViewError.ListCollectionView.RangeActionsNotSupported();
                    break;

                case NotifyCollectionChangedAction.Replace:
                    if (e.NewItems.Count != 1 || e.OldItems.Count != 1)
                        throw CollectionViewError.ListCollectionView.RangeActionsNotSupported();
                    break;

                case NotifyCollectionChangedAction.Reset:
                    break;

                default:
                    throw CollectionViewError.ListCollectionView.UnexpectedCollectionChangeAction(e.Action);
            }
        }

        #endregion //Private Methods

        #region Private Types

        private class ListOrdinalComparer : IComparer
        {
            IList _ilFull;
            int _index;
            object _item;

            internal ListOrdinalComparer(IList ilFull, object item, int index)
            {
                _ilFull = ilFull;
                _item = item;
                _index = index;
            }

            // IComparer
            public int Compare(object o1, object o2)
            {
                int i1 = Object.Equals(o1, _item) ? _index : _ilFull.IndexOf(o1);
                int i2 = Object.Equals(o2, _item) ? _index : _ilFull.IndexOf(o2);
                return (i1 - i2);
            }
        }

        #endregion //Private Types

        #region Private Fields

        private IComparer _activeComparer;
        private CultureInfo _culture; // culture to use when sorting
        private object _currentItem;
        private int _currentPosition;
        private Thread _dispatcherThread;
        private Type _elementType;
        private Predicate<object> _filter;
        private IList _internalList; // Exposed list
        private int _newItemIndex;  // position _newItem in the source collection
        private SortDescriptionCollection _sort;
        private IList _sourceCollection; // Original list
        private static readonly CurrentChangingEventArgs uncancelableCurrentChangingEventArgs = new CurrentChangingEventArgs(false);
        private const int _unknownIndex = -1;

        #endregion Private Fields
    }
}
