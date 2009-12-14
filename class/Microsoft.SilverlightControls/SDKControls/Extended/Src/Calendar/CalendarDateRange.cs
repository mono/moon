// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

namespace System.Windows.Controls
{
    /// <summary>
    /// Specifies a DateTime range class which has a start and end.
    /// </summary>
    public sealed class CalendarDateRange
    {
        #region Data
        DateTime _end;
        DateTime _start;
        #endregion Data

        /// <summary>
        /// Initializes a new instance of the CalendarDateRange class which creates a range from a single DateTime value.
        /// </summary>
        /// <param name="day"></param>
        public CalendarDateRange(DateTime day)
        {
            _end = day;
            _start = day;
        }

        /// <summary>
        /// Initializes a new instance of the CalendarDateRange class which accepts range start and end dates.
        /// </summary>
        /// <param name="start"></param>
        /// <param name="end"></param>
        public CalendarDateRange(DateTime start, DateTime end)
        {
            if (DateTime.Compare(end, start) >= 0)
            {
                _start = start;
                _end = end;
            }
            else
            {
                //we always coerce the second value to the first
                _start = start;
                _end = start;
            }
        }

        #region Public Properties

        /// <summary>
        /// Specifies the End date of the CalendarDateRange.
        /// </summary>
        public DateTime End
        {
            get { return _end; }
        }

        /// <summary>
        /// Specifies the Start date of the CalendarDateRange.
        /// </summary>
        public DateTime Start
        {
            get { return _start; }
        }

        #endregion Public Properties

        #region Internal Methods

        /// <summary>
        /// Returns true if any day in the given DateTime range is contained in the current CalendarDateRange.
        /// </summary>
        /// <param name="range"></param>
        /// <returns></returns>
        internal bool ContainsAny(CalendarDateRange range)
        {
            return (// supplied range starts in this range
                    (DateTime.Compare(this.Start, range.Start) <= 0 && DateTime.Compare(this.End, range.Start) >= 0) ||
                    // supplied range starts before the range and ends after the range starts
                    (DateTime.Compare(this.Start, range.Start) >= 0 && DateTime.Compare(this.Start, range.End) <= 0));
        }

        #endregion Internal Methods

    }
}
