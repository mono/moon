// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Diagnostics;
using System.Windows.Automation.Peers;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using System.Windows.Media;

namespace System.Windows.Controls
{
    /// <summary>
    /// Represents a control that enables a user to select a date by using a visual calendar display. 
    /// </summary>
    [TemplatePart(Name = Calendar.ElementRoot, Type = typeof(Panel))]
    [TemplatePart(Name = Calendar.ElementMonth, Type = typeof(CalendarItem))]
    public partial class Calendar : Control
    {
        #region Constants

        private const string ElementRoot = "Root";
        private const string ElementMonth = "CalendarItem";

        private const int COLS = 7;
        private const int ROWS = 7;
        private const int YEAR_ROWS = 3;
        private const int YEAR_COLS = 4;

        #endregion Constants

        #region Data
        internal CalendarDayButton _focusButton;
        internal bool _hasFocus;
        internal DateTime? _hoverEnd;
        internal int? _hoverEndIndex;
        internal DateTime? _hoverStart;
        internal int? _hoverStartIndex;
        private bool _isShiftPressed;
        internal DateTime? _lastSelectedDate;
        internal Panel _root;
        internal bool _isMouseSelection;
        internal Collection<DateTime> _removedItems;
        private DateTime _selectedMonth;
        private DateTime _selectedYear;


        #endregion Data

        #region Public Events
        /// <summary>
        /// Occurs when a date is selected.
        /// </summary>
        public event EventHandler<SelectionChangedEventArgs> SelectedDatesChanged;

        /// <summary>
        /// Occurs when the DisplayDate property is changed.
        /// </summary>
        public event EventHandler<CalendarDateChangedEventArgs> DisplayDateChanged;

        /// <summary>
        /// Occurs when the DisplayMode property is changed. 
        /// </summary>
        public event EventHandler<CalendarModeChangedEventArgs> DisplayModeChanged;

        #endregion Public Events

        /// <summary>
        /// Initializes a new instance of the Calendar class.
        /// </summary>
        public Calendar()
        {
            this.DisplayDate = DateTime.Today;
            this.GotFocus += new RoutedEventHandler(Calendar_GotFocus);
            this.LostFocus += new RoutedEventHandler(Calendar_LostFocus);
            this.IsEnabledChanged += new DependencyPropertyChangedEventHandler(OnIsEnabledChanged);
            this.FirstDayOfWeek = DateTimeHelper.GetCurrentDateFormat().FirstDayOfWeek;
            this.IsTodayHighlighted = true;
            this.MouseLeftButtonUp += new MouseButtonEventHandler(Calendar_MouseLeftButtonUp);
            this.BlackoutDates = new CalendarBlackoutDatesCollection(this);
            this.SelectedDates = new SelectedDatesCollection(this);
            this._removedItems = new Collection<DateTime>();
            DefaultStyleKey = typeof(Calendar);
        }

        #region Public Properties

        #region BlackoutDates

        /// <summary>
        /// Gets or sets the dates that are not selectable.
        /// </summary>
        public CalendarBlackoutDatesCollection BlackoutDates
        {
            get;
            private set;
        }

        #endregion BlackoutDates

        #region CalendarButtonStyle

        /// <summary>
        /// Gets or sets the style for displaying a CalendarButton.
        /// </summary>
        public Style CalendarButtonStyle
        {
            get { return (Style)GetValue(CalendarButtonStyleProperty); }
            set { SetValue(CalendarButtonStyleProperty, value); }
        }

        /// <summary>
        /// Identifies the CalendarButtonStyle dependency property.
        /// </summary>
        public static readonly DependencyProperty CalendarButtonStyleProperty =
            DependencyProperty.Register(
            "CalendarButtonStyle",
            typeof(Style),
            typeof(Calendar),
            new PropertyMetadata(OnCalendarButtonStyleChanged));

        private static void OnCalendarButtonStyleChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            Style newStyle = e.NewValue as Style;
            Style oldStyle = e.OldValue as Style;
            Calendar c = d as Calendar;

            if (newStyle != null && c != null)
            {
                CalendarItem monthControl = c.MonthControl;

                if (monthControl != null && monthControl.YearView != null)
                {
                    foreach (UIElement child in monthControl.YearView.Children)
                    {
                        CalendarButton calendarButton = child as CalendarButton;

                        if (calendarButton != null)
                        {
                            EnsureCalendarButtonStyle(calendarButton, oldStyle, newStyle);
                        }
                    }
                }
            }
        }

        #endregion CalendarButtonStyle

        #region CalendarDayButtonStyle

        /// <summary>
        /// Gets or sets the style for displaying a day.
        /// </summary>
        public Style CalendarDayButtonStyle
        {
            get { return (Style)GetValue(CalendarDayButtonStyleProperty); }
            set { SetValue(CalendarDayButtonStyleProperty, value); }
        }


        /// <summary>
        /// Identifies the DayButtonStyle dependency property.
        /// </summary>
        public static readonly DependencyProperty CalendarDayButtonStyleProperty =
            DependencyProperty.Register(
            "CalendarDayButtonStyle",
            typeof(Style),
            typeof(Calendar),
            new PropertyMetadata(OnCalendarDayButtonStyleChanged));

        private static void OnCalendarDayButtonStyleChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            Style newStyle = e.NewValue as Style;
            Style oldStyle = e.OldValue as Style;
            Calendar c = d as Calendar;

