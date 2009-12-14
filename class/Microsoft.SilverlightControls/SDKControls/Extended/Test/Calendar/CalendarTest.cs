// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Collections;
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
    /// Unit tests for System.Windows.Controls.Calendar.
    /// </summary>
    [TestClass]
    public partial class CalendarTest : SilverlightTest
    {
        const int sleepTime = 500;

        #region CalendarTests

        /// <summary>
        /// Create a Calendar control.
        /// </summary>
        [TestMethod]
        [Description("Create a Calendar control.")]
        public void Create()
        {
            Calendar calendar = new Calendar();
            Assert.IsNotNull(calendar);
        }

        /// <summary>
        /// Create a Calendar in XAML markup.
        /// </summary>
        [TestMethod]
        [Description("Create a Calendar in XAML markup.")]
        [Asynchronous]
        public void CreateInXaml()
        {
            object result = XamlReader.Load("<toolkit:Calendar  SelectedDate=\"04/30/2008\" DisplayDateStart=\"04/30/2020\" DisplayDateEnd=\"04/30/2010\" DisplayDate=\"02/02/2000\" xmlns='http://schemas.microsoft.com/client/2007'" +
  " xmlns:toolkit=\"clr-namespace:System.Windows.Controls;assembly=System.Windows.Controls\" />");
            Assert.IsInstanceOfType(result, typeof(Calendar));
            ((Calendar)result).Loaded += new RoutedEventHandler(calendar_Loaded);
            TestPanel.Children.Add((Calendar)result);
            EnqueueConditional(IsLoaded);
            EnqueueCallback(() => VerifyXamlValues((Calendar)result));
            EnqueueTestComplete();
        }

        private void VerifyXamlValues(Calendar calendar)
        {
            Assert.IsTrue(CompareDates(calendar.SelectedDate.Value, new DateTime(2008, 4, 30)));
            Assert.IsTrue(CompareDates(calendar.DisplayDateStart.Value, new DateTime(2008, 4, 30)));
            Assert.IsTrue(CompareDates(calendar.DisplayDate, new DateTime(2008, 4, 30)));
            Assert.IsTrue(CompareDates(calendar.DisplayDateEnd.Value, new DateTime(2010, 4, 30)));
            _isLoaded = false;
        }

        /// <summary>
        /// Ensure all default values are correct.
        /// </summary>
        [TestMethod]
        [Description("Ensure all default values are correct.")]
        [Asynchronous]
        public void CheckDefaultValues()
        {
            Calendar calendar = new Calendar();
            Assert.IsTrue(CompareDates(DateTime.Today, calendar.DisplayDate));
            Assert.IsNull(calendar.DisplayDateStart);
            Assert.IsNull(calendar.DisplayDateEnd);
            Assert.AreEqual(calendar.FirstDayOfWeek, DateTimeHelper.GetCurrentDateFormat().FirstDayOfWeek);
            Assert.IsTrue(calendar.IsTodayHighlighted);
            Assert.IsNull(calendar.SelectedDate);
            Assert.IsTrue(calendar.SelectedDates.Count == 0);
            Assert.IsTrue(calendar.BlackoutDates.Count == 0);
            Assert.IsTrue(calendar.IsEnabled);
            Assert.IsTrue(calendar.DisplayMode == CalendarMode.Month);
            Assert.IsTrue(calendar.SelectionMode == CalendarSelectionMode.SingleDate);
            calendar.Loaded += new RoutedEventHandler(calendar_Loaded);
            TestPanel.Children.Add(calendar);
            EnqueueConditional(IsLoaded);
            EnqueueCallback(() => VerifyDefaultValues(calendar));
            EnqueueTestComplete();
        }

        private void VerifyDefaultValues(Calendar calendar)
        {
            Assert.IsTrue(calendar.TestHook.MonthControl.TestHook.PreviousButton.IsEnabled);
            Assert.IsTrue(calendar.TestHook.MonthControl.TestHook.NextButton.IsEnabled);
            Assert.IsTrue(calendar.TestHook.MonthControl.TestHook.HeaderButton.IsEnabled);
            Assert.IsTrue(calendar.TestHook.MonthControl.TestHook.HeaderButton.Content.ToString() == calendar.DisplayDate.ToString("Y", DateTimeHelper.GetCurrentDateFormat()));
            _isLoaded = false;
        }

        /// <summary>
        /// Ensure Nullable Properties can be set to null.
        /// </summary>
        [TestMethod]
        [Description("Ensure Nullable Properties can be set to null.")]
        [Asynchronous]
        public void ArePropertiesNullable()
        {
            Calendar calendar = new Calendar();
            DateTime value = DateTime.Today;

            calendar.SelectedDate = null;
            Assert.IsNull(calendar.SelectedDate);

            calendar.SelectedDate = value;
            Assert.IsTrue(CompareDates(value, calendar.SelectedDate.Value));

            calendar.SelectedDate = null;
            Assert.IsNull(calendar.SelectedDate);

            calendar.DisplayDateStart = null;
            Assert.IsNull(calendar.DisplayDateStart);

            calendar.DisplayDateStart = value;
            Assert.IsTrue(CompareDates(value, calendar.DisplayDateStart.Value));

            calendar.DisplayDateStart = null;
            Assert.IsNull(calendar.DisplayDateStart);

            calendar.DisplayDateEnd = null;
            Assert.IsNull(calendar.DisplayDateEnd);

            calendar.DisplayDateEnd = value;
            Assert.IsTrue(CompareDates(value, calendar.DisplayDateEnd.Value));

            calendar.DisplayDateEnd = null;
            Assert.IsNull(calendar.DisplayDateEnd);

            calendar.Loaded += new RoutedEventHandler(calendar_Loaded);
            TestPanel.Children.Add(calendar);
            EnqueueConditional(IsLoaded);
            EnqueueCallback(() => VerifyNullValue(calendar));
            EnqueueTestComplete();
        }

        private void VerifyNullValue(Calendar calendar)
        {
            Assert.IsTrue(calendar.TestHook.MonthControl.TestHook.PreviousButton.IsEnabled);
            Assert.IsTrue(calendar.TestHook.MonthControl.TestHook.NextButton.IsEnabled);
            Assert.IsTrue(calendar.TestHook.MonthControl.TestHook.HeaderButton.IsEnabled);
            Assert.IsTrue(calendar.TestHook.MonthControl.TestHook.HeaderButton.Content.ToString() == calendar.DisplayDate.ToString("Y", DateTimeHelper.GetCurrentDateFormat()));
            _isLoaded = false;
        }

        /// <summary>
        /// Ensure DateTime Properties can be set to DateTime.MaxValue.
        /// </summary>
        [TestMethod]
        [Description("Ensure DateTime Properties can be set to DateTime.MaxValue")]
        [Asynchronous]
        public void SetToMaxValue()
        {
            Calendar calendar = new Calendar();
            calendar.DisplayDate = DateTime.MaxValue;
            Assert.IsTrue(CompareDates(DateTime.MaxValue, calendar.DisplayDate));

            calendar.DisplayDateEnd = DateTime.MinValue;
            calendar.DisplayDateStart = DateTime.MaxValue;
            Assert.IsTrue(CompareDates(calendar.DisplayDateStart.Value, DateTime.MaxValue));
            Assert.IsTrue(CompareDates(calendar.DisplayDateEnd.Value, DateTime.MaxValue));

            calendar.SelectedDate = DateTime.MaxValue;
            Assert.IsTrue(CompareDates(DateTime.MaxValue, (DateTime)calendar.SelectedDate));

            Assert.IsTrue(calendar.SelectedDates.Count == 1);
            Assert.IsTrue(CompareDates(calendar.SelectedDates[0], calendar.SelectedDate.Value));

            calendar.SelectedDates.Clear();
            calendar.BlackoutDates.Add(new CalendarDateRange(DateTime.MaxValue));
            Assert.IsTrue(CompareDates(calendar.BlackoutDates[0].End, DateTime.MaxValue));
            Assert.IsTrue(CompareDates(calendar.BlackoutDates[0].Start, DateTime.MaxValue));

            calendar.Loaded += new RoutedEventHandler(calendar_Loaded);
            TestPanel.Children.Add(calendar);
            EnqueueConditional(IsLoaded);
            EnqueueCallback(() => VerifyMaxValue(calendar));
            EnqueueTestComplete();
        }

        private void VerifyMaxValue(Calendar calendar)
        {
            Assert.IsFalse(calendar.TestHook.MonthControl.TestHook.PreviousButton.IsEnabled);
            Assert.IsFalse(calendar.TestHook.MonthControl.TestHook.NextButton.IsEnabled);
            Assert.IsTrue(calendar.TestHook.MonthControl.TestHook.HeaderButton.IsEnabled);
            Assert.IsTrue(calendar.TestHook.MonthControl.TestHook.HeaderButton.Content.ToString() == calendar.DisplayDate.ToString("Y", DateTimeHelper.GetCurrentDateFormat()));
            _isLoaded = false;
        }

        /// <summary>
        /// Ensure DateTime Properties can be set to DateTime.MinValue.
        /// </summary>
        [TestMethod]
        [Description("Ensure DateTime Properties can be set to DateTime.MinValue")]
        [Asynchronous]
        public void SetToMinValue()
        {
            Calendar calendar = new Calendar();
            calendar.DisplayDate = DateTime.MinValue;
            Assert.IsTrue(CompareDates(calendar.DisplayDate, DateTime.MinValue));

            calendar.DisplayDateStart = DateTime.MinValue;
            calendar.DisplayDateEnd = DateTime.MinValue;
            Assert.IsTrue(CompareDates(calendar.DisplayDateStart.Value, DateTime.MinValue));
            Assert.IsTrue(CompareDates(calendar.DisplayDateEnd.Value, DateTime.MinValue));

            calendar.SelectedDate = DateTime.MinValue;
            Assert.IsTrue(CompareDates(DateTime.MinValue, (DateTime)calendar.SelectedDate));

            Assert.IsTrue(calendar.SelectedDates.Count == 1);
            Assert.IsTrue(CompareDates(calendar.SelectedDates[0], calendar.SelectedDate.Value));

            calendar.SelectedDates.Clear();
            calendar.BlackoutDates.Add(new CalendarDateRange(DateTime.MinValue));
            Assert.IsTrue(CompareDates(calendar.BlackoutDates[0].End, DateTime.MinValue));
            Assert.IsTrue(CompareDates(calendar.BlackoutDates[0].Start, DateTime.MinValue));

            calendar.Loaded += new RoutedEventHandler(calendar_Loaded);
            TestPanel.Children.Add(calendar);
            EnqueueConditional(IsLoaded);
            EnqueueCallback(() => VerifyMinValue(calendar));
            EnqueueTestComplete();
        }

        private void VerifyMinValue(Calendar calendar)
        {
            Assert.IsFalse(calendar.TestHook.MonthControl.TestHook.PreviousButton.IsEnabled);
            Assert.IsFalse(calendar.TestHook.MonthControl.TestHook.NextButton.IsEnabled);
            Assert.IsTrue(calendar.TestHook.MonthControl.TestHook.HeaderButton.IsEnabled);
            Assert.IsTrue(calendar.TestHook.MonthControl.TestHook.HeaderButton.Content.ToString() == calendar.DisplayDate.ToString("Y", DateTimeHelper.GetCurrentDateFormat()));
            _isLoaded = false;
        }
        #endregion CalendarTests

        #region Events
        /// <summary>
        /// Ensure DateSelected event is fired.
        /// </summary>
        [TestMethod]
        [Description("Ensure DateSelected event is fired.")]
        public void DateSelectedEvent()
        {
            string testString = null;
            Calendar calendar = new Calendar();
            calendar.SelectionMode = CalendarSelectionMode.SingleDate;
            calendar.SelectedDatesChanged += new EventHandler<SelectionChangedEventArgs>(delegate
            {
                testString = "Handled!";
            });
            DateTime value = new DateTime(2000, 10, 10);
            calendar.SelectedDate = value;
            Assert.AreEqual(testString, "Handled!");
            Assert.AreEqual(calendar.ToString(), value.ToString());
        }

        /// <summary>
        /// Ensure DisplayDateChanged event is fired.
        /// </summary>
        [TestMethod]
        [Description("Ensure DisplayDateChanged event is fired.")]
        public void DisplayDateChangedEvent()
        {
            string testString = null;
            Calendar calendar = new Calendar();
            calendar.SelectionMode = CalendarSelectionMode.SingleDate;
            calendar.DisplayDateChanged += new EventHandler<CalendarDateChangedEventArgs>(delegate
            {
                testString = "Handled!";
            });
            DateTime value = new DateTime(2000, 10, 10);
            calendar.DisplayDate = value;
            Assert.AreEqual(testString, "Handled!");
        }

        #endregion Events

        #region DependencyProperties

        #region BlackoutDates

        /// <summary>
        /// Set the value of SelectedDateProperty.
        /// </summary>
        [TestMethod]
        [Description("Set the value of SelectedDateProperty.")]
        public void BlackoutDatesSingleDay()
        {
            Calendar calendar = new Calendar();
            calendar.BlackoutDates.AddDatesInPast();
            Common.AssertExpectedExceptionWithoutMessageControl(new ArgumentOutOfRangeException(), () => { calendar.SelectedDate = DateTime.Today.AddDays(-1); });
        }

        [TestMethod]
        public void BlackoutDatesRange()
        {
            Calendar calendar = new Calendar();
            calendar.BlackoutDates.Add(new CalendarDateRange(DateTime.Today, DateTime.Today.AddDays(10)));
            calendar.SelectedDate = DateTime.Today.AddDays(-1);
            Assert.IsTrue(CompareDates(calendar.SelectedDate.Value, DateTime.Today.AddDays(-1)));
            Assert.IsTrue(CompareDates(calendar.SelectedDate.Value, calendar.SelectedDates[0]));
            calendar.SelectedDate = DateTime.Today.AddDays(11);
            Assert.IsTrue(CompareDates(calendar.SelectedDate.Value, DateTime.Today.AddDays(11)));
            Assert.IsTrue(CompareDates(calendar.SelectedDate.Value, calendar.SelectedDates[0]));
            Common.AssertExpectedExceptionWithoutMessageControl(new ArgumentOutOfRangeException(), () => { calendar.SelectedDate = DateTime.Today.AddDays(5); });
        }

        [TestMethod]
        public void SetBlackoutDatesRange()
        {
            Calendar calendar = new Calendar();
            calendar.SelectedDate = DateTime.Today.AddDays(5);
            Common.AssertExpectedExceptionWithoutMessageControl(new ArgumentOutOfRangeException(), () => { calendar.BlackoutDates.Add(new CalendarDateRange(DateTime.Today, DateTime.Today.AddDays(10))); });
        }

        [TestMethod]
        public void SetBlackoutDatesRangeDisplayStart()
        {
            Calendar calendar = new Calendar();
            calendar.DisplayDateStart = DateTime.Today.AddDays(-5);
            calendar.BlackoutDates.Add(new CalendarDateRange(DateTime.Today.AddDays(-10), DateTime.Today.AddDays(10)));
        }

        [TestMethod]
        public void SetBlackoutDatesRangeDisplayEnd()
        {
            Calendar calendar = new Calendar();
            calendar.DisplayDateEnd = DateTime.Today.AddDays(5);
            calendar.BlackoutDates.Add(new CalendarDateRange(DateTime.Today, DateTime.Today.AddDays(10)));
        }

        #endregion BlackoutDates

        #region DisplayDate
        /// <summary>
        /// Set the value of DisplayDateProperty.
        /// </summary>
        [TestMethod]
        [Description("Set the value of DisplayDateProperty.")]
        public void DisplayDatePropertySetValue()
        {
            Calendar calendar = new Calendar();
            calendar.SelectionMode = CalendarSelectionMode.SingleDate;
            DateTime value = DateTime.Today.AddMonths(3);
            calendar.DisplayDate = value;
            Assert.IsTrue(CompareDates(calendar.DisplayDate, value));
        }


        [TestMethod]
        public void DisplayDateStartEnd()
        {
            Calendar calendar = new Calendar();
            calendar.SelectionMode = CalendarSelectionMode.SingleDate;
            calendar.DisplayDateStart = new DateTime(2005, 12, 30);

            DateTime value = new DateTime(2005, 12, 15);
            calendar.DisplayDate = value;

            Assert.IsTrue(CompareDates(calendar.DisplayDate, calendar.DisplayDateStart.Value));

            value = new DateTime(2005, 12, 30);
            calendar.DisplayDate = value;

            Assert.IsTrue(CompareDates(calendar.DisplayDate, value));

            value = DateTime.MaxValue;
            calendar.DisplayDate = value;

            Assert.IsTrue(CompareDates(calendar.DisplayDate, value));

            calendar.DisplayDateEnd = new DateTime(2010, 12, 30);
            Assert.IsTrue(CompareDates(calendar.DisplayDate, calendar.DisplayDateEnd.Value));
        }
        #endregion DisplayDate

        #region DisplayDateRange

        [TestMethod]
        public void DisplayDateRangeEnd()
        {
            Calendar calendar = new Calendar();
            DateTime value = new DateTime(2000, 1, 30);
            calendar.DisplayDate = value;
            calendar.DisplayDateEnd = value;
            calendar.DisplayDateStart = value;
            Assert.IsTrue(CompareDates(calendar.DisplayDateStart.Value, value));
            Assert.IsTrue(CompareDates(calendar.DisplayDateEnd.Value, value));

            value = value.AddMonths(2);
            calendar.DisplayDateStart = value;

            Assert.IsTrue(CompareDates(calendar.DisplayDateStart.Value, value));
            Assert.IsTrue(CompareDates(calendar.DisplayDateEnd.Value, value));
            Assert.IsTrue(CompareDates(calendar.DisplayDate, value));
        }

        [TestMethod]
        public void DisplayDateRangeEndSelectedDate()
        {
            Calendar calendar = new Calendar();
            calendar.SelectionMode = CalendarSelectionMode.SingleDate;
            calendar.SelectedDate = DateTime.MaxValue;
            Assert.IsTrue(CompareDates((DateTime)calendar.SelectedDate, DateTime.MaxValue));
            calendar.DisplayDateEnd = DateTime.MaxValue.AddDays(-1);
            Assert.IsTrue(CompareDates((DateTime)calendar.DisplayDateEnd, DateTime.MaxValue));
        }

        [TestMethod]
        public void DisplayDateRangeStartOutOfRangeExceptionSelectedDate()
        {
            Calendar calendar = new Calendar();
            calendar.SelectionMode = CalendarSelectionMode.SingleDate;
            calendar.SelectedDate = DateTime.MinValue;
            Assert.IsTrue(CompareDates((DateTime)calendar.SelectedDate, DateTime.MinValue));
            calendar.DisplayDateStart = DateTime.MinValue.AddDays(1);
            Assert.IsTrue(CompareDates((DateTime)calendar.DisplayDateStart, DateTime.MinValue));
        }

        ///<summary>
        ///Ensure ArgumentOutOfRangeException is thrown.
        ///</summary>
        [TestMethod]
        [Description("Ensure ArgumentOutOfRangeException is thrown.")]
        public void DisplayDateRangeEndBlackoutDayStart()
        {
            Calendar calendar = new Calendar();
            calendar.SelectionMode = CalendarSelectionMode.SingleDate;
            calendar.BlackoutDates.Add(new CalendarDateRange(DateTime.Today, DateTime.Today));
            calendar.DisplayDateEnd = DateTime.Today.AddDays(-1);
        }

        ///<summary>
        ///Ensure ArgumentOutOfRangeException is thrown.
        ///</summary>
        [TestMethod]
        [Description("Ensure ArgumentOutOfRangeException is thrown.")]
        public void DisplayDateRangeEndBlackoutDayEnd()
        {
            Calendar calendar = new Calendar();
            calendar.SelectionMode = CalendarSelectionMode.SingleDate;
            calendar.BlackoutDates.Add(new CalendarDateRange(DateTime.Today, DateTime.Today.AddDays(4)));
            calendar.DisplayDateEnd = DateTime.Today;
        }

        #endregion DisplayDateRange

        #region DisplayMode

        /// <summary>
        /// Ensure ArgumentOutOfRangeException is thrown.
        /// </summary>
        [TestMethod]
        [Description("Ensure ArgumentOutOfRangeException is thrown.")]
        public void DisplayModeOutOfRangeException()
        {
            Calendar calendar = new Calendar();
            Common.AssertExpectedExceptionWithoutMessageControl(new ArgumentOutOfRangeException(), () => { calendar.DisplayMode = (CalendarMode)4; });
        }

        [TestMethod]
        [Asynchronous]
        public void DisplayModeYearToMonth()
        {
            Calendar calendar = new Calendar();
            calendar.DisplayMode = CalendarMode.Year;
            calendar.Loaded += new RoutedEventHandler(calendar_Loaded);
            calendar.DisplayModeChanged += new EventHandler<CalendarModeChangedEventArgs>(calendar_DisplayModeChanged);
            TestPanel.Children.Add(calendar);
            EnqueueConditional(IsLoaded);
            EnqueueCallback(() => VerifyYearModeToMonth(calendar));
            EnqueueTestComplete();
        }

        bool _modeChanged;

        private void calendar_DisplayModeChanged(object sender, CalendarModeChangedEventArgs e)
        {
            _modeChanged = true;
        }

        private void VerifyYearModeToMonth(Calendar calendar)
        {
            CalendarItem month = calendar._root.Children[0] as CalendarItem;
            Assert.IsTrue(month.MonthView.Visibility == Visibility.Collapsed);
            Assert.IsTrue(month.YearView.Visibility == Visibility.Visible);
            Assert.IsTrue(calendar.DisplayMode == CalendarMode.Year);
            _modeChanged = false;
            month.TestHook.Month_CalendarButtonMouseUp(month.YearView.Children[0], null);
            Assert.IsTrue(_modeChanged);
            Assert.IsTrue(calendar.DisplayMode == CalendarMode.Month);
            _isLoaded = false;
        }

        [TestMethod]
        [Asynchronous]
        public void DisplayModeYearToDecade()
        {
            Calendar calendar = new Calendar();
            calendar.DisplayMode = CalendarMode.Year;
            calendar.Loaded += new RoutedEventHandler(calendar_Loaded);
            TestPanel.Children.Add(calendar);
            EnqueueConditional(IsLoaded);
            EnqueueCallback(() => VerifyYearModeToDecade(calendar));
            EnqueueTestComplete();
        }

        private void VerifyYearModeToDecade(Calendar calendar)
        {
            CalendarItem month = calendar._root.Children[0] as CalendarItem;
            Assert.IsTrue(month.MonthView.Visibility == Visibility.Collapsed);
            Assert.IsTrue(month.YearView.Visibility == Visibility.Visible);
            Assert.IsTrue(calendar.DisplayMode == CalendarMode.Year);
            month.TestHook.HeaderButton_Click(month.TestHook.HeaderButton, new RoutedEventArgs());
            Assert.IsTrue(calendar.DisplayMode == CalendarMode.Decade);
            _isLoaded = false;
        }

        [TestMethod]
        [Asynchronous]
        public void DisplayModeDecadeToYear()
        {
            Calendar calendar = new Calendar();
            calendar.DisplayMode = CalendarMode.Decade;
            calendar.Loaded += new RoutedEventHandler(calendar_Loaded);
            TestPanel.Children.Add(calendar);
            EnqueueConditional(IsLoaded);
            EnqueueCallback(() => VerifyDecadeToYear(calendar));
            EnqueueTestComplete();
        }

        private void VerifyDecadeToYear(Calendar calendar)
        {
            CalendarItem month = calendar._root.Children[0] as CalendarItem;
            Assert.IsTrue(month.MonthView.Visibility == Visibility.Collapsed);
            Assert.IsTrue(month.YearView.Visibility == Visibility.Visible);
            Assert.IsTrue(calendar.DisplayMode == CalendarMode.Decade);
            month.TestHook.Month_CalendarButtonMouseUp(month.YearView.Children[0], null);
            Assert.IsTrue(calendar.DisplayMode == CalendarMode.Year);
            _isLoaded = false;
        }

        [TestMethod]
        [Asynchronous]
        public void DisplayModeDecadeToMonth()
        {
            Calendar calendar = new Calendar();
            calendar.DisplayDate = new DateTime(2000, 1, 1);
            calendar.DisplayDateStart = new DateTime(2000, 1, 1);
            calendar.DisplayDateEnd = new DateTime(2000, 1, 1);
            calendar.DisplayMode = CalendarMode.Decade;
            calendar.Loaded += new RoutedEventHandler(calendar_Loaded);
            TestPanel.Children.Add(calendar);
            EnqueueConditional(IsLoaded);
            EnqueueCallback(() => VerifyDecadeToMonth(calendar));
            EnqueueTestComplete();
        }

        private void VerifyDecadeToMonth(Calendar calendar)
        {
            CalendarItem month = calendar._root.Children[0] as CalendarItem;
            Assert.IsTrue(month.MonthView.Visibility == Visibility.Collapsed);
            Assert.IsTrue(month.YearView.Visibility == Visibility.Visible);
            Assert.IsTrue(calendar.DisplayMode == CalendarMode.Decade);
            Assert.IsFalse(month.TestHook.HeaderButton.IsEnabled);
            Assert.IsFalse(month.TestHook.PreviousButton.IsEnabled);
            Assert.IsFalse(month.TestHook.NextButton.IsEnabled);
            calendar.DisplayMode = CalendarMode.Month;
            Assert.IsTrue(month.TestHook.HeaderButton.IsEnabled);
            Assert.IsFalse(month.TestHook.PreviousButton.IsEnabled);
            Assert.IsFalse(month.TestHook.NextButton.IsEnabled);
            Assert.IsTrue(calendar.DisplayMode == CalendarMode.Month);
            Assert.IsTrue(month.MonthView.Visibility == Visibility.Visible);
            Assert.IsTrue(month.YearView.Visibility == Visibility.Collapsed);
            calendar.DisplayMode = CalendarMode.Year;
            Assert.IsTrue(month.MonthView.Visibility == Visibility.Collapsed);
            Assert.IsTrue(month.YearView.Visibility == Visibility.Visible);
            _isLoaded = false;
        }

        [TestMethod]
        [Asynchronous]
        public void DisplayModeDecadeMinValue()
        {
            Calendar calendar = new Calendar();
            calendar.DisplayDate = DateTime.MinValue;
            calendar.DisplayMode = CalendarMode.Decade;
            calendar.Loaded += new RoutedEventHandler(calendar_Loaded);
            TestPanel.Children.Add(calendar);
            EnqueueConditional(IsLoaded);
            EnqueueCallback(() => VerifyDisplayModeDecadeMinValue(calendar));
            EnqueueTestComplete();
        }

        private void VerifyDisplayModeDecadeMinValue(Calendar calendar)
        {
            Assert.IsFalse(calendar.TestHook.MonthControl.TestHook.PreviousButton.IsEnabled);
            Assert.IsTrue(calendar.TestHook.MonthControl.TestHook.NextButton.IsEnabled);
            _isLoaded = false;
        }

        [TestMethod]
        [Asynchronous]
        public void DisplayModeDecadeMaxValue()
        {
            Calendar calendar = new Calendar();
            calendar.DisplayDate = DateTime.MaxValue;
            calendar.DisplayMode = CalendarMode.Decade;
            calendar.Loaded += new RoutedEventHandler(calendar_Loaded);
            TestPanel.Children.Add(calendar);
            EnqueueConditional(IsLoaded);
            EnqueueCallback(() => VerifyDisplayModeDecadeMaxValue(calendar));
            EnqueueTestComplete();
        }

        private void VerifyDisplayModeDecadeMaxValue(Calendar calendar)
        {
            Assert.IsFalse(calendar.TestHook.MonthControl.TestHook.NextButton.IsEnabled);
            Assert.IsTrue(calendar.TestHook.MonthControl.TestHook.PreviousButton.IsEnabled);
            _isLoaded = false;
        }

        [TestMethod]
        [Asynchronous]
        public void DisplayModeOneWeek()
        {
            Calendar calendar = new Calendar();
            calendar.DisplayDateStart = new DateTime(2000, 2, 2);
            calendar.DisplayDateEnd = new DateTime(2000, 2, 5);
            calendar.DisplayMode = CalendarMode.Year;
            calendar.Loaded += new RoutedEventHandler(calendar_Loaded);
            TestPanel.Children.Add(calendar);
            EnqueueConditional(IsLoaded);
            EnqueueCallback(() => VerifyDisplayModeOneWeek(calendar));
            EnqueueTestComplete();
        }

        private void VerifyDisplayModeOneWeek(Calendar calendar)
        {
            Assert.IsTrue(((CalendarButton)calendar.TestHook.MonthControl.YearView.Children[1]).IsEnabled);
            calendar.DisplayMode = CalendarMode.Decade;
            Assert.IsTrue(((CalendarButton)calendar.TestHook.MonthControl.YearView.Children[1]).IsEnabled);
            _isLoaded = false;
        }

        [TestMethod]
        [Asynchronous]
        public void DisplayModeVisibleDayCount()
        {
            Calendar calendar = new Calendar();
            calendar.DisplayDateStart = new DateTime(2008, 6, 15);
            calendar.DisplayDateEnd = new DateTime(2008, 6, 29);
            calendar.Loaded += new RoutedEventHandler(calendar_Loaded);
            TestPanel.Children.Add(calendar);
            EnqueueConditional(IsLoaded);
            EnqueueCallback(() => VerifyDisplayModeVisibleDayCount(calendar));
            EnqueueTestComplete();
        }

        private void VerifyDisplayModeVisibleDayCount(Calendar calendar)
        {
            calendar.DisplayMode = CalendarMode.Year;
            calendar.DisplayMode = CalendarMode.Month;
            Assert.IsFalse((calendar.FindDayButtonFromDay(new DateTime(2008, 6, 14))).IsEnabled);
            Assert.IsFalse((calendar.FindDayButtonFromDay(new DateTime(2008, 6, 30))).IsEnabled);
            _isLoaded = false;
        }

        #endregion DisplayMode

        #region FirstDayOfWeek
        /// <summary>
        /// Set the value of FirstDayOfWeekProperty.
        /// </summary>
        [TestMethod]
        [Description("Set the value of FirstDayOfWeekProperty.")]
        public void FirstDayOfWeekPropertySetValue()
        {
            Calendar calendar = new Calendar();
            DayOfWeek value = DayOfWeek.Thursday;
            calendar.FirstDayOfWeek = value;

            Assert.AreEqual(value, calendar.GetValue(Calendar.FirstDayOfWeekProperty));
            Assert.AreEqual(value, calendar.FirstDayOfWeek);

            value = (DayOfWeek)3;
            calendar.FirstDayOfWeek = value;
            Assert.AreEqual(value, calendar.FirstDayOfWeek);
        }

        /// <summary>
        /// Ensure ArgumentOutOfRangeException is thrown.
        /// </summary>
        [TestMethod]
        [Description("Ensure ArgumentOutOfRangeException is thrown.")]
        public void FirstDayOfWeekOutOfRangeException()
        {
            Calendar calendar = new Calendar();
            Common.AssertExpectedExceptionWithoutMessageControl(new ArgumentOutOfRangeException(), () => { calendar.FirstDayOfWeek = (DayOfWeek)7; });
        }

        [TestMethod]
        [Asynchronous]
        public void DisplayModeDisplayDate()
        {
            Calendar calendar = new Calendar();
            calendar.Loaded += new RoutedEventHandler(calendar_Loaded);
            TestPanel.Children.Add(calendar);
            EnqueueConditional(IsLoaded);
            EnqueueCallback(() => VerifyDisplayModeYearDisplayDate(calendar));
            EnqueueTestComplete();
        }

        private void VerifyDisplayModeYearDisplayDate(Calendar calendar)
        {
            calendar.DisplayMode = CalendarMode.Year;
            Assert.IsTrue(calendar.TestHook.MonthControl.TestHook.HeaderButton.Content.ToString() == calendar.DisplayDate.Year.ToString());
        }

        [TestMethod]
        [Asynchronous]
        public void DisplayModeDecadeDisplayDate()
        {
            Calendar calendar = new Calendar();
            calendar.Loaded += new RoutedEventHandler(calendar_Loaded);
            TestPanel.Children.Add(calendar);
            EnqueueConditional(IsLoaded);
            EnqueueCallback(() => VerifyDisplayModeDecadeDisplayDate(calendar));
            EnqueueTestComplete();
        }

        private void VerifyDisplayModeDecadeDisplayDate(Calendar calendar)
        {
            calendar.DisplayMode = CalendarMode.Decade;
            Assert.IsTrue(calendar.DisplayMode == CalendarMode.Decade);
        }

        #endregion FirstDayOfWeek

        #region IsTodayHighlighted

        [TestMethod]
        [Description("Verify IsTodayHighlighted property.")]
        [Asynchronous]
        public void IsTodayHighlighted()
        {
            Calendar calendar = new Calendar();
            calendar.Loaded += new RoutedEventHandler(calendar_Loaded);
            TestPanel.Children.Add(calendar);
            EnqueueConditional(IsLoaded);
            EnqueueCallback(() => VerifyIsTodayHighlighted(calendar));
            EnqueueTestComplete();
        }

        private void VerifyIsTodayHighlighted(Calendar calendar)
        {
            CalendarDayButton b = calendar.FindDayButtonFromDay(DateTime.Today);
            Assert.IsTrue(b.IsToday);
            calendar.IsTodayHighlighted = false;
            Assert.IsFalse(b.IsToday);
            _isLoaded = false;
        }

        #endregion IsTodayHighlighted

        #region SelectedDate/SelectedDates

        #region SingleMode

        [TestMethod]
        public void SelectedDateSingle()
        {
            Clear();
            Calendar calendar = new Calendar();
            calendar.SelectedDatesChanged += new EventHandler<SelectionChangedEventArgs>(calendar_SelectedDatesChanged);
            calendar.SelectionMode = CalendarSelectionMode.SingleDate;
            calendar.SelectedDate = DateTime.Today;
            Assert.IsTrue(CompareDates(calendar.SelectedDate.Value, DateTime.Today));
            Assert.IsTrue(calendar.SelectedDates.Count == 1);
            Assert.IsTrue(CompareDates(calendar.SelectedDates[0], DateTime.Today));
            Assert.IsTrue(_eventCount == 1);
            Assert.IsTrue(_addedDays.Count == 1);
            Assert.IsTrue(_removedDays.Count == 0);
            Clear();

            calendar.SelectedDate = DateTime.Today;
            Assert.IsTrue(CompareDates(calendar.SelectedDate.Value, DateTime.Today));
            Assert.IsTrue(calendar.SelectedDates.Count == 1);
            Assert.IsTrue(CompareDates(calendar.SelectedDates[0], DateTime.Today));
            Assert.IsTrue(_eventCount == 0);

            calendar.SelectionMode = CalendarSelectionMode.None;
            Assert.IsTrue(calendar.SelectedDates.Count == 0);
            Assert.IsNull(calendar.SelectedDate);

            calendar.SelectionMode = CalendarSelectionMode.SingleDate;

            calendar.SelectedDates.Add(DateTime.Today.AddDays(1));
            Assert.IsTrue(CompareDates(calendar.SelectedDate.Value, DateTime.Today.AddDays(1)));
            Assert.IsTrue(calendar.SelectedDates.Count == 1);

            Common.AssertExpectedExceptionWithoutMessageControl(new InvalidOperationException(), () => { calendar.SelectedDates.Add(DateTime.Today.AddDays(2)); });
        }

        #endregion SingleMode

        #region SingleRangeMode

        [TestMethod]
        public void SelectedDateSingleRange()
        {
            Clear();
            Calendar calendar = new Calendar();
            calendar.SelectedDatesChanged += new EventHandler<SelectionChangedEventArgs>(calendar_SelectedDatesChanged);
            calendar.SelectionMode = CalendarSelectionMode.SingleRange;
            calendar.SelectedDate = DateTime.Today;
            Assert.IsTrue(CompareDates(calendar.SelectedDate.Value, DateTime.Today));
            Assert.IsTrue(calendar.SelectedDates.Count == 1);
            Assert.IsTrue(CompareDates(calendar.SelectedDates[0], DateTime.Today));
            Assert.IsTrue(_eventCount == 1);
            Assert.IsTrue(_addedDays.Count == 1);
            Assert.IsTrue(_removedDays.Count == 0);
            Clear();

            calendar.SelectedDates.Clear();
            Assert.IsNull(calendar.SelectedDate);

            Clear();
            calendar.SelectedDates.AddRange(DateTime.Today, DateTime.Today.AddDays(10));
            Assert.IsTrue(CompareDates(calendar.SelectedDate.Value, DateTime.Today));
            Assert.IsTrue(calendar.SelectedDates.Count == 11);

            Clear();
            calendar.SelectedDates.AddRange(DateTime.Today, DateTime.Today.AddDays(10));
            Assert.IsTrue(calendar.SelectedDates.Count == 11);
            Assert.IsTrue(_eventCount == 0);

            calendar.SelectedDates.AddRange(DateTime.Today.AddDays(-20), DateTime.Today);
            Assert.IsTrue(CompareDates(calendar.SelectedDate.Value, DateTime.Today.AddDays(-20)));
            Assert.IsTrue(calendar.SelectedDates.Count == 21);
            Assert.IsTrue(_eventCount == 1);
            Assert.IsTrue(_addedDays.Count == 21);
            Assert.IsTrue(_removedDays.Count == 11);
            Clear();

            calendar.SelectedDates.Add(DateTime.Today.AddDays(100));
            Assert.IsTrue(CompareDates(calendar.SelectedDate.Value, DateTime.Today.AddDays(100)));
            Assert.IsTrue(calendar.SelectedDates.Count == 1);
        }

        #endregion SingleRangeMode

        #region MultipleRangeMode

        [TestMethod]
        public void SelectedDateMultipleRange()
        {
            Calendar calendar = new Calendar();
            calendar.SelectionMode = CalendarSelectionMode.MultipleRange;
            calendar.SelectedDate = DateTime.Today;
            Assert.IsTrue(CompareDates(calendar.SelectedDate.Value, DateTime.Today));
            Assert.IsTrue(calendar.SelectedDates.Count == 1);
            Assert.IsTrue(CompareDates(calendar.SelectedDates[0], DateTime.Today));

            calendar.SelectedDates.Clear();
            Assert.IsNull(calendar.SelectedDate);

            calendar.SelectedDates.AddRange(DateTime.Today, DateTime.Today.AddDays(10));
            Assert.IsTrue(CompareDates(calendar.SelectedDate.Value, DateTime.Today));
            Assert.IsTrue(calendar.SelectedDates.Count == 11);

            calendar.SelectedDates.Add(DateTime.Today);
            Assert.IsTrue(calendar.SelectedDates.Count == 11);
        }



        #endregion MultipleRangeMode

        #endregion SelectedDate

        #region SelectionMode

        [TestMethod]
        [Description("Verify SelectionMode property.")]
        [Asynchronous]
        public void SelectionMode()
        {
            Calendar calendar = new Calendar();
            calendar.SelectionMode = CalendarSelectionMode.SingleRange;
            calendar.SelectedDates.AddRange(DateTime.Today, DateTime.Today.AddDays(10));
            Assert.IsTrue(calendar.SelectedDates.Count == 11);
            calendar.Loaded += new RoutedEventHandler(calendar_Loaded);
            TestPanel.Children.Add(calendar);
            EnqueueConditional(IsLoaded);
            EnqueueCallback(() => VerifySelectionMode(calendar));
            EnqueueTestComplete();
        }

        private void VerifySelectionMode(Calendar calendar)
        {
            calendar.SelectionMode = CalendarSelectionMode.MultipleRange;
            Assert.IsTrue(calendar.SelectedDates.Count == 0);
            _isLoaded = false;
        }

        /// <summary>
        /// Ensure ArgumentOutOfRangeException is thrown.
        /// </summary>
        [TestMethod]
        [Description("Ensure ArgumentOutOfRangeException is thrown.")]
        public void SelectionModeOutOfRangeException()
        {
            Calendar calendar = new Calendar();
            Common.AssertExpectedExceptionWithoutMessageControl(new ArgumentOutOfRangeException(), () => { calendar.SelectionMode = (CalendarSelectionMode)7; });
        }

        #endregion SelectionMode

        #endregion DependencyProperties

        #region MouseHandlers

        #region DaySelection

        #region NoSelection

        [TestMethod]
        [Description("Verify selection is not possible is None Mode.")]
        public void SelectDayNoneInvalidOperationException()
        {
            Calendar calendar = new Calendar();
            calendar.SelectionMode = CalendarSelectionMode.None;
            Common.AssertExpectedExceptionWithoutMessageControl(new InvalidOperationException(), () => { calendar.SelectedDate = new DateTime(2000, 2, 2); });
        }

        [TestMethod]
        [Description("Verify selection is not possible is None Mode.")]
        [Asynchronous]
        public void SelectDayNone()
        {
            Calendar calendar = new Calendar();
            calendar.SelectionMode = CalendarSelectionMode.None;
            calendar.Loaded += new RoutedEventHandler(calendar_Loaded);
            TestPanel.Children.Add(calendar);
            EnqueueConditional(IsLoaded);
            EnqueueCallback(() => VerifySelectDayNone(calendar));
            EnqueueTestComplete();
        }

        private void VerifySelectDayNone(Calendar calendar)
        {
            // Assumes that the Calendar is Gregorian
            calendar.DisplayDate = new DateTime(2003, 10, 1);
            calendar.TestHook.MonthControl.TestHook.Cell_MouseLeftButtonDown((CalendarDayButton)calendar.TestHook.MonthControl.MonthView.Children[10], null);
            Assert.IsFalse(((CalendarDayButton)calendar.TestHook.MonthControl.MonthView.Children[10]).IsSelected);
            Assert.IsNull(calendar.SelectedDate);
            Assert.IsTrue(calendar.SelectedDates.Count == 0);

            //check if the trailing days work in None Mode
            calendar.TestHook.MonthControl.TestHook.Cell_MouseLeftButtonDown((CalendarDayButton)calendar.TestHook.MonthControl.MonthView.Children[7], null);
            Assert.IsNull(calendar.SelectedDate);
            calendar.TestHook.MonthControl.TestHook.Cell_MouseLeftButtonUp((CalendarDayButton)calendar.TestHook.MonthControl.MonthView.Children[7], null);
            Assert.AreEqual(calendar.DisplayDate, new DateTime(2003, 9, 1));
            Assert.IsTrue(calendar.SelectedDates.Count == 0);

            _isLoaded = false;
        }

        [TestMethod]
        [Description("Check if an exception is thrown if SelectedDates are updated in None Selection Mode")]
        public void AddNone()
        {
            Calendar calendar = new Calendar();
            calendar.SelectionMode = CalendarSelectionMode.None;
            Common.AssertExpectedExceptionWithoutMessageControl(new InvalidOperationException(), () => { calendar.SelectedDates.Add(new DateTime()); });
        }

        #endregion NoSelection

        #region SingleSelection

        [TestMethod]
        [Description("Verify if a day can be selected by mouse click and the state is updated.")]
        [Asynchronous]
        public void SelectDaySingle()
        {
            Calendar calendar = new Calendar();

            calendar.SelectionMode = CalendarSelectionMode.SingleDate;
            calendar.SelectedDatesChanged += new EventHandler<SelectionChangedEventArgs>(calendar_SelectedDatesChanged);

            Assert.IsNull(calendar.SelectedDate);
            Assert.IsTrue(calendar.SelectedDates.Count == 0);

            DateTime d = new DateTime(2000, 2, 2);
            calendar.SelectedDate = d;
            Assert.IsTrue(_addedDays.Count == 1);
            Assert.IsTrue(_removedDays.Count == 0);
            Assert.IsTrue(_eventCount == 1);
            Assert.AreEqual(d, calendar.SelectedDate);
            Assert.IsTrue(calendar.SelectedDates.Count == 1);
            Assert.IsTrue(calendar.SelectedDates.Contains(d));

            calendar.Loaded += new RoutedEventHandler(calendar_Loaded);
            TestPanel.Children.Add(calendar);
            EnqueueConditional(IsLoaded);
            EnqueueCallback(() => VerifySelectDaySingle(calendar));
            EnqueueTestComplete();
        }



        private void VerifySelectDaySingle(Calendar calendar)
        {
            // Assumes that the Calendar is Gregorian
            CalendarDayButton b;

            Clear();
            DateTime d = new DateTime(2003, 10, 1);
            calendar.DisplayDate = d;
            b = calendar.FindDayButtonFromDay(d);
            calendar.TestHook.MonthControl.TestHook.Cell_MouseLeftButtonDown(b, null);

            Assert.IsTrue(_addedDays.Count == 1);
            Assert.IsTrue(_removedDays.Count == 1);
            Assert.IsTrue(_eventCount == 1);
            Assert.IsTrue(CompareDates((DateTime)_addedDays[0], (DateTime)b.DataContext));
            Assert.IsTrue(CompareDates((DateTime)_removedDays[0], new DateTime(2000, 2, 2)));
            Assert.IsTrue(b.IsSelected);
            Assert.AreEqual(b.DataContext, calendar.SelectedDate);
            Assert.IsTrue(calendar.SelectedDates.Count == 1);
            Assert.IsTrue(calendar.SelectedDates.Contains(d));

            //check if the trailing days work in Single Mode
            Clear();
            b = calendar.FindDayButtonFromDay(new DateTime(2003, 9, 30));
            d = (DateTime)b.DataContext;
            calendar.TestHook.MonthControl.TestHook.Cell_MouseLeftButtonDown(b, null);
            //we find the button again since the button changed when clicked-on
            b = calendar.FindDayButtonFromDay(new DateTime(2003, 9, 30));
            Assert.IsTrue(_addedDays.Count == 1);
            Assert.IsTrue(_removedDays.Count == 1);
            Assert.IsTrue(_eventCount == 1);
            Assert.IsTrue(CompareDates((DateTime)_addedDays[0], (DateTime)b.DataContext));
            Assert.IsTrue(CompareDates((DateTime)_removedDays[0], new DateTime(2003, 10, 1)));
            Assert.IsTrue(b.IsSelected);
            Assert.AreEqual(b.DataContext, calendar.SelectedDate);
            Assert.IsTrue(calendar.SelectedDates.Count == 1);
            Assert.IsTrue(calendar.SelectedDates.Contains(d));
            Assert.AreEqual(d, calendar.SelectedDate);
            calendar.TestHook.MonthControl.TestHook.Cell_MouseLeftButtonUp(b, null);
            Assert.AreEqual(calendar.DisplayDate, new DateTime(2003, 9, 1));
            b = calendar.FindDayButtonFromDay(d);
            Assert.IsTrue(b.IsSelected);
            Assert.IsTrue(calendar.SelectedDates.Count == 1);
            Assert.IsTrue(calendar.SelectedDates.Contains(d));

            _isLoaded = false;
        }

        [TestMethod]
        [Description("Check if an exception is thrown if SelectedDates have 2 items in Single SelectionMode.")]
        public void AddSingle()
        {
            Calendar calendar = new Calendar();
            calendar.SelectionMode = CalendarSelectionMode.SingleDate;
            calendar.SelectedDates.Add(DateTime.Today);
            Common.AssertExpectedExceptionWithoutMessageControl(new InvalidOperationException(), () => { calendar.SelectedDates.Add(DateTime.Today.AddDays(1)); });
        }

        #endregion SingleSelection

        #region SingleRangeSelection

        [TestMethod]
        [Asynchronous]
        public void SelectDaySingleRangeMouseEnter()
        {
            Calendar calendar = new Calendar();

            calendar.SelectionMode = CalendarSelectionMode.SingleRange;
            calendar.SelectedDatesChanged += new EventHandler<SelectionChangedEventArgs>(calendar_SelectedDatesChanged);
            calendar.Loaded += new RoutedEventHandler(calendar_Loaded);
            TestPanel.Children.Add(calendar);
            EnqueueConditional(IsLoaded);
            EnqueueCallback(() => VerifySelectDaySingleRangeMouseEnter(calendar));
            EnqueueTestComplete();
        }

        private void VerifySelectDaySingleRangeMouseEnter(Calendar calendar)
        {
            // Assumes that the Calendar is Gregorian
            CalendarDayButton b;
            int start, end;

            //
            //verify if a single day can be selected
            //
            Clear();
            DateTime d = new DateTime(2003, 10, 1);
            calendar.DisplayDate = d;
            b = calendar.FindDayButtonFromDay(d);
            calendar.TestHook.MonthControl.TestHook.Cell_MouseLeftButtonDown(b, null);
            Assert.IsTrue(calendar.SelectedDates.Count == 0);
            Assert.IsNull(calendar.SelectedDate);
            calendar.TestHook.MonthControl.TestHook.Cell_MouseLeftButtonUp(b, null);


            Assert.IsTrue(_addedDays.Count == 1);
            Assert.IsTrue(_removedDays.Count == 0);
            Assert.IsTrue(_eventCount == 1);
            Assert.IsTrue(CompareDates((DateTime)_addedDays[0], (DateTime)b.DataContext));
            Assert.IsTrue(b.IsSelected);
            Assert.AreEqual(b.DataContext, calendar.SelectedDate);
            Assert.IsTrue(calendar.SelectedDates.Count == 1);
            Assert.IsTrue(calendar.SelectedDates.Contains(d));

            calendar.SelectedDates.Clear();

            //
            //verify SingleRange selection with MouseEnter -- One single set
            //
            Clear();
            d = new DateTime(2003, 10, 10);
            b = calendar.FindDayButtonFromDay(d);
            start = b.Index;
            calendar.TestHook.MonthControl.TestHook.Cell_MouseLeftButtonDown(b, null);
            Assert.IsTrue(calendar.SelectedDates.Count == 0);
            Assert.IsNull(calendar.SelectedDate);
            calendar.TestHook.MonthControl.TestHook.Cell_MouseLeave(b, null);
            Assert.IsTrue(calendar.SelectedDates.Count == 0);

            d = new DateTime(2003, 10, 20);
            b = calendar.FindDayButtonFromDay(d);
            end = b.Index;
            calendar.TestHook.MonthControl.TestHook.Cell_MouseEnter(b, null);
            Assert.IsTrue(calendar.SelectedDates.Count == 0);
            Assert.IsNull(calendar.SelectedDate);

            //check if the buttons are highlighted
            for (int i = start; i <= end; i++)
            {
                Assert.IsTrue(((CalendarDayButton)calendar.TestHook.MonthControl.MonthView.Children[i]).IsSelected);
            }

            calendar.TestHook.MonthControl.TestHook.Cell_MouseLeftButtonUp(b, null);

            Assert.IsTrue(_addedDays.Count == 11);
            Assert.IsTrue(_removedDays.Count == 0);
            Assert.IsTrue(_eventCount == 1);
            int j = 0;
            for (int i = start; i <= end; i++)
            {
                Assert.IsTrue(((CalendarDayButton)calendar.TestHook.MonthControl.MonthView.Children[i]).IsSelected);
                Assert.IsTrue(calendar.SelectedDates.Contains((DateTime)((CalendarDayButton)calendar.TestHook.MonthControl.MonthView.Children[i]).DataContext));
                Assert.IsTrue(CompareDates((DateTime)_addedDays[j], (DateTime)((CalendarDayButton)calendar.TestHook.MonthControl.MonthView.Children[i]).DataContext));
                j++;
            }

            Assert.IsTrue(CompareDates(calendar.SelectedDates[0], calendar.SelectedDate.Value));
            Assert.IsTrue(calendar.SelectedDates.Count == 11);


            //
            //verify SingleRange selection with MouseEnter -- two sets
            //
            Clear();
            d = new DateTime(2003, 10, 10);
            b = calendar.FindDayButtonFromDay(d);
            start = b.Index;
            calendar.TestHook.MonthControl.TestHook.Cell_MouseLeftButtonDown(b, null);
            calendar.TestHook.MonthControl.TestHook.Cell_MouseLeave(b, null);


            d = new DateTime(2003, 10, 20);
            b = calendar.FindDayButtonFromDay(d);
            end = b.Index;
            calendar.TestHook.MonthControl.TestHook.Cell_MouseEnter(b, null);

            //check if the buttons are highlighted
            for (int i = start; i <= end; i++)
            {
                Assert.IsTrue(((CalendarDayButton)calendar.TestHook.MonthControl.MonthView.Children[i]).IsSelected);
            }

            //check if the selectedDates are not updated yet
            Assert.IsTrue(calendar.SelectedDates.Count == 11);

            d = new DateTime(2003, 10, 1);
            calendar.TestHook.MonthControl.TestHook.Cell_MouseLeave(b, null);
            b = calendar.FindDayButtonFromDay(d);
            calendar.TestHook.MonthControl.TestHook.Cell_MouseEnter(b, null);
            //check if the first set is unhighlighted
            for (int i = start + 1; i <= end; i++)
            {
                Assert.IsFalse(((CalendarDayButton)calendar.TestHook.MonthControl.MonthView.Children[i]).IsSelected);
            }
            end = b.Index;
            //check if the second set is highlighted
            for (int i = end; i <= start; i++)
            {
                Assert.IsTrue(((CalendarDayButton)calendar.TestHook.MonthControl.MonthView.Children[i]).IsSelected);
            }
            calendar.TestHook.MonthControl.TestHook.Cell_MouseLeftButtonUp(b, null);



            Assert.IsTrue(_addedDays.Count == 10);
            Assert.IsTrue(_removedDays.Count == 11);
            Assert.IsTrue(_eventCount == 1);
            j = 0;
            for (int i = start; i >= end; i--)
            {
                Assert.IsTrue(((CalendarDayButton)calendar.TestHook.MonthControl.MonthView.Children[i]).IsSelected);
                Assert.IsTrue(calendar.SelectedDates.Contains((DateTime)((CalendarDayButton)calendar.TestHook.MonthControl.MonthView.Children[i]).DataContext));
                Assert.IsTrue(CompareDates((DateTime)_addedDays[j], (DateTime)((CalendarDayButton)calendar.TestHook.MonthControl.MonthView.Children[i]).DataContext));
                j++;
            }

            Assert.IsTrue(CompareDates(calendar.SelectedDates[0], calendar.SelectedDate.Value));
            Assert.IsTrue(calendar.SelectedDates.Count == 10);


            ////check if the trailing days work in SingleRange Mode
            Clear();
            start = b.Index;
            calendar.TestHook.MonthControl.TestHook.Cell_MouseLeftButtonDown(b, null);
            calendar.TestHook.MonthControl.TestHook.Cell_MouseLeave(b, null);
            d = new DateTime(2003, 11, 1);
            b = calendar.FindDayButtonFromDay(d);
            end = b.Index;
            calendar.TestHook.MonthControl.TestHook.Cell_MouseEnter(b, null);
            ////check if the buttons are highlighted
            for (int i = start; i <= end; i++)
            {
                Assert.IsTrue(((CalendarDayButton)calendar.TestHook.MonthControl.MonthView.Children[i]).IsSelected);
            }
            calendar.TestHook.MonthControl.TestHook.Cell_MouseLeftButtonUp(b, null);

            Assert.IsTrue(_addedDays.Count == 32);
            Assert.IsTrue(_removedDays.Count == 10);
            Assert.IsTrue(_eventCount == 1);
            Assert.IsTrue(DateTime.Compare(calendar.DisplayDate, d) == 0);
            Assert.IsTrue(calendar.SelectedDates.Count == 32);
            Assert.IsTrue(CompareDates(calendar.SelectedDates[0], calendar.SelectedDate.Value));

            _isLoaded = false;
        }

        [TestMethod]
        [Asynchronous]
        public void SelectDaySingleRangeMouseHover()
        {
            Calendar calendar = new Calendar();
            calendar.SelectionMode = CalendarSelectionMode.SingleRange;
            calendar.SelectedDatesChanged += new EventHandler<SelectionChangedEventArgs>(calendar_SelectedDatesChanged);
            calendar.Loaded += new RoutedEventHandler(calendar_Loaded);
            TestPanel.Children.Add(calendar);
            EnqueueConditional(IsLoaded);
            EnqueueCallback(() => VerifySelectDaySingleRangeMouseHover(calendar));
            EnqueueTestComplete();
        }

        private void VerifySelectDaySingleRangeMouseHover(Calendar calendar)
        {
            //Assumes that the Calendar is Gregorian
            CalendarDayButton b;
            System.Globalization.Calendar cal = new GregorianCalendar();
            int start, end;
            DateTime r1, r2;

            r1 = new DateTime(2003, 10, 10);
            r2 = new DateTime(2003, 11, 1);

            Clear();
            calendar.DisplayDate = r1;
            b = calendar.FindDayButtonFromDay(r1);
            start = b.Index;
            calendar.TestHook.MonthControl.TestHook.Cell_MouseLeftButtonDown(b, null);
            calendar.TestHook.MonthControl.TestHook.Cell_MouseLeave(b, null);

            while (DateTime.Compare(r2, r1) > 0)
            {
                r1 = cal.AddDays(r1, 1);
                b = calendar.FindDayButtonFromDay(r1);
                calendar.TestHook.MonthControl.TestHook.Cell_MouseEnter(b, null);
                calendar.TestHook.MonthControl.TestHook.Cell_MouseLeave(b, null);
            }
            end = b.Index;
            Assert.IsTrue(calendar.SelectedDates.Count == 0);

            //check if the buttons are highlighted
            for (int i = start; i <= end; i++)
            {
                Assert.IsTrue(((CalendarDayButton)calendar.TestHook.MonthControl.MonthView.Children[i]).IsSelected);
            }

            b = calendar.FindDayButtonFromDay(r2);
            calendar.TestHook.MonthControl.TestHook.Cell_MouseLeftButtonUp(b, null);

            Assert.IsTrue(_addedDays.Count == 23);
            Assert.IsTrue(_eventCount == 1);

            Assert.IsTrue(DateTimeHelper.CompareYearMonth(calendar.DisplayDate, r2) == 0);
            Assert.IsTrue(calendar.SelectedDates.Count == 23);
            Assert.IsTrue(CompareDates(calendar.SelectedDates[0], calendar.SelectedDate.Value));

            _isLoaded = false;
        }

        #endregion SingleRangeSelection

        #endregion DaySelection

        #endregion MouseHandlers

        #region KeyboardSelection

        #region NoSelection
        [TestMethod]
        [Description("Verify keyboard selection is not possible is None Mode.")]
        [Asynchronous]
        public void KeyboardNone()
        {
            Calendar calendar = new Calendar();

            calendar.SelectionMode = CalendarSelectionMode.None;
            calendar.Loaded += new RoutedEventHandler(calendar_Loaded);
            TestPanel.Children.Add(calendar);
            EnqueueConditional(IsLoaded);
            EnqueueCallback(() => VerifyKeyboardNone(calendar));
            EnqueueTestComplete();
        }

        private void VerifyKeyboardNone(Calendar calendar)
        {
            calendar.TestHook.ProcessUpKey(false, false);
            Assert.IsNull(calendar.SelectedDate);
            Assert.IsTrue(calendar.SelectedDates.Count == 0);
            _isLoaded = false;
        }
        #endregion NoSelection

        #region SingleSelection

        [TestMethod]
        [Description("Verify if a day can be selected by mouse click and the state is updated.")]
        [Asynchronous]
        public void KeyboardSingle()
        {
            Calendar calendar = new Calendar();
            calendar.SelectionMode = CalendarSelectionMode.SingleDate;
            Assert.IsNull(calendar.SelectedDate);
            Assert.IsTrue(calendar.SelectedDates.Count == 0);
            calendar.Loaded += new RoutedEventHandler(calendar_Loaded);
            TestPanel.Children.Add(calendar);
            EnqueueConditional(IsLoaded);
            EnqueueCallback(() => VerifyKeyboardSingle(calendar));
            EnqueueTestComplete();
        }

        private void VerifyKeyboardSingle(Calendar calendar)
        {
            // Assumes that the Calendar is Gregorian
            DateTime d;

            calendar.TestHook.ProcessDownKey(false, true);
            d = DateTime.Today.AddDays(7);
            VerifySelectedDate(calendar, d);

            calendar.TestHook.ProcessUpKey(false, true);
            d = DateTime.Today;
            VerifySelectedDate(calendar, d);

            calendar.TestHook.ProcessLeftKey(true);
            d = DateTime.Today.AddDays(-1);
            VerifySelectedDate(calendar, d);

            calendar.TestHook.ProcessRightKey(true);
            d = DateTime.Today;
            VerifySelectedDate(calendar, d);

            calendar.TestHook.ProcessPageDownKey(true);
            d = DateTime.Today.AddMonths(1);
            VerifySelectedDate(calendar, d);

            calendar.TestHook.ProcessPageUpKey(true);
            d = d.AddMonths(-1);
            VerifySelectedDate(calendar, d);

            calendar.TestHook.ProcessHomeKey(true);
            d = calendar.DisplayDateInternal;
            VerifySelectedDate(calendar, d);

            calendar.TestHook.ProcessEndKey(true);
            d = d.AddMonths(1);
            d = d.AddDays(-1);
            VerifySelectedDate(calendar, d);

            _isLoaded = false;
        }

        #endregion SingleSelection

        #region SingleRangeSelection

        [TestMethod]
        [Asynchronous]
        public void KeyboardSingleRange()
        {
            Calendar calendar = new Calendar();
            calendar.SelectionMode = CalendarSelectionMode.SingleRange;
            Assert.IsNull(calendar.SelectedDate);
            Assert.IsTrue(calendar.SelectedDates.Count == 0);
            calendar.Loaded += new RoutedEventHandler(calendar_Loaded);
            TestPanel.Children.Add(calendar);
            EnqueueConditional(IsLoaded);
            EnqueueCallback(() => VerifyKeyboardSingleRange(calendar));
            EnqueueTestComplete();
        }

        private void VerifyKeyboardSingleRange(Calendar calendar)
        {
            // Assumes that the Calendar is Gregorian
            DateTime d, end;
            CalendarDayButton b;

            //single selection with keyboard in SingleRange Mode
            calendar.TestHook.ProcessDownKey(false, false);
            d = DateTime.Today.AddDays(7);
            VerifySelectedDate(calendar, d);

            calendar.TestHook.ProcessUpKey(false, false);
            d = DateTime.Today;
            VerifySelectedDate(calendar, d);

            calendar.TestHook.ProcessLeftKey(false);
            d = DateTime.Today.AddDays(-1);
            VerifySelectedDate(calendar, d);

            calendar.TestHook.ProcessRightKey(false);
            d = DateTime.Today;
            VerifySelectedDate(calendar, d);

            calendar.TestHook.ProcessPageDownKey(false);
            d = DateTime.Today.AddMonths(1);
            VerifySelectedDate(calendar, d);

            calendar.TestHook.ProcessPageUpKey(false);
            d = d.AddMonths(-1);
            VerifySelectedDate(calendar, d);

            calendar.TestHook.ProcessHomeKey(false);
            d = calendar.DisplayDateInternal;
            VerifySelectedDate(calendar, d);

            calendar.TestHook.ProcessEndKey(false);
            d = d.AddMonths(1);
            d = d.AddDays(-1);
            VerifySelectedDate(calendar, d);

            calendar.TestHook.ProcessDownKey(false, true);
            calendar.TestHook.ProcessShiftKeyUp();
            end = d.AddDays(7);
            Assert.IsTrue(DateTime.Compare(calendar.SelectedDate.Value, calendar.SelectedDates[0]) == 0);
            Assert.IsTrue(calendar.SelectedDates.Count == 8);
            while (DateTime.Compare(d, end) < 1)
            {
                b = calendar.FindDayButtonFromDay(d);
                Assert.IsTrue(b.IsSelected);
                Assert.IsTrue(calendar.SelectedDates.Contains((DateTime)b.DataContext));
                d = d.AddDays(1);
            }

            calendar.TestHook.ProcessUpKey(false, true);
            calendar.TestHook.ProcessShiftKeyUp();
            d = d.AddDays(-8);
            Assert.IsTrue(DateTime.Compare(calendar.SelectedDate.Value, calendar.SelectedDates[0]) == 0);
            Assert.IsTrue(calendar.SelectedDates.Count == 1);
            b = calendar.FindDayButtonFromDay(d);
            Assert.IsTrue(b.IsSelected);

            calendar.TestHook.ProcessUpKey(false, true);
            calendar.TestHook.ProcessShiftKeyUp();
            end = d.AddDays(-7);
            Assert.IsTrue(DateTime.Compare(calendar.SelectedDate.Value, calendar.SelectedDates[0]) == 0);
            Assert.IsTrue(calendar.SelectedDates.Count == 8);
            while (DateTime.Compare(end, d) < 1)
            {
                b = calendar.FindDayButtonFromDay(end);
                Assert.IsTrue(b.IsSelected);
                Assert.IsTrue(calendar.SelectedDates.Contains((DateTime)b.DataContext));
                end = end.AddDays(1);
            }

            calendar.TestHook.ProcessLeftKey(true);
            calendar.TestHook.ProcessShiftKeyUp();
            end = d.AddDays(-8);
            Assert.IsTrue(DateTime.Compare(calendar.SelectedDate.Value, calendar.SelectedDates[0]) == 0);
            Assert.IsTrue(calendar.SelectedDates.Count == 9);
            while (DateTime.Compare(end, d) < 1)
            {
                b = calendar.FindDayButtonFromDay(end);
                Assert.IsTrue(b.IsSelected);
                Assert.IsTrue(calendar.SelectedDates.Contains((DateTime)b.DataContext));
                end = end.AddDays(1);
            }

            calendar.TestHook.ProcessRightKey(true);
            calendar.TestHook.ProcessShiftKeyUp();
            end = d.AddDays(-7);
            Assert.IsTrue(DateTime.Compare(calendar.SelectedDate.Value, calendar.SelectedDates[0]) == 0);
            Assert.IsTrue(calendar.SelectedDates.Count == 8);
            while (DateTime.Compare(end, d) < 1)
            {
                b = calendar.FindDayButtonFromDay(end);
                Assert.IsTrue(b.IsSelected);
                Assert.IsTrue(calendar.SelectedDates.Contains((DateTime)b.DataContext));
                end = end.AddDays(1);
            }

            calendar.TestHook.ProcessPageDownKey(true);
            calendar.TestHook.ProcessShiftKeyUp();
            end = d.AddDays(-7);
            end = end.AddMonths(1);
            Assert.IsTrue(DateTime.Compare(calendar.SelectedDate.Value, calendar.SelectedDates[0]) == 0);
            Assert.IsTrue(calendar.SelectedDates.Count == ((end - d).Days + 1));
            while (DateTime.Compare(d, end) < 1)
            {
                b = calendar.FindDayButtonFromDay(d);
                Assert.IsTrue(b.IsSelected);
                Assert.IsTrue(calendar.SelectedDates.Contains((DateTime)b.DataContext));
                d = d.AddDays(1);
            }

            calendar.TestHook.ProcessPageUpKey(true);
            calendar.TestHook.ProcessShiftKeyUp();
            d = end.AddMonths(-1);
            end = d.AddDays(7);
            Assert.IsTrue(DateTime.Compare(calendar.SelectedDate.Value, end) == 0);
            Assert.IsTrue(calendar.SelectedDates.Count == 8);
            while (DateTime.Compare(d, end) < 1)
            {
                b = calendar.FindDayButtonFromDay(d);
                Assert.IsTrue(b.IsSelected);
                Assert.IsTrue(calendar.SelectedDates.Contains((DateTime)b.DataContext));
                d = d.AddDays(1);
            }

            calendar.TestHook.ProcessHomeKey(true);
            calendar.TestHook.ProcessShiftKeyUp();
            d = calendar.DisplayDateInternal;
            Assert.IsTrue(DateTime.Compare(calendar.SelectedDate.Value, end) == 0);
            Assert.IsTrue(calendar.SelectedDates.Count == ((end - d).Days + 1));
            while (DateTime.Compare(d, end) < 1)
            {
                b = calendar.FindDayButtonFromDay(d);
                Assert.IsTrue(b.IsSelected);
                Assert.IsTrue(calendar.SelectedDates.Contains((DateTime)b.DataContext));
                d = d.AddDays(1);
            }

            calendar.TestHook.ProcessEndKey(true);
            calendar.TestHook.ProcessShiftKeyUp();
            d = end;
            end = calendar.DisplayDateInternal.AddMonths(1);
            end = end.AddDays(-1);
            Assert.IsTrue(DateTime.Compare(calendar.SelectedDate.Value, end) == 0);
            Assert.IsTrue(calendar.SelectedDates.Count == ((end - d).Days + 1));
            while (DateTime.Compare(d, end) < 1)
            {
                b = calendar.FindDayButtonFromDay(d);
                Assert.IsTrue(b.IsSelected);
                Assert.IsTrue(calendar.SelectedDates.Contains((DateTime)b.DataContext));
                d = d.AddDays(1);
            }

            _isLoaded = false;
        }

        #endregion SingleRangeSelection

        #region MultipleRangeSelection

        [TestMethod]
        [Asynchronous]
        public void KeyboardMultipleRange()
        {
            Calendar calendar = new Calendar();
            calendar.SelectionMode = CalendarSelectionMode.MultipleRange;
            Assert.IsNull(calendar.SelectedDate);
            Assert.IsTrue(calendar.SelectedDates.Count == 0);
            calendar.Loaded += new RoutedEventHandler(calendar_Loaded);
            TestPanel.Children.Add(calendar);
            EnqueueConditional(IsLoaded);
            EnqueueCallback(() => VerifyKeyboardSingleRange(calendar));
            EnqueueTestComplete();
        }

        #endregion MultipleRangeSelection

        #endregion KeyboardSelection

        #region CalendarAutomationPeer Tests

        /// <summary>
        /// Tests the creation of an automation peer for the Calendar
        /// </summary>
        [TestMethod]
        [Asynchronous]
        [Description("Tests the creation of an automation peer for the Calendar")]
        public void Calendar_AutomationPeer()
        {
            Calendar calendar = new Calendar();
            Assert.IsNotNull(calendar);
            calendar.Height = 200;
            calendar.Width = 200;
            _isLoaded = false;
            DateTime date = new DateTime(2000, 2, 2);
            calendar.DisplayDate = date;
            calendar.SelectedDate = date;

            calendar.Loaded += new RoutedEventHandler(calendar_Loaded);
            CalendarAutomationPeer peer = ((CalendarAutomationPeer)CalendarAutomationPeer.CreatePeerForElement(calendar));
            Assert.IsNotNull(peer);

            TestPeer testPeer = new TestPeer(calendar);
            Assert.IsNotNull(testPeer);

            TestPanel.Children.Add(calendar);
            EnqueueConditional(IsLoaded);
            EnqueueCallback(delegate
            {
                Assert.AreEqual(peer.GetAutomationControlType(), AutomationControlType.Calendar, "Incorrect Control type for calendar");
                Assert.AreEqual(peer.GetClassName(), calendar.GetType().Name, "Incorrect ClassName value for Calendar");
                Assert.AreEqual(peer.GetName(), date.ToString(), "Incorrect Name value for CalendarPeer");
                Assert.IsTrue(peer.IsContentElement(), "Incorrect IsContentElement value");
                Assert.IsTrue(peer.IsControlElement(), "Incorrect IsControlElement value");

                #region CalendarAutomationPeer IGridProvider tests:

                IGridProvider calendarGridProvider = ((IGridProvider)peer.GetPattern(PatternInterface.Grid));
                Assert.IsNotNull(calendarGridProvider, "Incorrect calendarGridProvider value");
                Assert.AreEqual(calendar.MonthControl.MonthView.RowDefinitions.Count - 1, calendarGridProvider.RowCount, "Incorrect RowCount value");
                Assert.AreEqual(calendar.MonthControl.MonthView.ColumnDefinitions.Count, calendarGridProvider.ColumnCount, "Incorrect ColumnCount value");

                IRawElementProviderSimple cell = calendarGridProvider.GetItem(0, 3);
                Assert.IsNotNull(cell, "GetItem returned null for valid cell");
                AutomationPeer cellPeer = testPeer.GetPeerFromProvider(cell);
                Assert.AreEqual(typeof(CalendarDayButton).Name, cellPeer.GetClassName(), "GetItem did not return DayButton");

                calendar.DisplayMode = CalendarMode.Year;
                Assert.AreEqual(calendar.MonthControl.YearView.RowDefinitions.Count, calendarGridProvider.RowCount, "Incorrect RowCount value");
                Assert.AreEqual(calendar.MonthControl.YearView.ColumnDefinitions.Count, calendarGridProvider.ColumnCount, "Incorrect ColumnCount value");

                cell = calendarGridProvider.GetItem(2, 3);
                Assert.IsNotNull(cell, "GetItem returned null for valid cell");
                cellPeer = testPeer.GetPeerFromProvider(cell);
                Assert.AreEqual(typeof(CalendarButton).Name, cellPeer.GetClassName(), "GetItem did not return CalendarButton");

                calendar.DisplayMode = CalendarMode.Decade;
                Assert.AreEqual(calendar.MonthControl.YearView.RowDefinitions.Count, calendarGridProvider.RowCount, "Incorrect RowCount value");
                Assert.AreEqual(calendar.MonthControl.YearView.ColumnDefinitions.Count, calendarGridProvider.ColumnCount, "Incorrect ColumnCount value");

                cell = calendarGridProvider.GetItem(2, 3);
                Assert.IsNotNull(cell, "GetItem returned null for valid cell");
                cellPeer = testPeer.GetPeerFromProvider(cell);
                Assert.AreEqual(typeof(CalendarButton).Name, cellPeer.GetClassName(), "GetItem did not return CalendarButton");

                cell = calendarGridProvider.GetItem(10, 10);
                Assert.IsNull(cell, "GetItem returned object for invalid cell");

                #endregion

                #region CalendarAutomationPeer IMultipleViewProvider tests:
                calendar._hasFocus = true;
                IMultipleViewProvider calendarMultiViewProvider = ((IMultipleViewProvider)peer.GetPattern(PatternInterface.MultipleView));
                Assert.IsNotNull(calendarMultiViewProvider);
                Assert.IsTrue(calendarMultiViewProvider.CurrentView == (int)CalendarMode.Decade);
                Assert.IsTrue(CalendarMode.Decade.ToString() == calendarMultiViewProvider.GetViewName(calendarMultiViewProvider.CurrentView));
                calendarMultiViewProvider.SetCurrentView((int)CalendarMode.Year);
                Assert.IsTrue(calendar.DisplayMode == CalendarMode.Year);
                Assert.IsTrue(calendarMultiViewProvider.CurrentView == (int)CalendarMode.Year);
                Assert.IsTrue(CalendarMode.Year.ToString() == calendarMultiViewProvider.GetViewName(calendarMultiViewProvider.CurrentView));

                #endregion

                #region CalendarAutomationPeer ISelectionProvider tests:
                ISelectionProvider calendarSelectionProvider = ((ISelectionProvider)peer.GetPattern(PatternInterface.Selection));
                Assert.IsNotNull(calendarSelectionProvider);
                Assert.IsFalse(calendarSelectionProvider.IsSelectionRequired, "Incorrect IsSelectionRequired value");
                Assert.IsFalse(calendarSelectionProvider.CanSelectMultiple, "Incorrect CanSelectMultiple value");
                calendar.SelectionMode = CalendarSelectionMode.MultipleRange;
                Assert.IsNull(calendar.SelectedDate);
                Assert.IsTrue(calendarSelectionProvider.CanSelectMultiple, "Incorrect CanSelectMultiple value");
                calendar.SelectedDates.AddRange(new DateTime(2000, 2, 10), new DateTime(2000, 3, 30));

                IRawElementProviderSimple[] selection = calendarSelectionProvider.GetSelection();
                Assert.IsNotNull(selection, "GetSelection returned null for valid selection");
                Assert.AreEqual(selection.Length, 1, "GetSelection returned wrong number of selections");
                cellPeer = testPeer.GetPeerFromProvider(selection[0]);
                Assert.AreEqual(cellPeer.GetClassName(), typeof(CalendarButton).Name, "Incorrect name for CalendarButton");

                calendar.DisplayMode = CalendarMode.Month;
                selection = calendarSelectionProvider.GetSelection();
                Assert.IsNotNull(selection, "GetSelection returned null for valid selection");
                Assert.AreEqual(selection.Length, 31, "GetSelection returned wrong number of selections");
                cellPeer = testPeer.GetPeerFromProvider(selection[0]);
                Assert.AreEqual(cellPeer.GetClassName(), typeof(CalendarDayButton).Name, "Incorrect name for DayButton");
                #endregion

                #region CalendarAutomationPeer ITableProvider tests:

                ITableProvider calendarTableProvider = ((ITableProvider)peer.GetPattern(PatternInterface.Table));
                Assert.IsNotNull(calendarTableProvider);
                Assert.AreEqual(calendarTableProvider.RowOrColumnMajor, RowOrColumnMajor.RowMajor, "Incorrect RowOrColumnMajor value");

                IRawElementProviderSimple[] headers = calendarTableProvider.GetRowHeaders();
                Assert.IsNull(headers, "GetRowHeaders should return null");

                headers = calendarTableProvider.GetColumnHeaders();
                Assert.IsNotNull(headers, "GetColumnHeaders returned null");
                Assert.AreEqual(headers.Length, 7, "Incorrect number of column headers");
                #endregion

            });
            EnqueueTestComplete();
        }

        #endregion CalendarAutomationPeer Tests

        #region DayButtonAutomationPeer Tests

        /// <summary>
        /// Tests the creation of an automation peer for the DayButton
        /// </summary>
        [TestMethod]
        [Asynchronous]
        [Description("Tests the creation of an automation peer for the DayButton")]
        public void DayButton_AutomationPeer()
        {
            Calendar calendar = new Calendar();
            Assert.IsNotNull(calendar);
            _isLoaded = false;
            DateTime date = new DateTime(2000, 2, 2);
            calendar.DisplayDate = date;
            calendar.SelectedDate = date;

            CalendarAutomationPeer calendarAutomationPeer = (CalendarAutomationPeer)CalendarAutomationPeer.CreatePeerForElement(calendar);
            Assert.IsNotNull(calendarAutomationPeer);
            TestPeer testPeer = new TestPeer(calendar);

            calendar.Loaded += new RoutedEventHandler(calendar_Loaded);
            TestPanel.Children.Add(calendar);
            EnqueueConditional(IsLoaded);

            EnqueueCallback(delegate
            {
                CalendarDayButton dayButton = calendar.FindDayButtonFromDay(date);
                Assert.IsNotNull(dayButton);
                AutomationPeer peer = CalendarAutomationPeer.CreatePeerForElement(dayButton);
                Assert.IsNotNull(peer);

                Assert.AreEqual(peer.GetAutomationControlType(), AutomationControlType.Button, "Incorrect Control type for Daybutton");
                Assert.AreEqual(peer.GetClassName(), dayButton.GetType().Name, "Incorrect ClassName value for DayButton");
                Assert.AreEqual(peer.GetName(), dayButton.Content.ToString(), "Incorrect Name value for DayButtonPeer");
                Assert.IsTrue(peer.IsContentElement(), "Incorrect IsContentElement value");
                Assert.IsTrue(peer.IsControlElement(), "Incorrect IsControlElement value");
                Assert.IsFalse(peer.IsKeyboardFocusable(), "Incorrect IsKeyBoardFocusable value");

                #region DayButtonAutomationPeer ISelectionItemProvider tests:

                ISelectionItemProvider selectionItem = (ISelectionItemProvider)peer.GetPattern(PatternInterface.SelectionItem);
                Assert.IsNotNull(selectionItem);
                Assert.IsTrue(selectionItem.IsSelected);
                Assert.AreEqual(calendarAutomationPeer, testPeer.GetPeerFromProvider(selectionItem.SelectionContainer));
                selectionItem.RemoveFromSelection();
                Assert.IsFalse(selectionItem.IsSelected);
                selectionItem.AddToSelection();
                Assert.IsTrue(selectionItem.IsSelected);


                //check selection in SingleDate mode
                CalendarDayButton dayButton2 = calendar.FindDayButtonFromDay(date.AddDays(1));
                Assert.IsNotNull(dayButton2);
                AutomationPeer peer2 = CalendarAutomationPeer.CreatePeerForElement(dayButton2);
                Assert.IsNotNull(peer2);
                ISelectionItemProvider selectionItem2 = (ISelectionItemProvider)peer2.GetPattern(PatternInterface.SelectionItem);
                Assert.IsNotNull(selectionItem2);
                Assert.IsFalse(selectionItem2.IsSelected);
                selectionItem2.AddToSelection();
                Assert.IsTrue(selectionItem2.IsSelected);
                Assert.IsFalse(selectionItem.IsSelected);

                //check blackout day
                selectionItem.RemoveFromSelection();
                calendar.BlackoutDates.Add(new CalendarDateRange(date));
                selectionItem.AddToSelection();
                Assert.IsFalse(selectionItem.IsSelected);

                //check selection in None mode
                calendar.SelectionMode = CalendarSelectionMode.None;
                Assert.IsFalse(selectionItem2.IsSelected);
                selectionItem2.AddToSelection();
                Assert.IsFalse(selectionItem2.IsSelected);

                //check selection in MultiRange mode
                calendar.BlackoutDates.Clear();
                calendar.SelectionMode = CalendarSelectionMode.MultipleRange;
                Assert.IsFalse(selectionItem.IsSelected);
                Assert.IsFalse(selectionItem2.IsSelected);
                selectionItem.AddToSelection();
                selectionItem2.AddToSelection();
                Assert.IsTrue(selectionItem.IsSelected);
                Assert.IsTrue(selectionItem2.IsSelected);
                selectionItem2.RemoveFromSelection();
                Assert.IsTrue(selectionItem.IsSelected);
                Assert.IsFalse(selectionItem2.IsSelected);
                #endregion

                #region DayButtonAutomationPeer IInvoke tests:

                //check selection and trailing day functionality
                CalendarDayButton dayButton4 = calendar.FindDayButtonFromDay(new DateTime(2000,1,31));
                Assert.IsNotNull(dayButton4);
                AutomationPeer peer4 = CalendarAutomationPeer.CreatePeerForElement(dayButton4);
                Assert.IsNotNull(peer4);
                IInvokeProvider invokeItem = (IInvokeProvider)peer4.GetPattern(PatternInterface.Invoke);
                Assert.IsNotNull(invokeItem);
                invokeItem.Invoke();
                dayButton4 = calendar.FindDayButtonFromDay(new DateTime(2000, 1, 31));
                Assert.IsTrue(dayButton4.IsSelected);
                Assert.AreEqual(calendar.DisplayDate.Month, 1);

                #endregion

                #region DayButtonAutomationPeer ITableItemProvider tests:

                ITableItemProvider tableItem = (ITableItemProvider)peer.GetPattern(PatternInterface.TableItem);
                Assert.IsNotNull(tableItem);

                IRawElementProviderSimple[] headers = tableItem.GetColumnHeaderItems();
                Assert.AreEqual(1, headers.Length);
                Assert.Equals((((ITableProvider)calendarAutomationPeer).GetColumnHeaders())[3], headers[0]);
                Assert.IsNull(tableItem.GetRowHeaderItems());

                #endregion

                #region DayButtonAutomationPeer IGridItemProvider tests:

                foreach (UIElement child in calendar.MonthControl.MonthView.Children)
                {
                    int childRow = (int)child.GetValue(Grid.RowProperty);
                    IGridItemProvider gridItem;

                    if (childRow != 0)
                    {
                        peer = CalendarDayButtonAutomationPeer.CreatePeerForElement(child);
                        Assert.IsNotNull(peer);
                        gridItem = (IGridItemProvider)peer.GetPattern(PatternInterface.GridItem);
                        Assert.IsNotNull(gridItem);

                        Assert.AreEqual(child.GetValue(Grid.ColumnProperty), gridItem.Column);
                        Assert.AreEqual((int)child.GetValue(Grid.RowProperty) - 1, gridItem.Row);
                        Assert.AreEqual(1, gridItem.ColumnSpan);
                        Assert.AreEqual(1, gridItem.RowSpan);
                        Assert.AreEqual(calendarAutomationPeer, testPeer.GetPeerFromProvider(gridItem.ContainingGrid));
                    }
                }

                #endregion
            });

            EnqueueTestComplete();
        }

        #endregion DayButtonAutomationPeer Tests

        #region CalendarButtonAutomationPeer Tests

        /// <summary>
        /// Tests the creation of an automation peer for the DayButton
        /// </summary>
        [TestMethod]
        [Asynchronous]
        [Description("Tests the creation of an automation peer for the CalendarButton")]
        public void CalendarButton_AutomationPeer()
        {
            Calendar calendar = new Calendar();
            Assert.IsNotNull(calendar);
            _isLoaded = false;
            DateTime date = new DateTime(2000, 2, 2);
            calendar.DisplayDate = date;
            calendar.SelectedDate = date;

            CalendarAutomationPeer calendarAutomationPeer = (CalendarAutomationPeer)CalendarAutomationPeer.CreatePeerForElement(calendar);
            Assert.IsNotNull(calendarAutomationPeer);
            TestPeer testPeer = new TestPeer(calendar);

            calendar.Loaded += new RoutedEventHandler(calendar_Loaded);
            TestPanel.Children.Add(calendar);
            EnqueueConditional(IsLoaded);

            EnqueueCallback(delegate
            {
                calendar._hasFocus = true;
                calendar.DisplayMode = CalendarMode.Year;
                CalendarButton calendarButton = (CalendarButton)calendar.MonthControl.YearView.Children[1];
                Assert.IsNotNull(calendarButton);
                AutomationPeer peer = CalendarAutomationPeer.CreatePeerForElement(calendarButton);
                Assert.IsNotNull(peer);
               
                date = new DateTime(2000, 2, 1);

                Assert.AreEqual(peer.GetAutomationControlType(), AutomationControlType.Button, "Incorrect Control type for CalendarButton");
                Assert.AreEqual(peer.GetClassName(), calendarButton.GetType().Name, "Incorrect ClassName value for CalendarButton");
                Assert.AreEqual(peer.GetName(), calendarButton.Content.ToString(), "Incorrect Name value for CalendarButton");
                Assert.IsTrue(peer.IsContentElement(), "Incorrect IsContentElement value");
                Assert.IsTrue(peer.IsControlElement(), "Incorrect IsControlElement value");
                Assert.IsFalse(peer.IsKeyboardFocusable(), "Incorrect IsKeyBoardFocusable value");

                #region CalendarButtonAutomationPeer ISelectionItemProvider tests:

                ISelectionItemProvider selectionItem = (ISelectionItemProvider)peer.GetPattern(PatternInterface.SelectionItem);
                Assert.IsNotNull(selectionItem);
                Assert.IsTrue(calendarButton.IsFocusedOverride);
                Assert.IsTrue(selectionItem.IsSelected);
                Assert.AreEqual(calendarAutomationPeer, testPeer.GetPeerFromProvider(selectionItem.SelectionContainer));
                selectionItem.RemoveFromSelection();
                Assert.IsTrue(selectionItem.IsSelected);

                CalendarButton calendarButton2 = (CalendarButton)calendar.MonthControl.YearView.Children[0];
                Assert.IsNotNull(calendarButton2);
                AutomationPeer peer2 = CalendarAutomationPeer.CreatePeerForElement(calendarButton2);
                Assert.IsNotNull(peer2);
                ISelectionItemProvider selectionItem2 = (ISelectionItemProvider)peer2.GetPattern(PatternInterface.SelectionItem);
                Assert.IsNotNull(selectionItem2);
                selectionItem2.AddToSelection();
                Assert.IsTrue(selectionItem.IsSelected);
                Assert.IsFalse(calendarButton2.IsFocusedOverride);
                Assert.IsFalse(selectionItem2.IsSelected);
                selectionItem2.Select();
                Assert.IsTrue(selectionItem2.IsSelected);
                Assert.IsFalse(selectionItem.IsSelected);
                Assert.IsFalse(calendarButton.IsFocusedOverride);
                Assert.IsTrue(calendarButton2.IsFocusedOverride);

                calendar.DisplayDateEnd = new DateTime(2000, 9, 1);
                CalendarButton calendarButton3 = (CalendarButton)calendar.MonthControl.YearView.Children[10];
                Assert.IsFalse(calendarButton3.IsEnabled);
                AutomationPeer peer3 = CalendarAutomationPeer.CreatePeerForElement(calendarButton3);
                Assert.IsNotNull(peer3);
                ISelectionItemProvider selectionItem3 = (ISelectionItemProvider)peer3.GetPattern(PatternInterface.SelectionItem);
                Assert.IsNotNull(selectionItem3);
                Common.AssertExpectedExceptionWithoutMessageControl(new ElementNotEnabledException(), () => { selectionItem3.Select(); });

                #endregion

                #region CalendarButtonAutomationPeer IGridItemProvider tests:

                foreach (UIElement child in calendar.MonthControl.YearView.Children)
                {
                    int childRow = (int)child.GetValue(Grid.RowProperty);
                    IGridItemProvider gridItem;

                    if (childRow != 0)
                    {
                        peer = CalendarButtonAutomationPeer.CreatePeerForElement(child);
                        Assert.IsNotNull(peer);
                        gridItem = (IGridItemProvider)peer.GetPattern(PatternInterface.GridItem);
                        Assert.IsNotNull(gridItem);

                        Assert.AreEqual(child.GetValue(Grid.ColumnProperty), gridItem.Column);
                        Assert.AreEqual(child.GetValue(Grid.RowProperty), gridItem.Row);
                        Assert.AreEqual(1, gridItem.ColumnSpan);
                        Assert.AreEqual(1, gridItem.RowSpan);
                        Assert.AreEqual(calendarAutomationPeer, testPeer.GetPeerFromProvider(gridItem.ContainingGrid));
                    }
                }

                #endregion

                #region CalendarButtonAutomation Peer IInvokeProvider tests:

                Assert.AreEqual(calendar.DisplayMode, CalendarMode.Year);
                calendarButton = (CalendarButton)calendar.MonthControl.YearView.Children[1];
                peer = CalendarAutomationPeer.CreatePeerForElement(calendarButton);
                IInvokeProvider invokeProvider = (IInvokeProvider)peer.GetPattern(PatternInterface.Invoke);
                invokeProvider.Invoke();
                Assert.AreEqual(calendar.DisplayMode, CalendarMode.Month);

                #endregion
            });

            EnqueueTestComplete();
        }

        #endregion

        IList _addedDays;
        IList _removedDays;
        int _eventCount = 0;

        private void calendar_SelectedDatesChanged(object sender, SelectionChangedEventArgs e)
        {
            _addedDays = e.AddedItems;
            _removedDays = e.RemovedItems;
            _eventCount++;
        }

        private void Clear()
        {
            if (_addedDays != null)
            {
                _addedDays.Clear();
            }

            if (_removedDays != null)
            {
                _removedDays.Clear();
            }
            _eventCount = 0;
        }

        private static bool CompareDates(DateTime d1, DateTime d2)
        {
            return (d1.Year == d2.Year &&
                    d1.Month == d2.Month &&
                    d1.Day == d2.Day);
        }

        bool _isLoaded;

        private bool IsLoaded()
        {
            return _isLoaded;
        }

        private void calendar_Loaded(object sender, RoutedEventArgs e)
        {
            _isLoaded = true;
        }

        private void VerifySelectedDate(Calendar calendar, DateTime d)
        {
            CalendarDayButton b;
            Assert.IsTrue(CompareDates(calendar.SelectedDate.Value, d));
            Assert.IsTrue(calendar.SelectedDates.Contains(d));
            Assert.IsTrue(calendar.SelectedDates.Count == 1);
            b = calendar.FindDayButtonFromDay(d);
            Assert.IsTrue(b.IsSelected);

            for (int i = 7; i < 48; i++)
            {
                if (i != b.Index)
                {
                    Assert.IsFalse(((CalendarDayButton)calendar.TestHook.MonthControl.MonthView.Children[i]).IsSelected);
                }
            }
        }

        private class TestPeer : FrameworkElementAutomationPeer
        {
            public TestPeer(UIElement element)
                : base(element as FrameworkElement)
            {
            }

            public AutomationPeer GetPeerFromProvider(IRawElementProviderSimple provider)
            {
                return PeerFromProvider(provider);
            }
        }
    }
}
