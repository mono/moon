// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Windows.Input;

namespace System.Windows.Controls.Primitives
{
    /// <summary>
    /// TestHook for Month class
    /// </summary>
    public partial class CalendarItem
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
        /// Test hook class that exposes internal and private members of the month
        /// </summary>
        internal class InternalTestHook
        {
            //Reference to the outer 'parent' calendar
            private CalendarItem _month;

            internal InternalTestHook(CalendarItem mon)
            {
                _month = mon;
            }

            #region Internal Properties
            internal Button HeaderButton
            {
                get
                {
                    return _month.HeaderButton;
                }
            }

            internal Button NextButton
            {
                get
                {
                    return _month.NextButton;
                }
            }

            internal Button PreviousButton
            {
                get
                {
                    return _month.PreviousButton;
                }
            }
            #endregion


            #region Internal Methods

            internal void Cell_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
            {
                _month.Cell_MouseLeftButtonDown(sender, e);
            }

            internal void Cell_MouseEnter(object sender, MouseEventArgs e)
            {
                _month.Cell_MouseEnter(sender, e);
            }

            internal void Cell_MouseLeave(object sender, MouseEventArgs e)
            {
                _month.Cell_MouseLeave(sender, e);
            }

            internal void Cell_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
            {
                _month.Cell_MouseLeftButtonUp(sender, e);
            }

            internal void HeaderButton_Click(object sender, RoutedEventArgs e)
            {
                _month.HeaderButton_Click(sender, e);
            }

            internal void Month_CalendarButtonMouseUp(object sender, MouseButtonEventArgs e)
            {
                _month.Month_CalendarButtonMouseUp(sender, e);
            }

            internal void NextButton_Click(object sender, RoutedEventArgs e)
            {
                _month.NextButton_Click(sender, e);
            }

            internal void PreviousButton_Click(object sender, RoutedEventArgs e)
            {
                _month.PreviousButton_Click(sender, e);
            }
            #endregion
        }

    }
}
