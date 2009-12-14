// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Globalization;
using System.Windows.Automation;
using System.Windows.Automation.Peers;
using System.Windows.Automation.Provider;
using System.Windows.Controls.Test;
using System.Windows.Markup;
using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Windows.Controls.Primitives;
using Mono.Moonlight.UnitTesting;

namespace System.Windows.Controls.Extended.Test
{
    /// <summary>
    /// Unit tests for System.Windows.Controls.DatePicker.
    /// </summary>
    [TestClass]
    public class DatePickerTest : SilverlightTest
    {
        const int sleepTime = 500;
        DatePicker _elementToCleanUp;

        /// <summary>
        /// Create a DatePicker control.
        /// </summary>
        [TestMethod]
        [Description("Create a DatePicker control.")]
        public void CreateDatePicker()
        {
            DatePicker datePicker = new DatePicker();
            Assert.IsNotNull(datePicker);
        }


        /// <summary>
        /// Create a DatePicker in XAML markup.
        /// </summary>
        [TestMethod]
        [Description("Create a DatePicker in XAML markup.")]
        [Asynchronous]
        public void CreateInXaml()
        {
            object result = XamlReader.Load("<toolkit:DatePicker  SelectedDate=\"04/30/2008\" DisplayDateStart=\"04/30/2020\" DisplayDateEnd=\"04/30/2010\" DisplayDate=\"02/02/2000\"   xmlns='http://schemas.microsoft.com/client/2007'" +
  " xmlns:toolkit=\"clr-namespace:System.Windows.Controls;assembly=System.Windows.Controls\" />");
            Assert.IsInstanceOfType(result, typeof(DatePicker));
            _elementToCleanUp = (DatePicker)result;
            ((DatePicker)result).Loaded += new RoutedEventHandler(_elementToCleanUp_Loaded);
            TestPanel.Children.Add(_elementToCleanUp);
            EnqueueConditional(IsLoaded);
            EnqueueCallback(() => VerifyXamlValues());
            EnqueueTestComplete();
        }

        private void VerifyXamlValues()
        {
            Assert.IsTrue(CompareDates(_elementToCleanUp.SelectedDate.Value, new DateTime(2008, 4, 30)));
            Assert.IsTrue(CompareDates(_elementToCleanUp.DisplayDateStart.Value, new DateTime(2008, 4, 30)));
            Assert.IsTrue(CompareDates(_elementToCleanUp.DisplayDate, new DateTime(2008, 4, 30)));
            Assert.IsTrue(CompareDates(_elementToCleanUp.DisplayDateEnd.Value, new DateTime(2010, 4, 30)));
            _isLoaded = false;
        }

        /// <summary>
        /// Ensure all default values are correct.
        /// </summary>
        [TestMethod]
        [Description("Ensure all default values are correct.")]
        public void DPCheckDefaultValues()
        {
            DatePicker datePicker = new DatePicker();
            Assert.IsTrue(CompareDates(datePicker.DisplayDate, DateTime.Today));
            Assert.IsNull(datePicker.DisplayDateStart);
            Assert.IsNull(datePicker.DisplayDateEnd);
            Assert.AreEqual(datePicker.FirstDayOfWeek, CultureInfo.CurrentCulture.DateTimeFormat.FirstDayOfWeek);
            Assert.IsFalse(datePicker.IsDropDownOpen);
            Assert.IsTrue(datePicker.IsTodayHighlighted);
            Assert.IsNull(datePicker.SelectedDate);
            Assert.AreEqual(datePicker.SelectedDateFormat, DatePickerFormat.Short);
            Assert.IsTrue(datePicker.BlackoutDates.Count == 0);
            Assert.AreEqual(datePicker.Text, string.Empty);
            DateTime d = (DateTime)datePicker.GetValue(DatePicker.DisplayDateProperty);
            Assert.IsTrue(CompareDates(d, DateTime.Today));
            Assert.IsNull((CalendarDateRange)datePicker.GetValue(DatePicker.DisplayDateEndProperty));
            Assert.IsNull((CalendarDateRange)datePicker.GetValue(DatePicker.DisplayDateStartProperty));
            Assert.AreEqual((DayOfWeek)datePicker.GetValue(DatePicker.FirstDayOfWeekProperty), DayOfWeek.Sunday);
            Assert.IsFalse((bool)datePicker.GetValue(DatePicker.IsDropDownOpenProperty));
            Assert.IsTrue((bool)datePicker.GetValue(DatePicker.IsTodayHighlightedProperty));
            Assert.IsNull((DateTime?)datePicker.GetValue(DatePicker.SelectedDateProperty));
            Assert.AreEqual((DatePickerFormat)datePicker.GetValue(DatePicker.SelectedDateFormatProperty), DatePickerFormat.Short);
            Assert.AreEqual((string)datePicker.GetValue(DatePicker.TextProperty), string.Empty);

        }

        /// <summary>
        /// Verify properties set on DatePicker before load are reflected on the DatePicker after load.
        /// </summary>
        [TestMethod]
        [Description("Verify properties set on DatePicker before load are reflected on the DatePicker after load.")]
        [Asynchronous]
        public void DPVerifyBeforeLoad()
        {
            _elementToCleanUp = new DatePicker();
            _elementToCleanUp.DisplayDateStart = DateTime.Today.AddDays(-100);
            _elementToCleanUp.DisplayDateEnd = DateTime.Today.AddDays(1000);
            _elementToCleanUp.FirstDayOfWeek = DayOfWeek.Wednesday;
            _elementToCleanUp.IsDropDownOpen = true;
            _elementToCleanUp.IsTodayHighlighted = false;
            _elementToCleanUp.SelectedDate = DateTime.Today.AddDays(200);
            _elementToCleanUp.DisplayDate = DateTime.Today.AddDays(500);
            _elementToCleanUp.SelectedDateFormat = DatePickerFormat.Long;
            _elementToCleanUp.BlackoutDates.Add(new CalendarDateRange(DateTime.Today.AddDays(300), DateTime.Today.AddDays(500)));
            Assert.AreEqual(_elementToCleanUp.SelectedDate, DateTime.Today.AddDays(200));
            _elementToCleanUp.Text = "testbeforeload";
            _elementToCleanUp.Loaded += new RoutedEventHandler(_elementToCleanUp_Loaded);
            TestPanel.Children.Add(_elementToCleanUp);
            EnqueueConditional(IsLoaded);
            EnqueueSleep(sleepTime);
            EnqueueCallback(VerifyBeforeLoaded);
            EnqueueTestComplete();
        }

