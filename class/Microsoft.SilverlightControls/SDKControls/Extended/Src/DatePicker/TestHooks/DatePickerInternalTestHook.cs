// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Windows.Controls.Primitives;

namespace System.Windows.Controls
{
    public partial class DatePicker
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
        /// Test hook class that exposes internal and private members of the DatePicker
        /// </summary>
        internal class InternalTestHook
        {
            //Reference to the outer 'parent' DatePicker
            private DatePicker _datePicker;

            internal InternalTestHook(DatePicker datePicker)
            {
                _datePicker = datePicker;
            }

            #region Internal Properties

            internal Popup DropDown
            {
                get
                {
                    return _datePicker._popUp;
                }
            }

            internal Button DropDownButton
            {
                get
                {
                    return _datePicker._dropDownButton;
                }
            }

            internal Calendar DropDownCalendar
            {
                get
                {
                    return _datePicker._calendar;
                }
                set
                {
                    _datePicker._calendar = value;
                }
            }

            internal DatePickerTextBox DatePickerWatermarkedTextBox
            {
                get
                {
                    return _datePicker._textBox;
                }
            }

            #endregion

            #region Internal Methods

            internal string DateTimeToString(DateTime d)
            {
                return _datePicker.DateTimeToString(d);
            }

            internal void DropDownButton_Click(object sender, RoutedEventArgs e)
            {
                _datePicker.DropDownButton_Click(sender, e);
            }

            internal void SetSelectedDate()
            {
                _datePicker.SetSelectedDate();
            }

            #endregion
        }
    }
    
}
