// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Windows.Controls.Primitives;
using System.Windows.Input;

namespace System.Windows.Controls
{
    public partial class Calendar
    {
        private InternalTestHook _testHook;

        //Internal property to expose the testhook object
        internal InternalTestHook TestHook
        {
            get
            {
                if (_testHook == null)
                {
                    _testHook = new InternalTestHook(this);
                }
                return _testHook;
            }
        }
        /// <summary>
        /// Test hook class that exposes internal and private members of the calendar
        /// </summary>
        internal class InternalTestHook
        {
            //Reference to the outer 'parent' calendar
            private Calendar _calendar;

            internal InternalTestHook(Calendar cal)
            {
                _calendar = cal;
            }

            #region Internal Properties
            internal CalendarItem MonthControl
            {
                get
                {
                    return _calendar.MonthControl;
                }
            }

            internal DateTime SelectedMonth
            {
                get
                {
                    return _calendar.SelectedMonth;
                }
            }

            internal DateTime SelectedYear
            {
                get
                {
                    return _calendar.SelectedYear;
                }
            }

            internal static int YEAR_COLS
            {
                get
                {
                    return Calendar.YEAR_COLS;
                }
            }

            internal static int YEAR_ROWS
            {
                get
                {
                    return Calendar.YEAR_ROWS;
                }
            }
            #endregion


            #region Internal Methods

            internal void Calendar_KeyDown(object sender, KeyEventArgs e)
            {
                _calendar.Calendar_KeyDown(sender, e);
            }

            internal void ClickHeader()
            {
                _calendar.OnHeaderClick();
            }

            internal void ClickNext()
            {
                _calendar.OnNextClick();
            }

            internal void ClickPrevious()
            {
                _calendar.OnPreviousClick();
            }

            internal CalendarDayButton FindDayButtonFromDay(DateTime day)
            {
                return _calendar.FindDayButtonFromDay(day);
            }

            internal bool ProcessCalendarKey(KeyEventArgs e)
            {
                return _calendar.ProcessCalendarKey(e);
            }

            internal void ProcessDownKey(bool ctrl, bool shift)
            {
                _calendar.ProcessDownKey(ctrl, shift);
            }

            internal void ProcessEndKey(bool shift)
            {
                _calendar.ProcessEndKey(shift);
            }

            internal void ProcessHomeKey(bool shift)
            {
                _calendar.ProcessHomeKey(shift);
            }

            internal void ProcessLeftKey(bool shift)
            {
                _calendar.ProcessLeftKey(shift);
            }

            internal void ProcessPageDownKey(bool shift)
            {
                _calendar.ProcessPageDownKey(shift);
            }

            internal void ProcessPageUpKey(bool shift)
            {
                _calendar.ProcessPageUpKey(shift);
            }

            internal void ProcessRightKey(bool shift)
            {
                _calendar.ProcessRightKey(shift);
            }

            internal void ProcessShiftKeyUp()
            {
                _calendar.ProcessShiftKeyUp();
            }

            internal void ProcessUpKey(bool ctrl, bool shift)
            {
                _calendar.ProcessUpKey(ctrl, shift);
            }

            #endregion

        }
    }
}