        private void VerifyBeforeLoaded()
        {
            Assert.IsTrue(CompareDates(_elementToCleanUp.DisplayDate, DateTime.Today.AddDays(500)));
            Assert.AreEqual(_elementToCleanUp.DisplayDateStart.Value, DateTime.Today.AddDays(-100));
            Assert.AreEqual(_elementToCleanUp.DisplayDateEnd.Value, DateTime.Today.AddDays(1000));
            Assert.AreEqual(_elementToCleanUp.FirstDayOfWeek, DayOfWeek.Wednesday);
            Assert.IsTrue(_elementToCleanUp.IsDropDownOpen);
            Assert.IsFalse(_elementToCleanUp.IsTodayHighlighted);
            Assert.AreEqual(_elementToCleanUp.SelectedDate, DateTime.Today.AddDays(200));
            Assert.AreEqual(_elementToCleanUp.SelectedDateFormat, DatePickerFormat.Long);
            Assert.IsTrue(CompareDates(_elementToCleanUp.BlackoutDates[0].Start, DateTime.Today.AddDays(300)));
            Assert.IsTrue(CompareDates(_elementToCleanUp.BlackoutDates[0].End, DateTime.Today.AddDays(500)));
            _elementToCleanUp.IsDropDownOpen = false;
            _isLoaded = false;
        }

        /// <summary>
        /// Ensure Nullable Properties can be set to null.
        /// </summary>
        [TestMethod]
        [Description("Ensure Nullable Properties can be set to null.")]
        public void DPArePropertiesNullable()
        {
            DatePicker datePicker = new DatePicker();
            DateTime value = DateTime.Today;

            datePicker.SelectedDate = null;
            Assert.IsNull(datePicker.SelectedDate);

            datePicker.SelectedDate = value;
            Assert.IsTrue(CompareDates(value, (DateTime)datePicker.SelectedDate));

            datePicker.SelectedDate = null;
            Assert.IsNull(datePicker.SelectedDate);
        }

        /// <summary>
        /// Ensure DateTime Properties can be set to DateTime.MaxValue.
        /// </summary>
        [TestMethod]
        [Description("Ensure DateTime Properties can be set to DateTime.MaxValue")]
        public void DPSetToMaxValue()
        {
            DatePicker datePicker = new DatePicker();

            datePicker.DisplayDate = DateTime.MaxValue;
            Assert.IsTrue(CompareDates(datePicker.DisplayDate, DateTime.MaxValue));

            datePicker.SelectedDate = DateTime.MaxValue;
            Assert.IsTrue(CompareDates(DateTime.MaxValue, (DateTime)datePicker.SelectedDate));

            datePicker.DisplayDateStart = DateTime.MaxValue;
            datePicker.DisplayDateEnd = DateTime.MaxValue;
        }

        /// <summary>
        /// Ensure DateTime Properties can be set to DateTime.MinValue.
        /// </summary>
        [TestMethod]
        [Description("Ensure DateTime Properties can be set to DateTime.MinValue")]
        public void DPSetToMinValue()
        {
            DatePicker datePicker = new DatePicker();

            datePicker.DisplayDate = DateTime.MinValue;
            Assert.IsTrue(CompareDates(datePicker.DisplayDate, DateTime.MinValue));

            datePicker.DisplayDateStart = DateTime.MinValue;
            datePicker.DisplayDateEnd = DateTime.MinValue;

            datePicker.SelectedDate = DateTime.MinValue;
            Assert.IsTrue(CompareDates(DateTime.MinValue, (DateTime)datePicker.SelectedDate));

            datePicker.SelectedDate = null;
            datePicker.BlackoutDates.Add(new CalendarDateRange(DateTime.MinValue));
            Assert.IsTrue(CompareDates(datePicker.BlackoutDates[0].End, DateTime.MinValue));
            Assert.IsTrue(CompareDates(datePicker.BlackoutDates[0].Start, DateTime.MinValue));
        }

        #region Events
        /// <summary>
        /// Ensure CalendarClosed/Opened events are fired.
        /// </summary>
        [TestMethod]
        [Description("Ensure CalendarClosed/Opened events are fired.")]
        [Asynchronous]
        public void DPCalendarClosedEvent()
        {
            _elementToCleanUp = new DatePicker();
            TestPanel.Children.Add(_elementToCleanUp);

            _elementToCleanUp.CalendarClosed += new RoutedEventHandler(delegate
            {
                testStr = "Calendar Closed!";
            });

            _elementToCleanUp.CalendarOpened += new RoutedEventHandler(delegate
            {
                testStr = "Calendar Opened!";
            });

            _elementToCleanUp.Loaded += new RoutedEventHandler(_elementToCleanUp_Loaded);


            EnqueueConditional(IsLoaded);
            EnqueueSleep(sleepTime);
            EnqueueCallback(VerifyEvents);
            EnqueueTestComplete();
        }

        bool _isLoaded;
        string testStr;

        private bool IsLoaded()
        {
            return _isLoaded;
        }

        private void _elementToCleanUp_Loaded(object sender, RoutedEventArgs e)
        {
            _isLoaded = true;
        }

        private void VerifyEvents()
        {
            _elementToCleanUp.IsDropDownOpen = true;
            Assert.AreEqual(testStr, "Calendar Opened!");
            _elementToCleanUp.IsDropDownOpen = false;
            Assert.AreEqual(testStr, "Calendar Closed!");
            _isLoaded = false;
        }

