// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Linq;
using System.Threading;

namespace System.Windows.Controls
{
    /// <summary>
    /// Represents a collection of DateTimeRanges.
    /// </summary>
    public sealed class CalendarBlackoutDatesCollection : ObservableCollection<CalendarDateRange>
    {
        #region Data

        private Thread _dispatcherThread;
        private Calendar _owner;

        #endregion Data

        /// <summary>
        /// Initializes a new instance of the CalendarBlackoutDatesCollection class.
        /// </summary>
        /// <param name="owner"></param>
        public CalendarBlackoutDatesCollection(Calendar owner)
        {
            _owner = owner;
            this._dispatcherThread = Thread.CurrentThread;
        }

        #region Public Methods

        /// <summary>
        /// Dates that are in the past are added to the BlackoutDates.
        /// </summary>
        public void AddDatesInPast()
        {
            this.Add(new CalendarDateRange(DateTime.MinValue, DateTime.Today.AddDays(-1)));
        }

        /// <summary>
        /// Checks if a DateTime is in the Collection
        /// </summary>
        /// <param name="date"></param>
        /// <returns></returns>
        public bool Contains(DateTime date)
        {
            for (int i = 0; i < Count; i++)
            {
                if (DateTimeHelper.InRange(date, this[i]))
                {
                    return true;
                }
            }
            return false;
        }

        /// <summary>
        /// Checks if a Range is in the collection
        /// </summary>
        /// <param name="start"></param>
        /// <param name="end"></param>
        /// <returns></returns>
        public bool Contains(DateTime start, DateTime end)
        {
            DateTime rangeStart, rangeEnd;
            int n = Count;

            if (DateTime.Compare(end, start) > -1)
            {
                rangeStart = DateTimeHelper.DiscardTime(start).Value;
                rangeEnd = DateTimeHelper.DiscardTime(end).Value;
            }
            else
            {
                rangeStart = DateTimeHelper.DiscardTime(end).Value;
                rangeEnd = DateTimeHelper.DiscardTime(start).Value;
            }

            for (int i = 0; i < n; i++)
            {
                if (DateTime.Compare(this[i].Start, rangeStart) == 0 && DateTime.Compare(this[i].End, rangeEnd) == 0)
                {
                    return true;
                }
            }
            return false;

        }

        /// <summary>
        /// Returns true if any day in the given DateTime range is contained in the BlackOutDays.
        /// </summary>
        /// <param name="range">CalendarDateRange that is searched in BlackOutDays</param>
        /// <returns>true if at least one day in the range is included in the BlackOutDays</returns>
        public bool ContainsAny(CalendarDateRange range)
        {
            return this.Any(CalendarDateRange => CalendarDateRange.ContainsAny(range));
        }

        #endregion Public Methods

        #region Protected Methods

        /// <summary>
        /// All the items in the collection are removed.
        /// </summary>
        protected override void ClearItems()
        {
            if (!IsValidThread())
            {
                throw new NotSupportedException(Resource.CalendarCollection_MultiThreadedCollectionChangeNotSupported);
            }
            base.ClearItems();
            _owner.UpdateMonths();
        }

        /// <summary>
        /// The item is inserted in the specified place in the collection.
        /// </summary>
        /// <param name="index"></param>
        /// <param name="item"></param>
        protected override void InsertItem(int index, CalendarDateRange item)
        {
            if (!IsValidThread())
            {
                throw new NotSupportedException(Resource.CalendarCollection_MultiThreadedCollectionChangeNotSupported);
            }

            if (IsValid(item))
            {
                base.InsertItem(index, item);
                _owner.UpdateMonths();
            }
            else
            {
                throw new ArgumentOutOfRangeException(Resource.Calendar_UnSelectableDates);
            }
        }

        /// <summary>
        /// The item in the specified index is removed from the collection.
        /// </summary>
        /// <param name="index"></param>
        protected override void RemoveItem(int index)
        {
            if (!IsValidThread())
            {
                throw new NotSupportedException(Resource.CalendarCollection_MultiThreadedCollectionChangeNotSupported);
            }
            base.RemoveItem(index);
            _owner.UpdateMonths();
        }

        /// <summary>
        /// The object in the specified index is replaced with the provided item.
        /// </summary>
        /// <param name="index"></param>
        /// <param name="item"></param>
        protected override void SetItem(int index, CalendarDateRange item)
        {
            if (!IsValidThread())
            {
                throw new NotSupportedException(Resource.CalendarCollection_MultiThreadedCollectionChangeNotSupported);
            }

            if (IsValid(item))
            {
                base.SetItem(index, item);
                _owner.UpdateMonths();
            }
            else
            {
                throw new ArgumentOutOfRangeException(Resource.Calendar_UnSelectableDates);
            }
        }

        #endregion Protected Methods

        #region Private Methods

        private bool IsValid(CalendarDateRange item)
        {
            foreach (object child in _owner.SelectedDates)
            {
                DateTime? day = child as DateTime?;
                Debug.Assert(day != null);
                if (DateTimeHelper.InRange(day.Value, item))
                {
                    return false;
                }
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

