// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Collections.ObjectModel;
using System.Threading;

namespace System.Windows.Controls
{
    /// <summary>
    /// Represents the collection of SelectedDates for the Calendar Control.
    /// </summary>
    public sealed class SelectedDatesCollection : ObservableCollection<DateTime>
    {
        #region Data
        private Collection<DateTime> _addedItems;
        private Thread _dispatcherThread;
        private bool _isCleared;
        private bool _isRangeAdded;
        private Calendar _owner;

        #endregion Data

        /// <summary>
        /// Initializes a new instance of the CalendarSelectedDatesCollection class.
        /// </summary>
        /// <param name="owner"></param>
        public SelectedDatesCollection(Calendar owner)
        {
            _owner = owner;
            this._addedItems = new Collection<DateTime>();
            this._dispatcherThread = Thread.CurrentThread;
        }

        #region Public methods

        /// <summary>
        /// Adds a range of dates to the Calendar SelectedDates.
        /// </summary>
        /// <param name="start"></param>
        /// <param name="end"></param>
        public void AddRange(DateTime start, DateTime end)
        {
            DateTime? rangeStart;
            //increment parameter specifies if the Days were selected in Descending order or Ascending order
            //based on this value, we add the days in the range either in Ascending order or in Descending order
            int increment = (DateTime.Compare(end, start) >= 0) ? 1 : -1;

            this._addedItems.Clear();

            rangeStart = start;
            _isRangeAdded = true;

            if (this._owner._isMouseSelection)
            {
                //In Mouse Selection we allow the user to be able to add multiple ranges in one action in MultipleRange Mode
                //In SingleRange Mode, we only add the first selected range
                while (rangeStart.HasValue && DateTime.Compare(end, rangeStart.Value) != -increment)
                {
                    if (Calendar.IsValidDateSelection(this._owner, rangeStart))
                    {
                        this.Add(rangeStart.Value);
                    }
                    else
                    {
                        if (this._owner.SelectionMode == CalendarSelectionMode.SingleRange)
                        {
                            this._owner.HoverEnd = rangeStart.Value.AddDays(-increment);
                            break;
                        }
                    }

                    rangeStart = DateTimeHelper.AddDays(rangeStart.Value, increment);
                }
            }
            else
            {
                //If CalendarSelectionMode.SingleRange and a user programmatically tries to add multiple ranges, we will throw away the old range and replace it with the new one.
                //in order to provide the removed items without an additional event, we are calling ClearInternal
                if (this._owner.SelectionMode == CalendarSelectionMode.SingleRange && this.Count > 0)
                {
                    foreach (DateTime item in this)
                    {
                        this._owner._removedItems.Add(item);
                    }
                    this.ClearInternal();
                }

                while (rangeStart.HasValue && DateTime.Compare(end, rangeStart.Value) != -increment)
                {
                    this.Add(rangeStart.Value);
                    rangeStart = DateTimeHelper.AddDays(rangeStart.Value, increment);
                }
            }

            _owner.OnSelectedDatesCollectionChanged(new SelectionChangedEventArgs(this._owner._removedItems, this._addedItems));
            this._owner._removedItems.Clear();
            this._owner.UpdateMonths();
            _isRangeAdded = false;
        }

        #endregion Public Methods

        #region Protected methods

        /// <summary>
        /// Clears all the items of the SelectedDates.
        /// </summary>
        protected override void ClearItems()
        {
            if (!IsValidThread())
            {
                throw new NotSupportedException(Resource.CalendarCollection_MultiThreadedCollectionChangeNotSupported);
            }
            Collection<DateTime> addedItems = new Collection<DateTime>();
            Collection<DateTime> removedItems = new Collection<DateTime>();

            foreach (DateTime item in this)
            {
                removedItems.Add(item);
            }

            base.ClearItems();

            //The event fires after SelectedDate changes
            if (this._owner.SelectionMode != CalendarSelectionMode.None)
            {
                this._owner.SelectedDate = null;
            }

            if (removedItems.Count != 0)
            {
                _owner.OnSelectedDatesCollectionChanged(new SelectionChangedEventArgs(removedItems, addedItems));
            }
            this._owner.UpdateMonths();
        }