        /// <summary>
        /// Ensure DateSelected event is fired.
        /// </summary>
        [TestMethod]
        [Description("Ensure DateSelected event is fired.")]
        [Asynchronous]
        public void DPDateSelectedEvent()
        {
            string testString = null;
            DatePicker datePicker = new DatePicker();

            datePicker.SelectedDateChanged += new EventHandler<SelectionChangedEventArgs>(dp_SelectedDatesChanged);
            datePicker.SelectedDateChanged += new EventHandler<SelectionChangedEventArgs>(delegate
            {
                testString = "Handled!";
            });
            DateTime value = new DateTime(2000, 10, 10);
            datePicker.SelectedDate = value;
            EnqueueSleep(sleepTime);
            EnqueueCallback(delegate
            {
                Assert.AreEqual(testString, "Handled!");
            });
            EnqueueTestComplete();
        }

        void dp_SelectedDatesChanged(object sender, SelectionChangedEventArgs e)
        {
            DatePicker datePicker = sender as DatePicker;
            Assert.IsTrue(e.AddedItems.Count == 1);
            Assert.IsTrue(CompareDates((DateTime)e.AddedItems[0], new DateTime(2000, 10, 10)));
            Assert.IsTrue(e.RemovedItems.Count == 0);
        }


        /// <summary>
        /// Ensure TextParseError event is fired.
        /// </summary>
        [TestMethod]
        [Description("Ensure TextParseError event is fired.")]
        [Asynchronous]
        public void DPTextParseErrorEvent()
        {
            _elementToCleanUp = new DatePicker();
            _elementToCleanUp.Loaded += new RoutedEventHandler(_elementToCleanUp_Loaded);
            TestPanel.Children.Add(_elementToCleanUp);

            _elementToCleanUp.DateValidationError += new EventHandler<DatePickerDateValidationErrorEventArgs>(_elementToCleanUp_TextParseError1);
            EnqueueConditional(IsLoaded);
            EnqueueCallback(SendErroneousText);
            EnqueueTestComplete();
        }

        /// <summary>
        /// Ensure TextParseError event is fired.
        /// </summary>
        [TestMethod]
        [Description("Ensure TextParseError event is fired.")]
        [Asynchronous]
        public void DPTextParseErrorEvent2()
        {
            _elementToCleanUp = new DatePicker();
            _elementToCleanUp.Loaded += new RoutedEventHandler(_elementToCleanUp_Loaded);
            TestPanel.Children.Add(_elementToCleanUp);

            _elementToCleanUp.DateValidationError += new EventHandler<DatePickerDateValidationErrorEventArgs>(_elementToCleanUp_TextParseError2);
            EnqueueConditional(IsLoaded);
            EnqueueCallback(SendErroneousText2);
            EnqueueTestComplete();
        }

        /// <summary>
        /// Ensure TextParseError event is fired.
        /// </summary>
        [TestMethod]
        [Description("Ensure TextParseError event is fired.")]
        [Asynchronous]
        public void DPTextParseErrorEvent3()
        {
            _elementToCleanUp = new DatePicker();
            _elementToCleanUp.Loaded += new RoutedEventHandler(_elementToCleanUp_Loaded);
            TestPanel.Children.Add(_elementToCleanUp);
            _elementToCleanUp.SelectedDate = new DateTime(2003, 10, 10);
            _elementToCleanUp.DateValidationError += new EventHandler<DatePickerDateValidationErrorEventArgs>(_elementToCleanUp_TextParseError2);
            EnqueueConditional(IsLoaded);
            EnqueueCallback(SendErroneousText3);
            EnqueueTestComplete();
        }

        /// <summary>
        /// Ensure DateValidationError event is fired.
        /// </summary>
        [TestMethod]
        [Description("Ensure DateValidationError event is fired.")]
        [Asynchronous]
        public void DPDateValidationErrorEvent()
        {
            _elementToCleanUp = new DatePicker();
            _elementToCleanUp.BlackoutDates.Add(new CalendarDateRange(new DateTime(2000, 2, 2)));
            _elementToCleanUp.Loaded += new RoutedEventHandler(_elementToCleanUp_Loaded);
            TestPanel.Children.Add(_elementToCleanUp);
            _elementToCleanUp.DateValidationError += new EventHandler<DatePickerDateValidationErrorEventArgs>(_elementToCleanUp_TextParseError1);
            EnqueueConditional(IsLoaded);
            EnqueueCallback(SendErroneousDate);
            EnqueueTestComplete();
        }

        private void SendErroneousDate()
        {
            Common.AssertExpectedExceptionWithoutMessageControl(new ArgumentOutOfRangeException(), () => { _elementToCleanUp.Text = "2000/02/02"; });
            _isLoaded = false;
        }

        /// <summary>
        /// Ensure DateValidationError event is fired.
        /// </summary>
        [TestMethod]
        [Description("Ensure DateValidationError event is fired.")]
        [Asynchronous]
        public void DPDateValidationErrorEvent2()
        {
            _elementToCleanUp = new DatePicker();
            _elementToCleanUp.BlackoutDates.Add(new CalendarDateRange(new DateTime(2000, 2, 2)));
            _elementToCleanUp.Loaded += new RoutedEventHandler(_elementToCleanUp_Loaded);
            TestPanel.Children.Add(_elementToCleanUp);
            _elementToCleanUp.DateValidationError += new EventHandler<DatePickerDateValidationErrorEventArgs>(_elementToCleanUp_TextParseError2);
            EnqueueConditional(IsLoaded);
            EnqueueCallback(SendErroneousDate2);
            EnqueueTestComplete();
        }

        private void SendErroneousDate2()
        {
            Assert.IsNull(_elementToCleanUp.SelectedDate);
            _isLoaded = false;
        }

        private void _elementToCleanUp_TextParseError1(object sender, DatePickerDateValidationErrorEventArgs e)
        {
            e.ThrowException = true;
        }


        private void _elementToCleanUp_TextParseError2(object sender, DatePickerDateValidationErrorEventArgs e)
        {
            e.ThrowException = false;
        }