            if (newStyle != null && c != null)
            {
                CalendarItem monthControl = c.MonthControl;

                if (monthControl != null && monthControl.MonthView != null)
                {
                    foreach (UIElement child in monthControl.MonthView.Children)
                    {
                        CalendarDayButton dayButton = child as CalendarDayButton;

                        if (dayButton != null)
                        {
                            EnsureDayButtonStyle(dayButton, oldStyle, newStyle);
                        }
                    }
                }
            }
        }

        #endregion CalendarDayButtonStyle

        #region CalendarItemStyle

        /// <summary>
        /// Gets or sets the style for a Month.
        /// </summary>
        public Style CalendarItemStyle
        {
            get { return (Style)GetValue(CalendarItemStyleProperty); }
            set { SetValue(CalendarItemStyleProperty, value); }
        }

        /// <summary>
        /// Identifies the MonthStyle dependency property.
        /// </summary>
        public static readonly DependencyProperty CalendarItemStyleProperty =
            DependencyProperty.Register(
            "CalendarItemStyle",
            typeof(Style),
            typeof(Calendar),
            new PropertyMetadata(OnCalendarItemStyleChanged));

        private static void OnCalendarItemStyleChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            Style newStyle = e.NewValue as Style;
            Style oldStyle = e.OldValue as Style;
            Calendar c = d as Calendar;

            if (newStyle != null && c != null)
            {
                CalendarItem monthControl = c.MonthControl;

                if (monthControl != null)
                {
                    EnsureMonthStyle(monthControl, oldStyle, newStyle);
                }
            }
        }

        #endregion CalendarItemStyle

        #region DisplayDate

        /// <summary>
        /// Gets or sets the date to display.
        /// </summary>
        /// 
        [TypeConverter(typeof(DateTimeTypeConverter))]
        public DateTime DisplayDate
        {
            get { return (DateTime)GetValue(DisplayDateProperty); }
            set { SetValue(DisplayDateProperty, value); }
        }

        /// <summary>
        /// Identifies the DisplayDate dependency property.
        /// </summary>
        public static readonly DependencyProperty DisplayDateProperty =
            DependencyProperty.Register(
            "DisplayDate",
            typeof(DateTime),
            typeof(Calendar),
            new PropertyMetadata(OnDisplayDateChanged));

        /// <summary>
        /// DisplayDateProperty property changed handler.
        /// </summary>
        /// <param name="d">Calendar that changed its DisplayDate.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnDisplayDateChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            Calendar c = d as Calendar;
            Debug.Assert(c != null);
            DateTime removedDate, addedDate;

            addedDate = (DateTime)e.NewValue;
            removedDate = (DateTime)e.OldValue;

            //If DisplayDate < DisplayDateStart, DisplayDate = DisplayDateStart
            if (DateTime.Compare(addedDate, c.DisplayDateRangeStart) < 0)
            {
                c.DisplayDate = c.DisplayDateRangeStart;
                return;
            }

            //If DisplayDate > DisplayDateEnd, DisplayDate = DisplayDateEnd
            if (DateTime.Compare(addedDate, c.DisplayDateRangeEnd) > 0)
            {
                c.DisplayDate = c.DisplayDateRangeEnd;
                return;
            }

            c.DisplayDateInternal = DateTimeHelper.DiscardDayTime(addedDate);
            c.UpdateMonths();
            c.OnDisplayDate(new CalendarDateChangedEventArgs(removedDate, addedDate));
        }

        #endregion DisplayDate

        #region DisplayDateEnd

        /// <summary>
        /// Gets or sets the last date to be displayed.
        /// </summary>
        /// 
        [TypeConverter(typeof(DateTimeTypeConverter))]
        public DateTime? DisplayDateEnd
        {
            get { return (DateTime?)GetValue(DisplayDateEndProperty); }
            set { SetValue(DisplayDateEndProperty, value); }
        }

        /// <summary>
        /// Identifies the DisplayDateEnd dependency property.
        /// </summary>
        public static readonly DependencyProperty DisplayDateEndProperty =
            DependencyProperty.Register(
            "DisplayDateEnd",
            typeof(DateTime?),
            typeof(Calendar),
            new PropertyMetadata(OnDisplayDateEndChanged));

        /// <summary>
        /// DisplayDateEndProperty property changed handler.
        /// </summary>
        /// <param name="d">Calendar that changed its DisplayDateEnd.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnDisplayDateEndChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            Calendar c = d as Calendar;
            Debug.Assert(c != null);

            if (!c.IsHandlerSuspended(Calendar.DisplayDateEndProperty))
            {
                DateTime? newValue = e.NewValue as DateTime?;

                if (newValue.HasValue)
                {
                    //DisplayDateEnd coerces to the value of the SelectedDateMax if SelectedDateMax > DisplayDateEnd
                    DateTime? selectedDateMax = SelectedDateMax(c);

                    if (selectedDateMax.HasValue && DateTime.Compare(selectedDateMax.Value, newValue.Value) > 0)
                    {
                        c.DisplayDateEnd = selectedDateMax.Value;
                        return;
                    }

                    // if DisplayDateEnd < DisplayDateStart, DisplayDateEnd = DisplayDateStart
                    if (DateTime.Compare(newValue.Value, c.DisplayDateRangeStart) < 0)
                    {
                        c.DisplayDateEnd = c.DisplayDateStart;
                        return;
                    }

                    //If DisplayDate > DisplayDateEnd, DisplayDate = DisplayDateEnd
                    if (DateTimeHelper.CompareYearMonth(newValue.Value, c.DisplayDateInternal) < 0)
                    {
                        c.DisplayDate = newValue.Value;
                    }
                }
                c.UpdateMonths();
            }
        }

        #endregion DisplayDateEnd

        #region DisplayDateStart

        /// <summary>
        /// Gets or sets the first date to be displayed.
        /// </summary>
        /// 
        [TypeConverter(typeof(DateTimeTypeConverter))]
        public DateTime? DisplayDateStart
        {
            get { return (DateTime?)GetValue(DisplayDateStartProperty); }
            set { SetValue(DisplayDateStartProperty, value); }
        }

        /// <summary>
        /// Identifies the DisplayDateStart dependency property.
        /// </summary>
        public static readonly DependencyProperty DisplayDateStartProperty =
            DependencyProperty.Register(
            "DisplayDateStart",
            typeof(DateTime?),
            typeof(Calendar),
            new PropertyMetadata(OnDisplayDateStartChanged));

        /// <summary>
        /// DisplayDateStartProperty property changed handler.
        /// </summary>
        /// <param name="d">Calendar that changed its DisplayDateStart.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnDisplayDateStartChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            Calendar c = d as Calendar;
            Debug.Assert(c != null);

            if (!c.IsHandlerSuspended(Calendar.DisplayDateStartProperty))
            {
                DateTime? newValue = e.NewValue as DateTime?;

                if (newValue.HasValue)
                {
                    //DisplayDateStart coerces to the value of the SelectedDateMin if SelectedDateMin < DisplayDateStart
                    DateTime? selectedDateMin = SelectedDateMin(c);

                    if (selectedDateMin.HasValue && DateTime.Compare(selectedDateMin.Value, newValue.Value) < 0)
                    {
                        c.DisplayDateStart = selectedDateMin.Value;
                        return;
                    }

                    // if DisplayDateStart > DisplayDateEnd, DisplayDateEnd = DisplayDateStart
                    if (DateTime.Compare(newValue.Value, c.DisplayDateRangeEnd) > 0)
                    {
                        c.DisplayDateEnd = c.DisplayDateStart;
                    }

                    //If DisplayDate < DisplayDateStart, DisplayDate = DisplayDateStart
                    if (DateTimeHelper.CompareYearMonth(newValue.Value, c.DisplayDateInternal) > 0)
                    {
                        c.DisplayDate = newValue.Value;
                    }
                }
                c.UpdateMonths();
            }
        }

        #endregion DisplayDateStart

        #region DisplayMode

        /// <summary>
        /// Gets or sets a value indicating whether the calendar is displayed in months or years.
        /// </summary>
        public CalendarMode DisplayMode
        {
            get { return (CalendarMode)GetValue(DisplayModeProperty); }
            set { SetValue(DisplayModeProperty, value); }
        }

        /// <summary>
        /// Identifies the DisplayMode dependency property.
        /// </summary>
        public static readonly DependencyProperty DisplayModeProperty =
            DependencyProperty.Register(
            "DisplayMode",
            typeof(CalendarMode),
            typeof(Calendar),
            new PropertyMetadata(OnDisplayModePropertyChanged));

        /// <summary>
        /// DisplayModeProperty property changed handler.
        /// </summary>
        /// <param name="d">Calendar that changed its DisplayMode.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnDisplayModePropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            Calendar c = d as Calendar;
            Debug.Assert(c != null);
            CalendarMode mode = (CalendarMode)e.NewValue;
            CalendarMode oldMode = (CalendarMode)e.OldValue;
            CalendarItem monthControl = c.MonthControl;

            if (IsValidDisplayMode(mode))
            {
                if (monthControl != null)
                {
                    switch (oldMode)
                    {
                        case CalendarMode.Month:
                            {
                                c.SelectedYear = c.DisplayDateInternal;
                                c.SelectedMonth = c.DisplayDateInternal;
                                break;
                            }
                        case CalendarMode.Year:
                            {
                                c.DisplayDate = c.SelectedMonth;
                                c.SelectedYear = c.SelectedMonth;
                                break;
                            }
                        case CalendarMode.Decade:
                            {
                                c.DisplayDate = c.SelectedYear;
                                c.SelectedMonth = c.SelectedYear;
                                break;
                            }
                    }
                    
                    switch (mode)
                    {
                        case CalendarMode.Month:
                            {
                                c.OnMonthClick();
                                break;
                            }
                        case CalendarMode.Year:
                        case CalendarMode.Decade:
                            {
                                c.OnHeaderClick();
                                break;
                            }
                    }
                }
                c.OnDisplayModeChanged(new CalendarModeChangedEventArgs((CalendarMode)e.OldValue, mode));
            }
            else
            {
                throw new ArgumentOutOfRangeException("d", Resource.Calendar_OnDisplayModePropertyChanged_InvalidValue);
            }
        }

        #endregion DisplayMode

        #region FirstDayOfWeek

        /// <summary>
        /// Gets or sets the day that is considered the beginning of the week.
        /// </summary>
        public DayOfWeek FirstDayOfWeek
        {
            get { return (DayOfWeek)GetValue(FirstDayOfWeekProperty); }
            set { SetValue(FirstDayOfWeekProperty, value); }
        }

        /// <summary>
        /// Identifies the FirstDayOfWeek dependency property.
        /// </summary>
        public static readonly DependencyProperty FirstDayOfWeekProperty =
            DependencyProperty.Register(
            "FirstDayOfWeek",
            typeof(DayOfWeek),
            typeof(Calendar),
            new PropertyMetadata(OnFirstDayOfWeekChanged));

        /// <summary>
        /// FirstDayOfWeekProperty property changed handler.
        /// </summary>
        /// <param name="d">Calendar that changed its FirstDayOfWeek.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnFirstDayOfWeekChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            Calendar c = d as Calendar;
            Debug.Assert(c != null);

            if (IsValidFirstDayOfWeek(e.NewValue))
            {
                c.UpdateMonths();
            }
            else
            {
                throw new ArgumentOutOfRangeException("d", Resource.Calendar_OnFirstDayOfWeekChanged_InvalidValue);
            }
        }

        #endregion FirstDayOfWeek

        #region IsEnabled

        /// <summary>
        ///  Called when the IsEnabled property changes.
        /// </summary>
        /// <param name="sender">Sender object</param>
        /// <param name="e">Property changed args</param>
        private void OnIsEnabledChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            Debug.Assert(e.NewValue is bool);
            bool isEnabled = (bool)e.NewValue;

            if (MonthControl != null)
            {
                MonthControl.UpdateDisabledGrid(isEnabled);
            }
        }

        #endregion IsEnabled

        #region IsTodayHighlighted

        /// <summary>
        /// Gets or sets a value indicating whether the current date is highlighted.
        /// </summary>
        public bool IsTodayHighlighted
        {
            get { return (bool)GetValue(IsTodayHighlightedProperty); }
            set { SetValue(IsTodayHighlightedProperty, value); }
        }

        /// <summary>
        /// Identifies the IsTodayHighlighted dependency property.
        /// </summary>
        public static readonly DependencyProperty IsTodayHighlightedProperty =
            DependencyProperty.Register(
            "IsTodayHighlighted",
            typeof(bool),
            typeof(Calendar),
            new PropertyMetadata(OnIsTodayHighlightedChanged));

        /// <summary>
        /// IsTodayHighlightedProperty property changed handler.
        /// </summary>
        /// <param name="d">Calendar that changed its IsTodayHighlighted.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnIsTodayHighlightedChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            Calendar c = d as Calendar;
            Debug.Assert(c != null);

            if (c.DisplayDate != null)
            {
                int i = DateTimeHelper.CompareYearMonth(c.DisplayDateInternal, DateTime.Today);

                if (i > -2 && i < 2)
                {
                    c.UpdateMonths();
                }
            }
        }

        #endregion IsTodayHighlighted

        #region SelectedDate

        /// <summary>
        /// Gets or sets the currently selected date.
        /// </summary>
        /// 
        [TypeConverter(typeof(DateTimeTypeConverter))]
        public DateTime? SelectedDate
        {
            get { return (DateTime?)GetValue(SelectedDateProperty); }
            set { SetValue(SelectedDateProperty, value); }
        }

        /// <summary>
        /// Identifies the SelectedDate dependency property.
        /// </summary>
        public static readonly DependencyProperty SelectedDateProperty =
            DependencyProperty.Register(
            "SelectedDate",
            typeof(DateTime?),
            typeof(Calendar),
            new PropertyMetadata(OnSelectedDateChanged));

        /// <summary>
        /// SelectedDateProperty property changed handler.
        /// </summary>
        /// <param name="d">Calendar that changed its SelectedDate.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnSelectedDateChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            Calendar c = d as Calendar;
            Debug.Assert(c != null);

            if (!c.IsHandlerSuspended(Calendar.SelectedDateProperty))
            {
                if (c.SelectionMode != CalendarSelectionMode.None)
                {
                    DateTime? addedDate;

                    addedDate = (DateTime?)e.NewValue;

                    if (IsValidDateSelection(c, addedDate))
                    {
                        if (addedDate == null)
                        {
                            c.SelectedDates.Clear();
                        }
                        else
                        {
                            if (addedDate.HasValue && !(c.SelectedDates.Count > 0 && c.SelectedDates[0] == addedDate.Value))
                            {
                                foreach (DateTime item in c.SelectedDates)
                                {
                                    c._removedItems.Add(item);
                                }
                                c.SelectedDates.ClearInternal();
                                //the value is added as a range so that the SelectedDatesChanged event can be thrown with all the removed items
                                c.SelectedDates.AddRange(addedDate.Value, addedDate.Value);
                            }
                        }

                        //We update the LastSelectedDate for only the Single mode.For the other modes it automatically gets updated
                        //when the HoverEnd is updated.
                        if (c.SelectionMode == CalendarSelectionMode.SingleDate)
                        {
                            c.LastSelectedDate = addedDate;
                        }
                    }
                    else
                    {
                        throw new ArgumentOutOfRangeException("d", Resource.Calendar_OnSelectedDateChanged_InvalidValue);
                    }
                }
                else
                {
                    throw new InvalidOperationException(Resource.Calendar_OnSelectedDateChanged_InvalidOperation);
                }
            }
        }

        #endregion SelectedDate

        #region SelectedDates

        /// <summary>
        /// Gets the dates that are currently selected.
        /// </summary>
        public SelectedDatesCollection SelectedDates
        {
            get;
            private set;
        }

        #endregion SelectedDates

        #region SelectionMode

        /// <summary>
        /// Gets or sets the selection mode for the calendar.
        /// </summary>
        public CalendarSelectionMode SelectionMode
        {
            get { return (CalendarSelectionMode)GetValue(SelectionModeProperty); }
            set { SetValue(SelectionModeProperty, value); }
        }

        /// <summary>
        /// Identifies the SelectionMode dependency property.
        /// </summary>
        public static readonly DependencyProperty SelectionModeProperty =
            DependencyProperty.Register(
            "SelectionMode",
            typeof(CalendarSelectionMode),
            typeof(Calendar),
            new PropertyMetadata(OnSelectionModeChanged));

        private static void OnSelectionModeChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            Calendar c = d as Calendar;
            Debug.Assert(c != null);

            if (IsValidSelectionMode(e.NewValue))
            {
                c.SetValueNoCallback(Calendar.SelectedDateProperty, null);
                c.SelectedDates.Clear();
            }
            else
            {
                throw new ArgumentOutOfRangeException("d", Resource.Calendar_OnSelectionModeChanged_InvalidValue);
            }
        }

        #endregion SelectionMode

        #endregion Public Properties

        #region Protected Properties
        #endregion Protected Properties

        #region Internal Events

        internal event MouseButtonEventHandler DayButtonMouseUp;

        #endregion Internal Events

        #region Internal Properties

        /// <summary>
        /// This flag is used to determine whether DatePicker should change its 
        /// DisplayDate because of a SelectedDate change on its Calendar
        /// </summary>
        internal bool DatePickerDisplayDateFlag
        {
            get;
            set;
        }

        internal DateTime DisplayDateInternal
        {
            get;
            private set;
        }

        internal DateTime DisplayDateRangeEnd
        {
            get
            {
                return this.DisplayDateEnd.GetValueOrDefault(DateTime.MaxValue);
            }
        }

        internal DateTime DisplayDateRangeStart
        {
            get
            {
                return this.DisplayDateStart.GetValueOrDefault(DateTime.MinValue);
            }
        }

        internal CalendarButton FocusCalendarButton
        {
            get;
            set;
        }

        internal DateTime? HoverEnd
        {
            get { return this._hoverEnd; }
            set
            {
                this._hoverEnd = value;
                this.LastSelectedDate = value;
            }
        }

        internal DateTime? LastSelectedDate
        {
            get { return _lastSelectedDate; }
            set
            {
                _lastSelectedDate = value;

                if (this.SelectionMode == CalendarSelectionMode.None)
                {
                    if (this._focusButton != null)
                    {
                        this._focusButton.IsCurrent = false;
                    }
                    this._focusButton = FindDayButtonFromDay(LastSelectedDate.Value);
                    if (this._focusButton != null)
                    {
                        this._focusButton.IsCurrent = this._hasFocus;
                    }
                }

            }
        }

        internal CalendarItem MonthControl
        {
            get
            {
                if (this._root != null && this._root.Children.Count > 0)
                {
                    return this._root.Children[0] as CalendarItem;
                }
                return null;
            }
        }

        internal DateTime SelectedMonth
        {
            get
            {
                return this._selectedMonth;
            }
            set
            {
                int monthDifferenceStart = DateTimeHelper.CompareYearMonth(value, this.DisplayDateRangeStart);
                int monthDifferenceEnd = DateTimeHelper.CompareYearMonth(value, this.DisplayDateRangeEnd);

                if (monthDifferenceStart >= 0 && monthDifferenceEnd <= 0)
                {
                    this._selectedMonth = DateTimeHelper.DiscardDayTime(value);
                }
                else
                {
                    if (monthDifferenceStart < 0)
                    {
                        this._selectedMonth = DateTimeHelper.DiscardDayTime(this.DisplayDateRangeStart);
                    }
                    else
                    {
                        Debug.Assert(monthDifferenceEnd > 0);
                        this._selectedMonth = DateTimeHelper.DiscardDayTime(this.DisplayDateRangeEnd);
                    }
                }

            }
        }

        internal DateTime SelectedYear
        {
            get
            {
                return this._selectedYear;
            }
            set
            {
                if (value.Year < this.DisplayDateRangeStart.Year)
                {
                    this._selectedYear = this.DisplayDateRangeStart;
                }
                else
                {
                    if (value.Year > this.DisplayDateRangeEnd.Year)
                    {
                        this._selectedYear = this.DisplayDateRangeEnd;
                    }
                    else
                    {
                        this._selectedYear = value;
                    }
                }
            }
        }

        #endregion Internal Properties

        #region Private Properties

        #endregion Private Properties

        #region Public Methods

        /// <summary>
        /// Invoked whenever application code or an internal process, 
        /// such as a rebuilding layout pass, calls the ApplyTemplate method.
        /// </summary>
        public override void OnApplyTemplate()
        {
            base.OnApplyTemplate();

            _root = GetTemplateChild(ElementRoot) as Panel;

            this.SelectedMonth = this.DisplayDate;
            this.SelectedYear = this.DisplayDate;


            if (_root != null)
            {
                CalendarItem month = GetTemplateChild(ElementMonth) as CalendarItem;

                if (month != null)
                {
                    month.Owner = this;

                    if (this.CalendarItemStyle != null)
                    {
                        month.Style = this.CalendarItemStyle;
                    }
                    // 
                }
            }


            this.SizeChanged += new SizeChangedEventHandler(Calendar_SizeChanged);
            this.KeyDown += new KeyEventHandler(Calendar_KeyDown);
            this.KeyUp += new KeyEventHandler(Calendar_KeyUp);
        }

        /// <summary>
        /// Provides a text representation of the selected date.
        /// </summary>
        /// <returns>A text representation of the selected date, or an empty string if SelectedDate is a null reference.</returns>
        public override string ToString()
        {
            if (this.SelectedDate != null)
            {
                return this.SelectedDate.Value.ToString(DateTimeHelper.GetCurrentDateFormat());
            }
            else
            {
                return string.Empty;
            }
        }

        #endregion Public Methods

        #region Protected Methods

        /// <summary>
        /// Creates the automation peer for this Calendar Control.
        /// </summary>
        /// <returns></returns>
        protected override AutomationPeer OnCreateAutomationPeer()
        {
            return new CalendarAutomationPeer(this);
        }

        #endregion Protected Methods

        #region Internal Methods

        internal CalendarDayButton FindDayButtonFromDay(DateTime day)
        {
            CalendarDayButton b;
            DateTime? d;
            CalendarItem monthControl = this.MonthControl;

            // 
            int count = ROWS * COLS;
            if (monthControl != null)
            {
                if (monthControl.MonthView != null)
                {
                    for (int childIndex = COLS; childIndex < count; childIndex++)
                    {
                        b = monthControl.MonthView.Children[childIndex] as CalendarDayButton;
                        d = b.DataContext as DateTime?;

                        if (d.HasValue)
                        {
                            if (DateTimeHelper.CompareDays(d.Value, day) == 0)
                            {
                                return b;
                            }
                        }
                    }
                }
            }
            return null;
        }

        //This method highlights the days in MultiSelection mode without adding them
        //to the SelectedDates collection.
        internal void HighlightDays()
        {
            if (this.HoverEnd != null && this._hoverStart != null)
            {
                int startIndex, endIndex, i;
                CalendarDayButton b;
                DateTime? d;
                CalendarItem monthControl = this.MonthControl;

                //This assumes a contiguous set of dates:
                if (this._hoverEndIndex != null && this._hoverStartIndex != null)
                {
                    SortHoverIndexes(out startIndex, out endIndex);

                    for (i = startIndex; i <= endIndex; i++)
                    {
                        b = monthControl.MonthView.Children[i] as CalendarDayButton;
                        b.IsSelected = true;
                        d = b.DataContext as DateTime?;

                        if (d.HasValue && DateTimeHelper.CompareDays(this.HoverEnd.Value, d.Value) == 0)
                        {
                            if (this._focusButton != null)
                            {
                                this._focusButton.IsCurrent = false;
                            }
                            b.IsCurrent = this._hasFocus;
                            this._focusButton = b;
                        }
                    }
                }
            }
        }

        internal static bool IsValidDateSelection(Calendar cal, object value)
        {
            if (value == null)
            {
                return true;
            }
            else
            {
                if (cal.BlackoutDates.Contains((DateTime)value))
                {
                    return false;
                }
                else
                {
                    if (DateTime.Compare((DateTime)value, cal.DisplayDateRangeStart) < 0)
                    {
                        cal.SetValueNoCallback(Calendar.DisplayDateStartProperty, value);
                    }
                    else if (DateTime.Compare((DateTime)value, cal.DisplayDateRangeEnd) > 0)
                    {
                        cal.SetValueNoCallback(Calendar.DisplayDateEndProperty, value);
                    }
                    return true;
                }
            }
        }

        internal void OnDayButtonMouseUp(MouseButtonEventArgs e)
        {
            MouseButtonEventHandler handler = this.DayButtonMouseUp;
            if (null != handler)
            {
                handler(this, e);
            }
        }

        // If the day is a trailing day, Update the DisplayDate
        internal void OnDayClick(DateTime selectedDate)
        {
            Debug.Assert(this.DisplayMode == CalendarMode.Month);
            int i = DateTimeHelper.CompareYearMonth(selectedDate, this.DisplayDateInternal);

            if (this.SelectionMode == CalendarSelectionMode.None)
            {
                this.LastSelectedDate = selectedDate;
            }

            if (i > 0)
            {
                OnNextClick();
            }
            else if (i < 0)
            {
                OnPreviousClick();
            }
        }

        internal void OnNextClick()
        {
            if (this.DisplayMode == CalendarMode.Month && this.DisplayDate != null)
            {
                DateTime? d = DateTimeHelper.AddMonths(DateTimeHelper.DiscardDayTime(this.DisplayDate), 1);
                if (d.HasValue)
                {
                    if (!this.LastSelectedDate.HasValue || DateTimeHelper.CompareYearMonth(this.LastSelectedDate.Value, d.Value) != 0)
                    {
                        this.LastSelectedDate = d.Value;
                    }
                    this.DisplayDate = d.Value;
                }
            }
            else
            {
                if (this.DisplayMode == CalendarMode.Year)
                {
                    DateTime? d = DateTimeHelper.AddYears(new DateTime(this.SelectedMonth.Year,1,1), 1);

                    if (d.HasValue)
                    {
                        this.SelectedMonth = d.Value;
                    }
                    else
                    {
                        this.SelectedMonth = DateTimeHelper.DiscardDayTime(this.DisplayDateRangeEnd);
                    }
                }
                else
                {
                    Debug.Assert(this.DisplayMode == CalendarMode.Decade);

                    DateTime? d = DateTimeHelper.AddYears(new DateTime(this.SelectedYear.Year, 1, 1), 10);

                    if (d.HasValue)
                    {
                        int decade = Math.Max(1, DateTimeHelper.DecadeOfDate(d.Value));
                        this.SelectedYear = new DateTime(decade, 1, 1);
                    }
                    else
                    {
                        this.SelectedYear = DateTimeHelper.DiscardDayTime(this.DisplayDateRangeEnd);
                    }
                }
                UpdateMonths();
            }
        }

        internal void OnPreviousClick()
        {
            if (this.DisplayMode == CalendarMode.Month && this.DisplayDate != null)
            {
                DateTime? d = DateTimeHelper.AddMonths(DateTimeHelper.DiscardDayTime(this.DisplayDate), -1);
                if (d.HasValue)
                {
                    if (!this.LastSelectedDate.HasValue || DateTimeHelper.CompareYearMonth(this.LastSelectedDate.Value, d.Value) != 0)
                    {
                        this.LastSelectedDate = d.Value;
                    }
                    this.DisplayDate = d.Value;
                }
            }
            else
            {
                if (this.DisplayMode == CalendarMode.Year)
                {
                    DateTime? d = DateTimeHelper.AddYears(new DateTime(this.SelectedMonth.Year, 1, 1), -1);

                    if (d.HasValue)
                    {
                        this.SelectedMonth = d.Value;
                    }
                    else
                    {
                        this.SelectedMonth = DateTimeHelper.DiscardDayTime(this.DisplayDateRangeStart);
                    }
                }
                else
                {
                    Debug.Assert(this.DisplayMode == CalendarMode.Decade);

                    DateTime? d = DateTimeHelper.AddYears(new DateTime(this.SelectedYear.Year, 1, 1), -10);

                    if (d.HasValue)
                    {
                        int decade = Math.Max(1, DateTimeHelper.DecadeOfDate(d.Value));
                        this.SelectedYear = new DateTime(decade, 1, 1);
                    }
                    else
                    {
                        this.SelectedYear = DateTimeHelper.DiscardDayTime(this.DisplayDateRangeStart);
                    }
                }
                UpdateMonths();
            }
        }

        internal void OnSelectedDatesCollectionChanged(SelectionChangedEventArgs e)
        {
            if (IsSelectionChanged(e))
            {
                EventHandler<SelectionChangedEventArgs> handler = this.SelectedDatesChanged;

                if (null != handler)
                {
                    handler(this, e);
                }

                if (AutomationPeer.ListenerExists(AutomationEvents.SelectionItemPatternOnElementSelected) ||
                    AutomationPeer.ListenerExists(AutomationEvents.SelectionItemPatternOnElementAddedToSelection) ||
                    AutomationPeer.ListenerExists(AutomationEvents.SelectionItemPatternOnElementRemovedFromSelection))
                {
                    CalendarAutomationPeer peer = FrameworkElementAutomationPeer.FromElement(this) as CalendarAutomationPeer;
                    if (peer != null)
                    {
                        peer.RaiseSelectionEvents(e);
                    }
                }
            }
        }

        internal void ResetStates()
        {
            CalendarDayButton d;
            CalendarItem monthControl = this.MonthControl;
            int count = ROWS * COLS;
            if (monthControl != null)
            {
                if (monthControl.MonthView != null)
                {
                    for (int childIndex = COLS; childIndex < count; childIndex++)
                    {
                        d = monthControl.MonthView.Children[childIndex] as CalendarDayButton;
                        d.IsMouseOverOverride = false;
                    }
                }
            }
        }

        //This method unhighlights the days that were hovered over but not added to the 
        //SelectedDates collection or unhighlights the previously selected days in SingleRange Mode
        internal void UnHighlightDays()
        {
            if (this.HoverEnd != null && this._hoverStart != null)
            {
                CalendarItem monthControl = this.MonthControl;
                CalendarDayButton b;
                DateTime? d;

                if (this._hoverEndIndex != null && this._hoverStartIndex != null)
                {
                    int startIndex, endIndex, i;
                    SortHoverIndexes(out startIndex, out endIndex);

                    if (this.SelectionMode == CalendarSelectionMode.MultipleRange)
                    {
                        for (i = startIndex; i <= endIndex; i++)
                        {
                            b = monthControl.MonthView.Children[i] as CalendarDayButton;
                            d = b.DataContext as DateTime?;

                            if (d.HasValue)
                            {
                                if (!this.SelectedDates.Contains(d.Value))
                                {
                                    b.IsSelected = false;
                                }
                            }
                        }
                    }
                    else
                    {
                        //It is SingleRange
                        for (i = startIndex; i <= endIndex; i++)
                        {
                            (monthControl.MonthView.Children[i] as CalendarDayButton).IsSelected = false;
                        }
                    }
                }
            }
        }

        internal void UpdateMonths()
        {
            CalendarItem monthControl = this.MonthControl;
            if (monthControl != null)
            {
                switch (this.DisplayMode)
                {
                    case CalendarMode.Month:
                        {
                            monthControl.UpdateMonthMode();
                            break;
                        }
                    case CalendarMode.Year:
                        {
                            monthControl.UpdateYearMode();
                            break;
                        }
                    case CalendarMode.Decade:
                        {
                            monthControl.UpdateDecadeMode();
                            break;
                        }
                }
            }
        }

        #endregion Internal Methods

        #region Private Methods

        // This method adds the days that were selected by Keyboard to the SelectedDays Collection 
        private void AddSelection()
        {
            if (this.HoverEnd != null && this._hoverStart != null)
            {
                foreach (DateTime item in this.SelectedDates)
                {
                    this._removedItems.Add(item);
                }

                this.SelectedDates.ClearInternal();
                //In keyboard selection, we are sure that the collection does not include any blackout days
                this.SelectedDates.AddRange(this._hoverStart.Value, this.HoverEnd.Value);
            }
        }

        private void Calendar_GotFocus(object sender, RoutedEventArgs e)
        {
            Calendar c = sender as Calendar;
            Debug.Assert(c != null);
            this._hasFocus = true;

            switch (this.DisplayMode)
            {
                case CalendarMode.Month:
                    {
                        DateTime focusDate;
                        if (this.LastSelectedDate.HasValue && DateTimeHelper.CompareYearMonth(this.DisplayDateInternal, this.LastSelectedDate.Value) == 0)
                        {
                            focusDate = this.LastSelectedDate.Value;
                        }
                        else
                        {
                            focusDate = this.DisplayDate;
                            this.LastSelectedDate = this.DisplayDate;
                        }
                        Debug.Assert(focusDate != null);
                        this._focusButton = FindDayButtonFromDay(focusDate);

                        if (this._focusButton != null)
                        {
                            this._focusButton.IsCurrent = true;
                        }
                        break;
                    }
                case CalendarMode.Year:
                case CalendarMode.Decade:
                    {
                        this.FocusCalendarButton.IsFocusedOverride = true;
                        break;
                    }
            }
        }

        private void Calendar_KeyDown(object sender, KeyEventArgs e)
        {
            Calendar c = sender as Calendar;
            Debug.Assert(c != null);

            if (!e.Handled && c.IsEnabled)
            {
                e.Handled = ProcessCalendarKey(e);
            }
        }

        private void Calendar_KeyUp(object sender, KeyEventArgs e)
        {
            if (!e.Handled && e.Key == Key.Shift)
            {
                ProcessShiftKeyUp();
            }
        }

        private void Calendar_LostFocus(object sender, RoutedEventArgs e)
        {
            Calendar c = sender as Calendar;
            Debug.Assert(c != null);
            this._hasFocus = false;

            switch (this.DisplayMode)
            {
                case CalendarMode.Month:
                    {
                        if (this._focusButton != null)
                        {
                            this._focusButton.IsCurrent = false;
                        }
                        break;
                    }
                case CalendarMode.Year:
                case CalendarMode.Decade:
                    {
                        if (this.FocusCalendarButton != null)
                        {
                            this.FocusCalendarButton.IsFocusedOverride = false;
                        }
                        break;
                    }
            }
        }

        private void Calendar_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            if (!this._hasFocus)
            {
                this.Focus();
            }
        }

        private void Calendar_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            Debug.Assert(sender is Calendar);

            RectangleGeometry rg = new RectangleGeometry();
            rg.Rect = new Rect(0, 0, e.NewSize.Width, e.NewSize.Height);

            if (_root != null)
            {
                _root.Clip = rg;
            }
        }

        private static void EnsureCalendarButtonStyle(CalendarButton calendarButton, Style oldCalendarButtonStyle, Style newCalendarButtonStyle)
        {
            Debug.Assert(calendarButton != null);

            if (newCalendarButtonStyle != null)
            {
                // 

                if (calendarButton != null && (calendarButton.Style == null || calendarButton.Style == oldCalendarButtonStyle))
                {
                    calendarButton.Style = newCalendarButtonStyle;
                }
            }
        }

        private static void EnsureDayButtonStyle(CalendarDayButton dayButton, Style oldDayButtonStyle, Style newDayButtonStyle)
        {
            Debug.Assert(dayButton != null);

            if (newDayButtonStyle != null)
            {
                // 

                if (dayButton != null && (dayButton.Style == null || dayButton.Style == oldDayButtonStyle))
                {
                    dayButton.Style = newDayButtonStyle;
                }
            }
        }

        private static void EnsureMonthStyle(CalendarItem month, Style oldMonthStyle, Style newMonthStyle)
        {
            Debug.Assert(month != null);

            if (newMonthStyle != null)
            {
                // 

                if (month != null && (month.Style == null || month.Style == oldMonthStyle))
                {
                    month.Style = newMonthStyle;
                }
            }
        }

        private static bool IsSelectionChanged(SelectionChangedEventArgs e)
        {
            if (e.AddedItems.Count != e.RemovedItems.Count)
            {
                return true;
            }
            foreach (DateTime addedDate in e.AddedItems)
            {
                if (!e.RemovedItems.Contains(addedDate))
                {
                    return true;
                }
            }
            return false;
        }

        private static bool IsValidDisplayMode(CalendarMode mode)
        {
            return mode == CalendarMode.Month
                || mode == CalendarMode.Year
                || mode == CalendarMode.Decade;
        }

        private static bool IsValidFirstDayOfWeek(object value)
        {
            DayOfWeek day = (DayOfWeek)value;

            return day == DayOfWeek.Sunday
                || day == DayOfWeek.Monday
                || day == DayOfWeek.Tuesday
                || day == DayOfWeek.Wednesday
                || day == DayOfWeek.Thursday
                || day == DayOfWeek.Friday
                || day == DayOfWeek.Saturday;
        }

        private static bool IsValidKeyboardSelection(Calendar cal, object value)
        {
            if (value == null)
            {
                return true;
            }
            else
            {
                if (cal.BlackoutDates.Contains((DateTime)value))
                {
                    return false;
                }
                else
                {
                    return (DateTime.Compare((DateTime)value, cal.DisplayDateRangeStart) >= 0 && DateTime.Compare((DateTime)value, cal.DisplayDateRangeEnd) <= 0);
                }
            }
        }

        private static bool IsValidSelectionMode(object value)
        {
            CalendarSelectionMode mode = (CalendarSelectionMode)value;

            return mode == CalendarSelectionMode.SingleDate
                || mode == CalendarSelectionMode.SingleRange
                || mode == CalendarSelectionMode.MultipleRange
                || mode == CalendarSelectionMode.None;
        }

        private void OnDisplayDate(CalendarDateChangedEventArgs e)
        {
            EventHandler<CalendarDateChangedEventArgs> handler = this.DisplayDateChanged;
            if (null != handler)
            {
                handler(this, e);
            }
        }

        private void OnDisplayModeChanged(CalendarModeChangedEventArgs args)
        {
            EventHandler<CalendarModeChangedEventArgs> handler = this.DisplayModeChanged;

            if (null != handler)
            {
                handler(this, args);
            }
        }

        private void OnHeaderClick()
        {
            Debug.Assert(this.DisplayMode == CalendarMode.Year || this.DisplayMode == CalendarMode.Decade);
            CalendarItem monthControl = this.MonthControl;
            if (monthControl != null)
            {
                monthControl.MonthView.Visibility = Visibility.Collapsed;
                monthControl.YearView.Visibility = Visibility.Visible;
                this.UpdateMonths();
            }
        }

        private void OnMonthClick()
        {
            CalendarItem monthControl = this.MonthControl;
            if (monthControl != null)
            {
                monthControl.YearView.Visibility = Visibility.Collapsed;
                monthControl.MonthView.Visibility = Visibility.Visible;

                if (!this.LastSelectedDate.HasValue || DateTimeHelper.CompareYearMonth(this.LastSelectedDate.Value, this.DisplayDate) != 0)
                {
                    this.LastSelectedDate = this.DisplayDate;
                }

                this.UpdateMonths();
            }
        }

        private void OnSelectedMonthChanged(DateTime? selectedMonth)
        {
            if (selectedMonth.HasValue)
            {
                Debug.Assert(this.DisplayMode == CalendarMode.Year);
                this.SelectedMonth = selectedMonth.Value;
                UpdateMonths();
            }
        }

        private void OnSelectedYearChanged(DateTime? selectedYear)
        {
            if (selectedYear.HasValue)
            {
                Debug.Assert(this.DisplayMode == CalendarMode.Decade);
                this.SelectedYear = selectedYear.Value;
                UpdateMonths();
            }
        }

        private bool ProcessCalendarKey(KeyEventArgs e)
        {
            if (this.DisplayMode == CalendarMode.Month)
            {
                if (this.LastSelectedDate.HasValue && this.DisplayDateInternal != null)
                {
                    //If a blackout day is inactive, when clicked on it, the previous inactive day which is not a blackout day can get the focus.
                    //In this case we should allow keyboard functions on that inactive day
                    if (DateTimeHelper.CompareYearMonth(this.LastSelectedDate.Value, this.DisplayDateInternal) != 0 && this._focusButton != null && !this._focusButton.IsInactive)
                    {
                        return true;
                    }
                }
            }

            bool ctrl, shift;
            KeyboardHelper.GetMetaKeyState(out ctrl, out shift);

            switch (e.Key)
            {
                case Key.Up:
                    {
                        ProcessUpKey(ctrl, shift);
                        return true;
                    }
                case Key.Down:
                    {
                        ProcessDownKey(ctrl, shift);
                        return true;
                    }
                case Key.Left:
                    {
                        ProcessLeftKey(shift);
                        return true;
                    }
                case Key.Right:
                    {
                        ProcessRightKey(shift);
                        return true;
                    }
                case Key.PageDown:
                    {
                        ProcessPageDownKey(shift);
                        return true;
                    }
                case Key.PageUp:
                    {
                        ProcessPageUpKey(shift);
                        return true;
                    }
                case Key.Home:
                    {
                        ProcessHomeKey(shift);
                        return true;
                    }
                case Key.End:
                    {
                        ProcessEndKey(shift);
                        return true;
                    }
                case Key.Enter:
                case Key.Space:
                    {
                        return ProcessEnterKey();
                    }

            }
            return false;
        }

        private void ProcessDownKey(bool ctrl, bool shift)
        {
            switch (this.DisplayMode)
            {
                case CalendarMode.Month:
                    {
                        if (!ctrl || shift)
                        {
                            DateTime? selectedDate = DateTimeHelper.AddDays(this.LastSelectedDate.GetValueOrDefault(DateTime.Today), COLS);
                            ProcessSelection(shift, selectedDate, COLS);
                        }
                        break;
                    }
                case CalendarMode.Year:
                    {
                        if (ctrl)
                        {
                            this.DisplayDate = this.SelectedMonth;
                            this.DisplayMode = CalendarMode.Month;
                        }
                        else
                        {
                            DateTime? selectedMonth = DateTimeHelper.AddMonths(this._selectedMonth, YEAR_COLS);
                            OnSelectedMonthChanged(selectedMonth);
                        }
                        break;
                    }
                case CalendarMode.Decade:
                    {
                        if (ctrl)
                        {
                            this.SelectedMonth = this.SelectedYear;
                            this.DisplayMode = CalendarMode.Year;
                        }
                        else
                        {
                            DateTime? selectedYear = DateTimeHelper.AddYears(this.SelectedYear, YEAR_COLS);
                            OnSelectedYearChanged(selectedYear);
                        }
                        break;
                    }
            }
        }

        private void ProcessEndKey(bool shift)
        {
            switch (this.DisplayMode)
            {
                case CalendarMode.Month:
                    {
                        if (this.DisplayDate != null)
                        {
                            DateTime? selectedDate = new DateTime((this.DisplayDateInternal).Year, (this.DisplayDateInternal).Month, 1);

                            if (DateTimeHelper.CompareYearMonth(DateTime.MaxValue, selectedDate.Value) > 0)
                            {
                                //since DisplayDate is not equal to DateTime.MaxValue we are sure selectedDate is not null
                                selectedDate = DateTimeHelper.AddMonths(selectedDate.Value, 1).Value;
                                selectedDate = DateTimeHelper.AddDays(selectedDate.Value, -1).Value;
                            }
                            else
                            {
                                selectedDate = DateTime.MaxValue;
                            }
                            ProcessSelection(shift, selectedDate, null);
                        }
                        break;
                    }
                case CalendarMode.Year:
                    {
                        DateTime selectedMonth = new DateTime(this._selectedMonth.Year, 12, 1);
                        OnSelectedMonthChanged(selectedMonth);
                        break;
                    }
                case CalendarMode.Decade:
                    {
                        DateTime? selectedYear = new DateTime(DateTimeHelper.EndOfDecade(this.SelectedYear), 1, 1);
                        OnSelectedYearChanged(selectedYear);
                        break;
                    }
            }
        }

        private bool ProcessEnterKey()
        {
            switch (this.DisplayMode)
            {
                case CalendarMode.Year:
                    {
                        this.DisplayDate = this.SelectedMonth;
                        this.DisplayMode = CalendarMode.Month;
                        return true;
                    }
                case CalendarMode.Decade:
                    {
                        this.SelectedMonth = this.SelectedYear;
                        this.DisplayMode = CalendarMode.Year;
                        return true;
                    }
            }
            return false;
        }

        private void ProcessHomeKey(bool shift)
        {
            switch (this.DisplayMode)
            {
                case CalendarMode.Month:
                    {
                        // 
                        DateTime? selectedDate = new DateTime((this.DisplayDateInternal).Year, (this.DisplayDateInternal).Month, 1);
                        ProcessSelection(shift, selectedDate, null);
                        break;
                    }
                case CalendarMode.Year:
                    {
                        DateTime selectedMonth = new DateTime(this._selectedMonth.Year, 1, 1);
                        OnSelectedMonthChanged(selectedMonth);
                        break;
                    }
                case CalendarMode.Decade:
                    {
                        DateTime? selectedYear = new DateTime(DateTimeHelper.DecadeOfDate(this.SelectedYear), 1, 1);
                        OnSelectedYearChanged(selectedYear);
                        break;
                    }
            }
        }

        private void ProcessLeftKey(bool shift)
        {
            switch (this.DisplayMode)
            {
                case CalendarMode.Month:
                    {
                        DateTime? selectedDate = DateTimeHelper.AddDays(this.LastSelectedDate.GetValueOrDefault(DateTime.Today), -1);
                        ProcessSelection(shift, selectedDate, -1);
                        break;
                    }
                case CalendarMode.Year:
                    {
                        DateTime? selectedMonth = DateTimeHelper.AddMonths(this._selectedMonth, -1);
                        OnSelectedMonthChanged(selectedMonth);
                        break;
                    }
                case CalendarMode.Decade:
                    {
                        DateTime? selectedYear = DateTimeHelper.AddYears(this.SelectedYear, -1);
                        OnSelectedYearChanged(selectedYear);
                        break;
                    }
            }
        }

        private void ProcessPageDownKey(bool shift)
        {
            switch (this.DisplayMode)
            {
                case CalendarMode.Month:
                    {
                        DateTime? selectedDate = DateTimeHelper.AddMonths(this.LastSelectedDate.GetValueOrDefault(DateTime.Today), 1);
                        ProcessSelection(shift, selectedDate, null);
                        break;
                    }
                case CalendarMode.Year:
                    {
                        DateTime? selectedMonth = DateTimeHelper.AddYears(this._selectedMonth, 1);
                        OnSelectedMonthChanged(selectedMonth);
                        break;
                    }
                case CalendarMode.Decade:
                    {
                        DateTime? selectedYear = DateTimeHelper.AddYears(this.SelectedYear, 10);
                        OnSelectedYearChanged(selectedYear);
                        break;
                    }
            }
        }

        private void ProcessPageUpKey(bool shift)
        {
            switch (this.DisplayMode)
            {
                case CalendarMode.Month:
                    {
                        DateTime? selectedDate = DateTimeHelper.AddMonths(this.LastSelectedDate.GetValueOrDefault(DateTime.Today), -1);
                        ProcessSelection(shift, selectedDate, null);
                        break;
                    }
                case CalendarMode.Year:
                    {
                        DateTime? selectedMonth = DateTimeHelper.AddYears(this._selectedMonth, -1);
                        OnSelectedMonthChanged(selectedMonth);
                        break;
                    }
                case CalendarMode.Decade:
                    {
                        DateTime? selectedYear = DateTimeHelper.AddYears(this.SelectedYear, -10);
                        OnSelectedYearChanged(selectedYear);
                        break;
                    }
            }
        }

        private void ProcessRightKey(bool shift)
        {
            switch (this.DisplayMode)
            {
                case CalendarMode.Month:
                    {
                        DateTime? selectedDate = DateTimeHelper.AddDays(this.LastSelectedDate.GetValueOrDefault(DateTime.Today), 1);
                        ProcessSelection(shift, selectedDate, 1);
                        break;
                    }
                case CalendarMode.Year:
                    {
                        DateTime? selectedMonth = DateTimeHelper.AddMonths(this._selectedMonth, 1);
                        OnSelectedMonthChanged(selectedMonth);
                        break;
                    }
                case CalendarMode.Decade:
                    {
                        DateTime? selectedYear = DateTimeHelper.AddYears(this.SelectedYear, 1);
                        OnSelectedYearChanged(selectedYear);
                        break;
                    }
            }
        }

        private void ProcessSelection(bool shift, DateTime? lastSelectedDate, int? index)
        {
            if (this.SelectionMode == CalendarSelectionMode.None && lastSelectedDate != null)
            {
                OnDayClick(lastSelectedDate.Value);
                return;
            }
            if (lastSelectedDate != null && IsValidKeyboardSelection(this, lastSelectedDate.Value))
            {
                if (this.SelectionMode == CalendarSelectionMode.SingleRange || this.SelectionMode == CalendarSelectionMode.MultipleRange)
                {
                    foreach (DateTime item in this.SelectedDates)
                    {
                        this._removedItems.Add(item);
                    }
                    this.SelectedDates.ClearInternal();
                    if (shift)
                    {
                        CalendarDayButton b;
                        _isShiftPressed = true;
                        if (this._hoverStart == null)
                        {
                            if (this.LastSelectedDate != null)
                            {
                                this._hoverStart = this.LastSelectedDate;
                            }
                            else
                            {
                                if (DateTimeHelper.CompareYearMonth(this.DisplayDateInternal, DateTime.Today) == 0)
                                {
                                    this._hoverStart = DateTime.Today;
                                }
                                else
                                {
                                    this._hoverStart = this.DisplayDateInternal;
                                }
                            }

                            b = FindDayButtonFromDay(this._hoverStart.Value);
                            if (b != null)
                            {
                                this._hoverStartIndex = b.Index;
                            }
                        }
                        //the index of the SelectedDate is always the last selectedDate's index
                        UnHighlightDays();
                        //If we hit a BlackOutDay with keyboard we do not update the HoverEnd
                        CalendarDateRange range;

                        if (DateTime.Compare(this._hoverStart.Value, lastSelectedDate.Value) < 0)
                        {
                            range = new CalendarDateRange(this._hoverStart.Value, lastSelectedDate.Value);
                        }
                        else
                        {
                            range = new CalendarDateRange(lastSelectedDate.Value, this._hoverStart.Value);
                        }

                        if (!this.BlackoutDates.ContainsAny(range))
                        {
                            this.HoverEnd = lastSelectedDate;

                            if (index.HasValue)
                            {
                                this._hoverEndIndex += index;
                            }
                            else
                            {
                                //
                                b = FindDayButtonFromDay(this._hoverEnd.Value);

                                if (b != null)
                                {
                                    this._hoverEndIndex = b.Index;
                                }
                            }
                        }

                        OnDayClick(this.HoverEnd.Value);
                        HighlightDays();
                    }
                    else
                    {
                        this._hoverStart = lastSelectedDate;
                        this.HoverEnd = lastSelectedDate;
                        AddSelection();
                        OnDayClick(lastSelectedDate.Value);
                    }
                }
                else
                {
                    //ON CLEAR 
                    this.LastSelectedDate = lastSelectedDate.Value;
                    if (this.SelectedDates.Count > 0)
                    {
                        this.SelectedDates[0] = lastSelectedDate.Value;
                    }
                    else
                    {
                        this.SelectedDates.Add(lastSelectedDate.Value);
                    }
                    OnDayClick(lastSelectedDate.Value);
                }
            }
        }

        private void ProcessShiftKeyUp()
        {
            if (_isShiftPressed && (this.SelectionMode == CalendarSelectionMode.SingleRange || this.SelectionMode == CalendarSelectionMode.MultipleRange))
            {
                AddSelection();
                _isShiftPressed = false;
            }
        }

        private void ProcessUpKey(bool ctrl, bool shift)
        {
            switch (this.DisplayMode)
            {
                case CalendarMode.Month:
                    {
                        if (ctrl)
                        {
                            this.SelectedMonth = this.DisplayDateInternal;
                            this.DisplayMode = CalendarMode.Year;
                        }
                        else
                        {
                            DateTime? selectedDate = DateTimeHelper.AddDays(this.LastSelectedDate.GetValueOrDefault(DateTime.Today), -COLS);
                            ProcessSelection(shift, selectedDate, -COLS);
                        }
                        break;
                    }
                case CalendarMode.Year:
                    {
                        if (ctrl)
                        {
                            this.SelectedYear = this.SelectedMonth;
                            this.DisplayMode = CalendarMode.Decade;
                        }
                        else
                        {
                            DateTime? selectedMonth = DateTimeHelper.AddMonths(this._selectedMonth, -YEAR_COLS);
                            OnSelectedMonthChanged(selectedMonth);
                        }
                        break;
                    }
                case CalendarMode.Decade:
                    {
                        if (!ctrl)
                        {
                            DateTime? selectedYear = DateTimeHelper.AddYears(this.SelectedYear, -YEAR_COLS);
                            OnSelectedYearChanged(selectedYear);
                        }
                        break;
                    }
            }
        }

        private static DateTime? SelectedDateMax(Calendar cal)
        {
            DateTime selectedDateMax;

            if (cal.SelectedDates.Count > 0)
            {
                selectedDateMax = cal.SelectedDates[0];
                Debug.Assert(DateTime.Compare(cal.SelectedDate.Value, selectedDateMax) == 0);
            }
            else
            {
                return null;
            }

            foreach (DateTime selectedDate in cal.SelectedDates)
            {
                if (DateTime.Compare(selectedDate, selectedDateMax) > 0)
                {
                    selectedDateMax = selectedDate;
                }
            }
            return selectedDateMax;
        }

        private static DateTime? SelectedDateMin(Calendar cal)
        {
            DateTime selectedDateMin;

            if (cal.SelectedDates.Count > 0)
            {
                selectedDateMin = cal.SelectedDates[0];
                Debug.Assert(DateTime.Compare(cal.SelectedDate.Value, selectedDateMin) == 0);
            }
            else
            {
                return null;
            }

            foreach (DateTime selectedDate in cal.SelectedDates)
            {
                if (DateTime.Compare(selectedDate, selectedDateMin) < 0)
                {
                    selectedDateMin = selectedDate;
                }
            }
            return selectedDateMin;
        }

        internal void SortHoverIndexes(out int startIndex, out int endIndex)
        {
            //not comparing indexes since the two days may not be on the same month
            // 


            if (DateTimeHelper.CompareDays(this.HoverEnd.Value, this._hoverStart.Value) > 0)
            {
                startIndex = this._hoverStartIndex.Value;
                endIndex = this._hoverEndIndex.Value;
            }
            else
            {
                startIndex = this._hoverEndIndex.Value;
                endIndex = this._hoverStartIndex.Value;
            }
        }

        #endregion Private Methods
    }
}