        /// <summary>
        /// Inserts the item in the specified position of the SelectedDates collection.
        /// </summary>
        /// <param name="index"></param>
        /// <param name="item"></param>
        protected override void InsertItem(int index, DateTime item)
        {
            if (!IsValidThread())
            {
                throw new NotSupportedException(Resource.CalendarCollection_MultiThreadedCollectionChangeNotSupported);
            }

            if (!this.Contains(item))
            {
                Collection<DateTime> addedItems = new Collection<DateTime>();

                if (CheckSelectionMode())
                {
                    if (Calendar.IsValidDateSelection(this._owner, item))
                    {
                        //If the Collection is cleared since it is SingleRange and it had another range
                        //set the index to 0
                        if (_isCleared)
                        {
                            index = 0;
                            _isCleared = false;
                        }

                        base.InsertItem(index, item);

                        //The event fires after SelectedDate changes
                        if (index == 0 && !(this._owner.SelectedDate.HasValue && DateTime.Compare(this._owner.SelectedDate.Value,item) == 0))
                        {
                            this._owner.SelectedDate = item;
                        }

                        if (!_isRangeAdded)
                        {
                            addedItems.Add(item);


                            _owner.OnSelectedDatesCollectionChanged(new SelectionChangedEventArgs(this._owner._removedItems, addedItems));
                            this._owner._removedItems.Clear();
                            int monthDifference = DateTimeHelper.CompareYearMonth(item, this._owner.DisplayDateInternal);

                            if (monthDifference < 2 && monthDifference > -2)
                            {
                                this._owner.UpdateMonths();
                            }
                        }
                        else
                        {
                            this._addedItems.Add(item);
                        }
                    }
                    else
                    {
                        throw new ArgumentOutOfRangeException(Resource.Calendar_OnSelectedDateChanged_InvalidValue);
                    }
                }
            }
        }

        /// <summary>
        /// Removes the item at the specified position.
        /// </summary>
        /// <param name="index"></param>
        protected override void RemoveItem(int index)
        {
            if (!IsValidThread())
            {
                throw new NotSupportedException(Resource.CalendarCollection_MultiThreadedCollectionChangeNotSupported);
            }

            if (index >= this.Count)
            {
                base.RemoveItem(index);
            }
            else
            {
                Collection<DateTime> addedItems = new Collection<DateTime>();
                Collection<DateTime> removedItems = new Collection<DateTime>();
                int monthDifference = DateTimeHelper.CompareYearMonth(this[index], this._owner.DisplayDateInternal);

                removedItems.Add(this[index]);
                base.RemoveItem(index);

                //The event fires after SelectedDate changes
                if (index == 0)
                {
                    if (Count > 0)
                    {
                        this._owner.SelectedDate = this[0];
                    }
                    else
                    {
                        this._owner.SelectedDate = null;
                    }
                }

                _owner.OnSelectedDatesCollectionChanged(new SelectionChangedEventArgs(removedItems, addedItems));

                if (monthDifference < 2 && monthDifference > -2)
                {
                    this._owner.UpdateMonths();
                }
            }
        }

        /// <summary>
        /// The object in the specified index is replaced with the provided item.
        /// </summary>
        /// <param name="index"></param>
        /// <param name="item"></param>
        protected override void SetItem(int index, DateTime item)
        {
            if (!IsValidThread())
            {
                throw new NotSupportedException(Resource.CalendarCollection_MultiThreadedCollectionChangeNotSupported);
            }

            if (!this.Contains(item))
            {
                Collection<DateTime> addedItems = new Collection<DateTime>();
                Collection<DateTime> removedItems = new Collection<DateTime>();

                if (index >= this.Count)
                {
                    base.SetItem(index, item);
                }
                else
                {
                    if (item != null && DateTime.Compare(this[index], item) != 0 && Calendar.IsValidDateSelection(this._owner, item))
                    {
                        removedItems.Add(this[index]);
                        base.SetItem(index, item);
                        addedItems.Add(item);

                        //The event fires after SelectedDate changes
                        if (index == 0 && !(this._owner.SelectedDate.HasValue && DateTime.Compare(this._owner.SelectedDate.Value,item) == 0))
                        {
                            this._owner.SelectedDate = item;
                        }
                        _owner.OnSelectedDatesCollectionChanged(new SelectionChangedEventArgs(removedItems, addedItems));

                        int monthDifference = DateTimeHelper.CompareYearMonth(item, this._owner.DisplayDateInternal);

                        if (monthDifference < 2 && monthDifference > -2)
                        {
                            this._owner.UpdateMonths();
                        }
                    }
                }
            }
        }

        #endregion Protected methods

        #region Internal Methods

        internal void ClearInternal()
        {
            base.ClearItems();
        }

        #endregion Internal Methods

        #region Private Methods

        private bool CheckSelectionMode()
        {
            if (this._owner.SelectionMode == CalendarSelectionMode.None)
            {
                throw new InvalidOperationException(Resource.Calendar_OnSelectedDateChanged_InvalidOperation);
            }
            if (this._owner.SelectionMode == CalendarSelectionMode.SingleDate && this.Count > 0)
            {
                throw new InvalidOperationException(Resource.Calendar_CheckSelectionMode_InvalidOperation);
            }
            //if user tries to add an item into the SelectedDates in SingleRange mode, we throw away the old range and replace it with the new one
            //in order to provide the removed items without an additional event, we are calling ClearInternal
            if (this._owner.SelectionMode == CalendarSelectionMode.SingleRange && !_isRangeAdded && this.Count > 0)
            {
                foreach (DateTime item in this)
                {
                    this._owner._removedItems.Add(item);
                }
                this.ClearInternal();
                _isCleared = true;
            }
            return true;
        }

        private bool IsValidThread()
        {
            return (Thread.CurrentThread == this._dispatcherThread);
        }

        #endregion Private Methods

    }
}