        private void SendErroneousText()
        {
            Common.AssertExpectedExceptionWithoutMessageControl(new FormatException(), () => { _elementToCleanUp.Text = "errortest"; });
            _isLoaded = false;
        }

        private void SendErroneousText2()
        {
            DateTimeFormatInfo dtfi = CultureInfo.CurrentCulture.DateTimeFormat;

            _elementToCleanUp.Text = "errortest";
            Assert.AreEqual(_elementToCleanUp.TestHook.DatePickerWatermarkedTextBox.Watermark, string.Format(CultureInfo.CurrentCulture, Resource.DatePicker_WatermarkText, dtfi.ShortDatePattern.ToString()));
            Assert.IsTrue(string.IsNullOrEmpty(_elementToCleanUp.Text));
            Assert.IsTrue(string.IsNullOrEmpty(_elementToCleanUp.TestHook.DatePickerWatermarkedTextBox.Text));
            _isLoaded = false;
        }

        private void SendErroneousText3()
        {
            DateTimeFormatInfo dtfi = CultureInfo.CurrentCulture.DateTimeFormat;
            DateTime d = new DateTime(2003, 10, 10);
            _elementToCleanUp.Text = "errortest";
            Assert.AreEqual(string.Format(CultureInfo.CurrentCulture, d.ToString(dtfi.ShortDatePattern, dtfi)), _elementToCleanUp.Text);
            Assert.AreEqual(string.Format(CultureInfo.CurrentCulture, d.ToString(dtfi.ShortDatePattern, dtfi)), _elementToCleanUp.TestHook.DatePickerWatermarkedTextBox.Text);
            _isLoaded = false;
        }

        #endregion Events

        #region BlackoutDates

        /// <summary>
        /// Set the value of SelectedDateProperty.
        /// </summary>
        [TestMethod]
        [Description("Set the value of SelectedDateProperty.")]
        public void BlackoutDatesSingleDay()
        {
            DatePicker datePicker = new DatePicker();
            datePicker.BlackoutDates.Add(new CalendarDateRange(DateTime.Today));
            Common.AssertExpectedExceptionWithoutMessageControl(new ArgumentOutOfRangeException(), () => { datePicker.SelectedDate = DateTime.Today; });
        }

        [TestMethod]
        public void BlackoutDatesRange()
        {
            DatePicker datePicker = new DatePicker();
            datePicker.BlackoutDates.Add(new CalendarDateRange(DateTime.Today, DateTime.Today.AddDays(10)));
            datePicker.SelectedDate = DateTime.Today.AddDays(-1);
            Assert.IsTrue(CompareDates(datePicker.SelectedDate.Value, DateTime.Today.AddDays(-1)));
            datePicker.SelectedDate = DateTime.Today.AddDays(11);
            Assert.IsTrue(CompareDates(datePicker.SelectedDate.Value, DateTime.Today.AddDays(11)));
            Common.AssertExpectedExceptionWithoutMessageControl(new ArgumentOutOfRangeException(), () => { datePicker.SelectedDate = DateTime.Today.AddDays(5); });
        }

        [TestMethod]
        public void SetBlackoutDatesRange()
        {
            DatePicker datePicker = new DatePicker();
            datePicker.SelectedDate = DateTime.Today.AddDays(5);
            Common.AssertExpectedExceptionWithoutMessageControl(new ArgumentOutOfRangeException(), () => { datePicker.BlackoutDates.Add(new CalendarDateRange(DateTime.Today, DateTime.Today.AddDays(10))); });
        }

        #endregion BlackoutDates

        #region DisplayDate
        /// <summary>
        /// Set the value of DisplayDateProperty.
        /// </summary>
        [TestMethod]
        [Description("Set the value of DisplayDateProperty.")]
        public void DPDisplayDatePropertySetValue()
        {
            DatePicker datePicker = new DatePicker();
            DateTime value = DateTime.Today.AddMonths(3);
            datePicker.DisplayDate = value;
            Assert.IsTrue(CompareDates(datePicker.DisplayDate, value));
            DateTime d = (DateTime)datePicker.GetValue(DatePicker.DisplayDateProperty);
            Assert.IsTrue(CompareDates(d, value));
        }

        /// <summary>
        /// Ensure ArgumentOutOfRangeException is thrown.
        /// </summary>
        [TestMethod]
        [Description("Ensure ArgumentOutOfRangeException is thrown.")]
        public void DPDisplayDateOutOfRangeExceptionRangeEnd()
        {
            DatePicker datePicker = new DatePicker();
            datePicker.DisplayDate = new DateTime(2020, 12, 2);
            datePicker.DisplayDateEnd = new DateTime(2020, 12, 2);
            DateTime value = new DateTime(2020, 12, 15);
            datePicker.DisplayDate = value;

            Assert.IsTrue(CompareDates(datePicker.DisplayDate, new DateTime(2020, 12, 15)));

            value = new DateTime(2020, 12, 2);
            datePicker.DisplayDate = value;

            Assert.IsTrue(CompareDates(datePicker.DisplayDate, value));

            value = DateTime.MinValue;
            datePicker.DisplayDate = value;

            Assert.IsTrue(CompareDates(datePicker.DisplayDate, value));

            value = new DateTime(2021, 1, 1);
            datePicker.DisplayDate = value;
            Assert.IsTrue(CompareDates(datePicker.DisplayDate, datePicker.DisplayDateEnd.Value));
        }
        #endregion DisplayDate

        #region FirstDayOfWeek

        /// <summary>
        /// Set the value of FirstDayOfWeekProperty.
        /// </summary>
        [TestMethod]
        [Description("Set the value of FirstDayOfWeekProperty.")]
        public void DPFirstDayOfWeekPropertySetValue()
        {
            DatePicker datePicker = new DatePicker();
            DayOfWeek value = DayOfWeek.Thursday;
            datePicker.FirstDayOfWeek = value;

            Assert.AreEqual(value, datePicker.GetValue(DatePicker.FirstDayOfWeekProperty));
            Assert.AreEqual(value, datePicker.FirstDayOfWeek);

            value = (DayOfWeek)3;
            datePicker.FirstDayOfWeek = value;
            Assert.AreEqual(value, datePicker.FirstDayOfWeek);
            Assert.AreEqual(value, datePicker.GetValue(DatePicker.FirstDayOfWeekProperty));
        }

        /// <summary>
        /// Ensure ArgumentOutOfRangeException is thrown.
        /// </summary>
        [TestMethod]
        [Description("Ensure ArgumentOutOfRangeException is thrown.")]
        public void DPFirstDayOfWeekOutOfRangeException()
        {
            DatePicker datePicker = new DatePicker();
            Common.AssertExpectedExceptionWithoutMessageControl(new ArgumentOutOfRangeException(), () => { datePicker.FirstDayOfWeek = (DayOfWeek)7; });
        }

        #endregion FirstDayOfWeek

        #region IsDropDownOpen

        /// <summary>
        /// Ensure DropDown is opening/closing properly.
        /// </summary>
        [TestMethod]
        [Description("Ensure DropDown is opening/closing properly.")]
        [Asynchronous]
        public void IsDropDownOpen()
        {
            _elementToCleanUp = new DatePicker();
            _elementToCleanUp.IsDropDownOpen = true;
            TestPanel.Children.Add(_elementToCleanUp);
            _elementToCleanUp.Loaded += new RoutedEventHandler(_elementToCleanUp_Loaded);
            EnqueueConditional(IsLoaded);
            EnqueueCallback(delegate
            {
                Assert.IsTrue(_elementToCleanUp.TestHook.DropDown.IsOpen);
                Assert.IsTrue((bool)_elementToCleanUp.GetValue(DatePicker.IsDropDownOpenProperty));
                Assert.IsTrue(_elementToCleanUp.TestHook.DropDown.ActualHeight > 0);
                _elementToCleanUp.IsDropDownOpen = false;
                Assert.IsFalse((bool)_elementToCleanUp.GetValue(DatePicker.IsDropDownOpenProperty));
                _elementToCleanUp.IsDropDownOpen = true;
                Assert.IsTrue(_elementToCleanUp.TestHook.DropDown.IsOpen);
                Assert.IsTrue(_elementToCleanUp.TestHook.DropDown.ActualHeight > 0);
                _isLoaded = false;
            });
            EnqueueTestComplete();
        }

        #endregion IsDropDownOpen

        #region IsTodayHighlighted

        [TestMethod]
        [Description("Verify IsTodayHighlighted propery.")]
        [MoonlightBug("Our state is wrong when loaded is fired")]
        [Asynchronous]
        public void IsTodayHighlighted()
        {
            _elementToCleanUp = new DatePicker();
            TestPanel.Children.Add(_elementToCleanUp);

            _elementToCleanUp.Loaded += new RoutedEventHandler(_elementToCleanUp_Loaded);
            EnqueueConditional(IsLoaded);
            EnqueueCallback(VerifyIsTodayHighlighted);
            EnqueueTestComplete();
        }

        private void VerifyIsTodayHighlighted()
        {
            _elementToCleanUp.TestHook.DropDownButton_Click(_elementToCleanUp.TestHook.DropDownButton, new RoutedEventArgs());
            _elementToCleanUp.TestHook.DropDownCalendar.Loaded += new RoutedEventHandler(DropDownCalendar_Loaded_IsTodayHighlighted);
        }

        private void DropDownCalendar_Loaded_IsTodayHighlighted(object sender, RoutedEventArgs e)
        {
            CalendarDayButton b = _elementToCleanUp.TestHook.DropDownCalendar.FindDayButtonFromDay(DateTime.Today);
            Assert.IsTrue(b.IsToday);
            Assert.IsFalse(b.IsSelected);
            _elementToCleanUp.IsTodayHighlighted = false;
            Assert.IsFalse(b.IsToday);
            _elementToCleanUp.TestHook.DropDownCalendar.TestHook.MonthControl.TestHook.Cell_MouseLeftButtonDown(b, null);
            Assert.IsTrue(b.IsSelected);
            Assert.IsTrue(CompareDates(_elementToCleanUp.SelectedDate.Value, DateTime.Today));
            _elementToCleanUp.IsDropDownOpen = false;
            _isLoaded = false;
        }

        #endregion IsTodayHighlighted

        #region SelectedDate

        /// <summary>
        /// Set the value of SelectedDateProperty.
        /// </summary>
        [TestMethod]
        [Description("Set the value of SelectedDateProperty.")]
        public void DPSelectedDatePropertySetValue()
        {
            DatePicker datePicker = new DatePicker();
            datePicker.SelectedDate = DateTime.Today;
            Assert.IsTrue(CompareDates((DateTime)datePicker.SelectedDate, DateTime.Today));
            Assert.IsTrue(CompareDates((DateTime)datePicker.GetValue(DatePicker.SelectedDateProperty), DateTime.Today));
            datePicker.BlackoutDates.Add(new CalendarDateRange(DateTime.MinValue, DateTime.Today.AddDays(-1)));
            datePicker.BlackoutDates.Add(new CalendarDateRange(DateTime.Today.AddDays(1), DateTime.MaxValue));
            datePicker.SelectedDate = DateTime.Today;
            Assert.IsTrue(CompareDates((DateTime)datePicker.SelectedDate, DateTime.Today));
            Assert.IsTrue(CompareDates((DateTime)datePicker.GetValue(DatePicker.SelectedDateProperty), DateTime.Today));
        }

        /// <summary>
        /// Ensure WaterMarkedTextBox gets cleared when the SelectedDate is set to null.
        /// </summary>
        [TestMethod]
        [Description("Ensure WaterMarkedTextBox gets cleared when the SelectedDate is set to null.")]
        [Asynchronous]
        public void DPClearWMTB()
        {
            _elementToCleanUp = new DatePicker();
            _elementToCleanUp.Loaded += new RoutedEventHandler(_elementToCleanUp_Loaded);
            TestPanel.Children.Add(_elementToCleanUp);
            EnqueueConditional(IsLoaded);
            DateTimeFormatInfo dtfi = CultureInfo.CurrentCulture.DateTimeFormat;
            DateTime d = new DateTime(2003, 10, 10);
            EnqueueCallback(delegate
            {
                _elementToCleanUp.SelectedDate = d;
            });
            EnqueueSleep(sleepTime);
            EnqueueCallback(delegate
            {
                Assert.AreEqual(string.Format(CultureInfo.CurrentCulture, d.ToString(dtfi.ShortDatePattern, dtfi)), _elementToCleanUp.Text);
                Assert.AreEqual(string.Format(CultureInfo.CurrentCulture, d.ToString(dtfi.ShortDatePattern, dtfi)), _elementToCleanUp.TestHook.DatePickerWatermarkedTextBox.Text);
                _elementToCleanUp.SelectedDate = null;
            });

            EnqueueCallback(delegate
            {
                Assert.AreEqual(_elementToCleanUp.TestHook.DatePickerWatermarkedTextBox.Watermark, string.Format(CultureInfo.CurrentCulture, Resource.DatePicker_WatermarkText, dtfi.ShortDatePattern.ToString()));
                Assert.IsTrue(string.IsNullOrEmpty(_elementToCleanUp.Text));
                Assert.IsTrue(string.IsNullOrEmpty(_elementToCleanUp.TestHook.DatePickerWatermarkedTextBox.Text));
                _isLoaded = false;
            });
            EnqueueTestComplete();
        }

        #endregion SelectedDate

        #region SelectedDateFormat

        /// <summary>
        /// Ensure SelectedDateFormat can be changed.
        /// </summary>
        [TestMethod]
        [Description("Ensure SelectedDateFormat can be changed.")]
        [Asynchronous]
        public void DPSelectedDateFormat()
        {
            _elementToCleanUp = new DatePicker();
            TestPanel.Children.Add(_elementToCleanUp);

            _elementToCleanUp.Loaded += new RoutedEventHandler(_elementToCleanUp_Loaded);
            EnqueueConditional(IsLoaded);
            EnqueueCallback(VerifySelectedDateFormat);
            EnqueueTestComplete();
        }

        private void VerifySelectedDateFormat()
        {
            DateTimeFormatInfo dtfi = CultureInfo.CurrentCulture.DateTimeFormat;
            DateTime d = new DateTime(2008, 1, 10);
            _elementToCleanUp.Text = string.Format(CultureInfo.CurrentCulture, d.ToString(dtfi.ShortDatePattern, dtfi));
            _elementToCleanUp.SelectedDateFormat = DatePickerFormat.Long;
            Assert.AreEqual(string.Format(CultureInfo.CurrentCulture, d.ToString(dtfi.LongDatePattern, dtfi)), _elementToCleanUp.Text);
            _elementToCleanUp.SelectedDateFormat = DatePickerFormat.Short;
            Assert.AreEqual(string.Format(CultureInfo.CurrentCulture, d.ToString(dtfi.ShortDatePattern, dtfi)), _elementToCleanUp.Text);
            _isLoaded = false;
        }
        #endregion SelectedDateFormat

        #region Text

        /// <summary>
        /// Ensure Text property reflects the changes in the TextBox part of the DatePicker.
        /// </summary>
        [TestMethod]
        [Description("Ensure Text property reflects the changes in the TextBox part of the DatePicker.")]
        [Asynchronous]        public void DPText()
        {
            _elementToCleanUp = new DatePicker();
            TestPanel.Children.Add(_elementToCleanUp);

            _elementToCleanUp.Loaded += new RoutedEventHandler(_elementToCleanUp_Loaded);
            EnqueueConditional(IsLoaded);
            DateTimeFormatInfo dtfi = CultureInfo.CurrentCulture.DateTimeFormat;
            EnqueueCallback(delegate
            {
                _elementToCleanUp.Text = "test";
                Assert.IsFalse(_elementToCleanUp.TestHook.DatePickerWatermarkedTextBox.Text == "test");
                Assert.AreEqual(_elementToCleanUp.TestHook.DatePickerWatermarkedTextBox.Watermark, string.Format(CultureInfo.CurrentCulture, Resource.DatePicker_WatermarkText, dtfi.ShortDatePattern.ToString()));
                Assert.IsTrue(string.IsNullOrEmpty((string)_elementToCleanUp.GetValue(DatePicker.TextProperty)));
                _elementToCleanUp.Text = DateTime.Today.ToString();
                Assert.IsTrue(CompareDates(_elementToCleanUp.SelectedDate.Value, DateTime.Today));
            });
            EnqueueSleep(sleepTime);
            EnqueueCallback(delegate
            {
                Assert.AreEqual(string.Format(CultureInfo.CurrentCulture, DateTime.Today.ToString(dtfi.ShortDatePattern, dtfi)), _elementToCleanUp.Text);
                _isLoaded = false;
            });
            EnqueueTestComplete();
        }

        /// <summary>
        /// Ensure TextBox text property reflects the changes in the Text property of the DatePicker.
        /// </summary>
        [TestMethod]
        [Description("Ensure TextBox text property reflects the changes in the Text property of the DatePicker.")]
        [Asynchronous]
        public void DPTextBeforeLoad()
        {
            _elementToCleanUp = new DatePicker();
            _elementToCleanUp.Text = "testbeforeload";
            TestPanel.Children.Add(_elementToCleanUp);

            _elementToCleanUp.Loaded += new RoutedEventHandler(_elementToCleanUp_Loaded);
            EnqueueConditional(IsLoaded);
            EnqueueSleep(sleepTime);
            EnqueueCallback(VerifyTextBeforeLoaded);
            EnqueueTestComplete();
        }

        /// <summary>
        /// Ensure TextBox text property reflects the changes in the Text property of the DatePicker.
        /// </summary>
        [TestMethod]
        [Description("Ensure TextBox text property reflects the changes in the Text property of the DatePicker.")]
        [Asynchronous]
        public void DPValidTextBeforeLoad()
        {
            _elementToCleanUp = new DatePicker();
            DateTime date = new DateTime(2005, 5, 5);
            _elementToCleanUp.Text = date.ToString();
            TestPanel.Children.Add(_elementToCleanUp);

            _elementToCleanUp.Loaded += new RoutedEventHandler(_elementToCleanUp_Loaded);
            EnqueueConditional(IsLoaded);
            EnqueueSleep(sleepTime);
            EnqueueCallback(VerifyValidTextBeforeLoaded);
            EnqueueTestComplete();
        }

        /// <summary>
        /// Ensure Text property reflects the changes in the TextBox part of the DatePicker.
        /// </summary>
        [TestMethod]
        [Description("Ensure Text property reflects the changes in the TextBox part of the DatePicker.")]
        [Asynchronous]
        public void DPTextNull()
        {
            _elementToCleanUp = new DatePicker();
            _elementToCleanUp.SelectedDate = DateTime.Today;
            TestPanel.Children.Add(_elementToCleanUp);
            _elementToCleanUp.Loaded += new RoutedEventHandler(_elementToCleanUp_Loaded);
            EnqueueConditional(IsLoaded);
            EnqueueCallback(VerifyTextNull);
            EnqueueTestComplete();
        }

        private void VerifyTextBeforeLoaded()
        {
            Assert.IsTrue(string.IsNullOrEmpty((string)_elementToCleanUp.GetValue(DatePicker.TextProperty)));
            _isLoaded = false;
        }

        private void VerifyValidTextBeforeLoaded()
        {
            DateTimeFormatInfo dtfi = CultureInfo.CurrentCulture.DateTimeFormat;
            DateTime date = new DateTime(2005, 5, 5);
            Assert.AreEqual(_elementToCleanUp.Text, string.Format(CultureInfo.CurrentCulture, date.ToString(dtfi.ShortDatePattern, dtfi)));
            _isLoaded = false;
        }

        private void VerifyTextNull()
        {
            _elementToCleanUp.Text = null;
            Assert.IsNull(_elementToCleanUp.SelectedDate);
            _isLoaded = false;
        }

        #endregion Text

        #region Navigation

        /// <summary>
        /// Ensure the popup does not close when navigating through the CalendarModes.
        /// </summary>
        [TestMethod]
        [Description("Ensure the popup does not close when navigating through the CalendarModes.")]
        [MoonlightBug("Our state is wrong when loaded is fired")]
        [Asynchronous]
        public void DPNavigation()
        {
            _elementToCleanUp = new DatePicker();
            TestPanel.Children.Add(_elementToCleanUp);

            _elementToCleanUp.Loaded += new RoutedEventHandler(_elementToCleanUp_Loaded);
            EnqueueConditional(IsLoaded);
            EnqueueSleep(sleepTime);
            EnqueueCallback(UpdateCalendarMode);
            EnqueueTestComplete();
        }

        private void UpdateCalendarMode()
        {
            Assert.IsFalse(_elementToCleanUp.IsDropDownOpen);
            _elementToCleanUp.TestHook.DropDownButton_Click(_elementToCleanUp.TestHook.DropDownButton, new RoutedEventArgs());
            _elementToCleanUp.TestHook.DropDownCalendar.Loaded += new RoutedEventHandler(DropDownCalendar_Loaded);
        }

        void DropDownCalendar_Loaded(object sender, RoutedEventArgs e)
        {
            _elementToCleanUp.TestHook.DropDownCalendar.TestHook.MonthControl.TestHook.HeaderButton_Click(_elementToCleanUp.TestHook.DropDownCalendar.TestHook.MonthControl.TestHook.HeaderButton, new RoutedEventArgs());
            Assert.IsTrue(_elementToCleanUp.TestHook.DropDownCalendar.DisplayMode == CalendarMode.Year);
            _elementToCleanUp.TestHook.DropDownCalendar.TestHook.MonthControl.TestHook.Month_CalendarButtonMouseUp(_elementToCleanUp.TestHook.DropDownCalendar.TestHook.MonthControl.YearView.Children[0], null);
            Assert.IsTrue(_elementToCleanUp.IsDropDownOpen);
            _elementToCleanUp.TestHook.DropDownCalendar.TestHook.MonthControl.TestHook.HeaderButton_Click(_elementToCleanUp.TestHook.DropDownCalendar.TestHook.MonthControl.TestHook.HeaderButton, new RoutedEventArgs());
            Assert.IsTrue(_elementToCleanUp.TestHook.DropDownCalendar.DisplayMode == CalendarMode.Year);
            _elementToCleanUp.TestHook.DropDownCalendar.TestHook.MonthControl.TestHook.HeaderButton_Click(_elementToCleanUp.TestHook.DropDownCalendar.TestHook.MonthControl.TestHook.HeaderButton, new RoutedEventArgs());
            Assert.IsTrue(_elementToCleanUp.TestHook.DropDownCalendar.DisplayMode == CalendarMode.Decade);
            _elementToCleanUp.TestHook.DropDownCalendar.TestHook.MonthControl.TestHook.Month_CalendarButtonMouseUp(_elementToCleanUp.TestHook.DropDownCalendar.TestHook.MonthControl.YearView.Children[0], null);
            Assert.IsTrue(_elementToCleanUp.IsDropDownOpen);
            Assert.IsTrue(_elementToCleanUp.TestHook.DropDownCalendar.DisplayMode == CalendarMode.Year);
            _elementToCleanUp.TestHook.DropDownButton_Click(_elementToCleanUp.TestHook.DropDownButton, new RoutedEventArgs());
            Assert.IsFalse(_elementToCleanUp.IsDropDownOpen);
            _isLoaded = false;
        }

        #endregion Navigation

        #region TextParsing

        /// <summary>
        /// Ensure Text is parsed in "D", "d" or "G" formats.
        /// </summary>
        [TestMethod]
        [Description("Ensure Text is parsed in \"D\", \"d\" or \"G\" formats.")]
        [Asynchronous]
        public void DPTextParse()
        {
            _elementToCleanUp = new DatePicker();
            _elementToCleanUp.Loaded += new RoutedEventHandler(_elementToCleanUp_Loaded);
            TestPanel.Children.Add(_elementToCleanUp);
            EnqueueConditional(IsLoaded);
            EnqueueCallback(SendText);
            EnqueueTestComplete();
        }

        private void SendText()
        {
            DateTimeFormatInfo dtfi = CultureInfo.CurrentCulture.DateTimeFormat;
            DateTime d = new DateTime(2003, 10, 10);
            _elementToCleanUp.Text = d.ToString("d");
            _elementToCleanUp.TestHook.SetSelectedDate();
            Assert.AreEqual(_elementToCleanUp.SelectedDate, new DateTime(2003, 10, 10));

            d = new DateTime(2003, 12, 10);
            _elementToCleanUp.Text = d.ToString("D");
            _elementToCleanUp.TestHook.SetSelectedDate();
            Assert.AreEqual(_elementToCleanUp.SelectedDate, new DateTime(2003, 12, 10));

            d = new DateTime(2003, 11, 10);
            _elementToCleanUp.Text = d.ToString("G");
            _elementToCleanUp.TestHook.SetSelectedDate();
            Assert.AreEqual(_elementToCleanUp.SelectedDate, new DateTime(2003, 11, 10));
            _isLoaded = false;
        }

        #endregion TextParsing

        #region DatePickerAutomationPeer Tests

        /// <summary>
        /// Tests the creation of an automation peer for the DatePicker
        /// </summary>
        [TestMethod]
        [Asynchronous]
        [Description("Tests the creation of an automation peer for the DatePicker")]
        public void DatePicker_AutomationPeer()
        {
            _elementToCleanUp = new DatePicker();
            _isLoaded = false;
            DatePickerAutomationPeer datePickerAutomationPeer = (DatePickerAutomationPeer)DatePickerAutomationPeer.CreatePeerForElement(_elementToCleanUp);
            Assert.IsNotNull(datePickerAutomationPeer);

            DateTime date = new DateTime(2000, 2, 2);
            _elementToCleanUp.SelectedDate = date;
            _elementToCleanUp.Height = 30;
            _elementToCleanUp.Width = 100;
            _elementToCleanUp.Loaded += new RoutedEventHandler(_elementToCleanUp_Loaded);
            TestPanel.Children.Add(_elementToCleanUp);
            EnqueueConditional(IsLoaded);
            DateTime date2 = new DateTime(2000, 5, 5);
            EnqueueCallback(delegate
            {
                Assert.AreEqual(datePickerAutomationPeer.GetAutomationControlType(), AutomationControlType.ComboBox, "Incorrect Control type for datepicker");
                Assert.AreEqual(datePickerAutomationPeer.GetClassName(), _elementToCleanUp.GetType().Name, "Incorrect ClassName value for datepicker");
                Assert.AreEqual(datePickerAutomationPeer.GetName(), date.ToString(), "Incorrect Name value for datepickerpeer");
                Assert.IsTrue(datePickerAutomationPeer.IsContentElement(), "Incorrect IsContentElement value");
                Assert.IsTrue(datePickerAutomationPeer.IsControlElement(), "Incorrect IsControlElement value");

                #region DatePickerAutomationPeer IExpandCollapseProvider tests:

                IExpandCollapseProvider datePickerExpandCollapseProvider = ((IExpandCollapseProvider)datePickerAutomationPeer.GetPattern(PatternInterface.ExpandCollapse));
                Assert.IsNotNull(datePickerAutomationPeer);
                Assert.AreEqual(datePickerExpandCollapseProvider.ExpandCollapseState, ExpandCollapseState.Collapsed);
                _elementToCleanUp.IsDropDownOpen = true;
                Assert.IsTrue(_elementToCleanUp.TestHook.DropDown.IsOpen);
                Assert.AreEqual(datePickerExpandCollapseProvider.ExpandCollapseState, ExpandCollapseState.Expanded);
                datePickerExpandCollapseProvider.Collapse();
                Assert.AreEqual(datePickerExpandCollapseProvider.ExpandCollapseState, ExpandCollapseState.Collapsed);
                Assert.IsFalse(_elementToCleanUp.TestHook.DropDown.IsOpen);
                datePickerExpandCollapseProvider.Expand();
                Assert.IsTrue(_elementToCleanUp.TestHook.DropDown.IsOpen);
                Assert.AreEqual(datePickerExpandCollapseProvider.ExpandCollapseState, ExpandCollapseState.Expanded);
                datePickerExpandCollapseProvider.Collapse();

                #endregion

                #region DatePickerAutomationPeer IValueProvider tests:

                IValueProvider datePickerValueProvider = ((IValueProvider)datePickerAutomationPeer.GetPattern(PatternInterface.Value));
                Assert.IsNotNull(datePickerValueProvider);
                Assert.IsFalse(datePickerValueProvider.IsReadOnly);
                Assert.AreEqual(datePickerValueProvider.Value, date.ToString());
                _elementToCleanUp.SelectedDate = null;
                Assert.AreEqual(datePickerValueProvider.Value, string.Empty);
                datePickerValueProvider.SetValue(date2.ToString());
                Assert.AreEqual(_elementToCleanUp.SelectedDate, date2);
                Assert.AreEqual(datePickerValueProvider.Value, date2.ToString());
                #endregion
            });
            EnqueueSleep(sleepTime);
            EnqueueCallback(delegate
            {
                Assert.AreEqual(_elementToCleanUp.Text, date2.ToString("d"));
            });

            EnqueueTestComplete();
        }

        #endregion

        private static bool CompareDates(DateTime d1, DateTime d2)
        {
            return (d1.Year == d2.Year &&
                    d1.Month == d2.Month &&
                    d1.Day == d2.Day);
        }

        [TestCleanup]
        public void RemoveControlsFromSurface()
        {
            if (_elementToCleanUp != null)
            {
                TestPanel.Children.Remove(_elementToCleanUp);
                _elementToCleanUp = null;
            }
        }

    }
}
