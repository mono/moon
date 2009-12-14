// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Diagnostics;
using System.Globalization;
using System.Windows.Input;


namespace System.Windows.Controls.Primitives
{
    [TemplatePart(Name = CalendarItem.ElementRoot, Type = typeof(FrameworkElement))]
    [TemplatePart(Name = CalendarItem.ElementHeaderButton, Type = typeof(Button))]
    [TemplatePart(Name = CalendarItem.ElementPreviousButton, Type = typeof(Button))]
    [TemplatePart(Name = CalendarItem.ElementNextButton, Type = typeof(Button))]
    [TemplatePart(Name = CalendarItem.ElementDayTitleTemplate, Type = typeof(DataTemplate))]
    [TemplatePart(Name = CalendarItem.ElementMonthView, Type = typeof(Grid))]
    [TemplatePart(Name = CalendarItem.ElementYearView, Type = typeof(Grid))]
    [TemplatePart(Name = CalendarItem.ElementDisabledVisual, Type = typeof(FrameworkElement))]
    [TemplateVisualState(Name = VisualStates.StateNormal, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = VisualStates.StateDisabled, GroupName = VisualStates.GroupCommon)]
    public sealed partial class CalendarItem : Control
    {
        #region Constants
        private const string ElementRoot = "Root";
        private const string ElementHeaderButton = "HeaderButton";
        private const string ElementPreviousButton = "PreviousButton";
        private const string ElementNextButton = "NextButton";
        private const string ElementDayTitleTemplate = "DayTitleTemplate";
        private const string ElementMonthView = "MonthView";
        private const string ElementYearView = "YearView";
        private const string ElementDisabledVisual = "DisabledVisual";

        private const int COLS = 7;
        private const int ROWS = 7;
        private const int YEAR_COLS = 4;
        private const int YEAR_ROWS = 3;
        private const int NUMBER_OF_DAYS_IN_WEEK = 7;

        #endregion Constants

        #region Data

        private System.Globalization.Calendar _calendar = new GregorianCalendar();
        private CalendarDayButton _currentButton;
        private DateTime _currentMonth;
        private DataTemplate _dayTitleTemplate;
        private FrameworkElement _disabledVisual;
        private MouseButtonEventArgs _downEventArg;
        private MouseButtonEventArgs _downEventArgYearView;
        private Button _headerButton;
        private bool _isMouseLeftButtonDown;
        private bool _isMouseLeftButtonDownYearView;
        private Boolean _isTopLeftMostMonth = true;
        private Boolean _isTopRightMostMonth = true;
        private CalendarButton _lastCalendarButton;
        private CalendarDayButton _lastCalendarDayButton;
        private Grid _monthView;
        private Button _nextButton;
        private Button _previousButton;
        private Grid _yearView;
        #endregion Data
        /// <summary>
        /// Represents the month that is used in Calendar Control.
        /// </summary>
        public CalendarItem()
        {
            DefaultStyleKey = typeof(CalendarItem);
        }

        #region Protected Properties
        #endregion Protected Properties

        #region Internal Properties

        internal CalendarDayButton CurrentButton
        {
            get { return this._currentButton; }
            set { this._currentButton = value; }
        }

        internal Grid MonthView
        {
            get { return _monthView; }
        }

        internal Calendar Owner
        {
            get;
            set;
        }

        internal Grid YearView
        {
            get { return _yearView; }
        }

        #endregion Internal Properties

        #region Private Properties

        private Button HeaderButton
        {
            get
            {
                return this._headerButton;
            }
        }

        private Button NextButton
        {
            get
            {
                return this._nextButton;
            }
        }

        private Button PreviousButton
        {
            get
            {
                return this._previousButton;
            }
        }

        #endregion Private Properties

        #region Public Methods

        /// <summary>
        /// Invoked whenever application code or an internal process, 
        /// such as a rebuilding layout pass, calls the ApplyTemplate method.
        /// </summary>
        public override void OnApplyTemplate()
        {
            base.OnApplyTemplate();

            _monthView = GetTemplateChild(ElementMonthView) as Grid;
            _yearView = GetTemplateChild(ElementYearView) as Grid;

            if (_monthView != null)
            {
                _monthView.MouseLeave += new MouseEventHandler(MonthView_MouseLeave);
            }

            if (_yearView != null)
            {
                _yearView.MouseLeave += new MouseEventHandler(YearView_MouseLeave);
            }

            _previousButton = GetTemplateChild(ElementPreviousButton) as Button;
            if (this._previousButton != null)
            {
                //If the user does not provide a Content value in template, we provide a helper text that can be used in Accessibility
                //this text is not shown on the UI, just used for Accessibility purposes
                if (this._previousButton.Content == null)
                {
                    this._previousButton.Content = Resource.Calendar_PreviousButtonName;
                }

                if (_isTopLeftMostMonth)
                {
                    this._previousButton.Visibility = Visibility.Visible;
                    this._previousButton.Click += new RoutedEventHandler(PreviousButton_Click);
                    this._previousButton.IsTabStop = false;
                }
            }

            _nextButton = GetTemplateChild(ElementNextButton) as Button;
            if (this._nextButton != null)
            {
                //If the user does not provide a Content value in template, we provide a helper text that can be used in Accessibility
                //this text is not shown on the UI, just used for Accessibility purposes
                if (this._nextButton.Content == null)
                {
                    this._nextButton.Content = Resource.Calendar_NextButtonName;
                }

                if (_isTopRightMostMonth)
                {
                    this._nextButton.Visibility = Visibility.Visible;
                    this._nextButton.Click += new RoutedEventHandler(NextButton_Click);
                    this._nextButton.IsTabStop = false;

                }
            }

            _headerButton = GetTemplateChild(ElementHeaderButton) as Button;
            if (this._headerButton != null)
            {
                this._headerButton.Click += new RoutedEventHandler(HeaderButton_Click);
                this._headerButton.IsTabStop = false;
            }

            _dayTitleTemplate = GetTemplateChild(ElementDayTitleTemplate) as DataTemplate;

            _disabledVisual = GetTemplateChild(ElementDisabledVisual) as FrameworkElement;

            if (this.Owner != null)
            {
                UpdateDisabledGrid(this.Owner.IsEnabled);
            }

            PopulateGrids();

            if (_monthView != null && _yearView != null)
            {
                if (this.Owner != null)
                {
                    this.Owner.SelectedMonth = this.Owner.DisplayDateInternal;
                    this.Owner.SelectedYear = this.Owner.DisplayDateInternal;

                    if (this.Owner.DisplayMode == CalendarMode.Year)
                    {
                        UpdateYearMode();
                    }
                    else if (this.Owner.DisplayMode == CalendarMode.Decade)
                    {
                        UpdateDecadeMode();
                    }

                    if (this.Owner.DisplayMode == CalendarMode.Month)
                    {
                        UpdateMonthMode();
                        _monthView.Visibility = Visibility.Visible;
                        _yearView.Visibility = Visibility.Collapsed;
                    }
                    else
                    {
                        _yearView.Visibility = Visibility.Visible;
                        _monthView.Visibility = Visibility.Collapsed;
                    }
                }
                else
                {
                    UpdateMonthMode();
                    _monthView.Visibility = Visibility.Visible;
                    _yearView.Visibility = Visibility.Collapsed;
                }
            }
        }

        #endregion Public Methods

        #region Protected methods

        #endregion Protected Methods

        #region Internal Methods

        internal void Month_CalendarButtonMouseUp(object sender, MouseButtonEventArgs e)
        {
            this._isMouseLeftButtonDownYearView = false;

            if (this.Owner != null)
            {
                DateTime newmonth = (DateTime)((CalendarButton)sender).DataContext;

                if (this.Owner.DisplayMode == CalendarMode.Year)
                {
                    this.Owner.DisplayDate = newmonth;
                    this.Owner.DisplayMode = CalendarMode.Month;
                }
                else
                {
                    Debug.Assert(this.Owner.DisplayMode == CalendarMode.Decade);
                    this.Owner.SelectedMonth = newmonth;
                    this.Owner.DisplayMode = CalendarMode.Year;

                }
            }
        }

        internal void UpdateDecadeMode()
        {
            DateTime selectedYear;

            if (this.Owner != null)
            {
                Debug.Assert(this.Owner.SelectedYear != null);
                selectedYear = this.Owner.SelectedYear;
                _currentMonth = (DateTime)this.Owner.SelectedMonth;
            }
            else
            {
                _currentMonth = DateTime.Today;
                selectedYear = DateTime.Today;
            }

            if (_currentMonth != null)
            {
                int decade = DateTimeHelper.DecadeOfDate(selectedYear);
                int decadeEnd = DateTimeHelper.EndOfDecade(selectedYear);

                SetDecadeModeHeaderButton(decade, decadeEnd);
                SetDecadeModePreviousButton(decade);
                SetDecadeModeNextButton(decadeEnd);

                if (_yearView != null)
                {
                    SetYearButtons(decade, decadeEnd);
                }
            }
        }

        internal void UpdateDisabledGrid(bool isEnabled)
        {
            if (isEnabled)
            {
                if (_disabledVisual != null)
                {
                    _disabledVisual.Visibility = Visibility.Collapsed;
                }
                VisualStates.GoToState(this, true, VisualStates.StateNormal);
            }
            else
            {
                if (_disabledVisual != null)
                {
                    _disabledVisual.Visibility = Visibility.Visible;
                }
                VisualStates.GoToState(this, true, VisualStates.StateDisabled, VisualStates.StateNormal);
            }
        }

        internal void UpdateMonthMode()
        {
            if (this.Owner != null)
            {
                Debug.Assert(this.Owner.DisplayDate != null);
                _currentMonth = this.Owner.DisplayDateInternal;
            }
            else
            {
                _currentMonth = DateTime.Today;
            }

            if (_currentMonth != null)
            {
                SetMonthModeHeaderButton();
                SetMonthModePreviousButton(_currentMonth);
                SetMonthModeNextButton(_currentMonth);

                if (_monthView != null)
                {
                    SetDayTitles();
                    SetCalendarDayButtons(_currentMonth);
                }
            }
        }

        internal void UpdateYearMode()
        {
            if (this.Owner != null)
            {
                Debug.Assert(this.Owner.SelectedMonth != null);
                _currentMonth = (DateTime)this.Owner.SelectedMonth;
            }
            else
            {
                _currentMonth = DateTime.Today;
            }

            if (_currentMonth != null)
            {
                SetYearModeHeaderButton();
                SetYearModePreviousButton();
                SetYearModeNextButton();

                if (_yearView != null)
                {
                    SetMonthButtonsForYearMode();
                }
            }
        }

        internal void UpdateYearViewSelection(CalendarButton calendarButton)
        {
            if (this.Owner != null && calendarButton!= null && calendarButton.DataContext != null)
            {
                this.Owner.FocusCalendarButton.IsFocusedOverride = false;
                this.Owner.FocusCalendarButton = calendarButton;
                calendarButton.IsFocusedOverride = this.Owner._hasFocus;

                if (this.Owner.DisplayMode == CalendarMode.Year)
                {
                    this.Owner.SelectedMonth = (DateTime)calendarButton.DataContext;
                }
                else
                {
                    this.Owner.SelectedYear = (DateTime)calendarButton.DataContext;
                }
            }
        }

        #endregion Internal Methods

        #region Private Methods

        private void AddSelection(CalendarDayButton b)
        {
            if (this.Owner != null)
            {
                this.Owner._hoverEndIndex = b.Index;
                this.Owner.HoverEnd = (DateTime)b.DataContext;

                if (this.Owner.HoverEnd != null && this.Owner._hoverStart != null)
                {
                    //this is selection with Mouse, we do not guarantee the range does not include BlackOutDates.
                    //AddRange method will throw away the BlackOutDates based on the SelectionMode
                    this.Owner._isMouseSelection = true;
                    this.Owner.SelectedDates.AddRange(this.Owner._hoverStart.Value, this.Owner.HoverEnd.Value);
                    Owner.OnDayClick((DateTime)b.DataContext);
                }
            }
        }

        private void Cell_Click(object sender, RoutedEventArgs e)
        {
            if (this.Owner != null)
            {
                bool ctrl, shift;
                KeyboardHelper.GetMetaKeyState(out ctrl, out shift);

                if (ctrl && this.Owner.SelectionMode == CalendarSelectionMode.MultipleRange)
                {
                    CalendarDayButton b = sender as CalendarDayButton;
                    Debug.Assert(b != null);

                    if (b.IsSelected)
                    {
                        this.Owner._hoverStart = null;
                        this._isMouseLeftButtonDown = false;
                        b.IsSelected = false;
                        if (b.DataContext != null)
                        {
                            this.Owner.SelectedDates.Remove((DateTime)b.DataContext);
                        }
                    }
                }
            }
        }

        private void Cell_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            if (this.Owner != null)
            {
                if (!this.Owner._hasFocus)
                {
                    this.Owner.Focus();
                }

                bool ctrl, shift;
                KeyboardHelper.GetMetaKeyState(out ctrl, out shift);
                CalendarDayButton b = sender as CalendarDayButton;

                if (b != null)
                {
                    if (b.IsEnabled && !b.IsDisabled)
                    {
                        DateTime selectedDate = (DateTime)b.DataContext;
                        Debug.Assert(selectedDate != null);
                        _isMouseLeftButtonDown = true;
                        //null check is added for unit tests
                        if (e != null)
                        {
                            _downEventArg = e;
                        }

                        switch (this.Owner.SelectionMode)
                        {
                            case CalendarSelectionMode.None:
                                {
                                    return;
                                }
                            case CalendarSelectionMode.SingleDate:
                                {
                                    this.Owner.DatePickerDisplayDateFlag = true;
                                    if (this.Owner.SelectedDates.Count == 0)
                                    {
                                        this.Owner.SelectedDates.Add(selectedDate);
                                    }
                                    else
                                    {
                                        this.Owner.SelectedDates[0] = selectedDate;
                                    }
                                    return;
                                }
                            case CalendarSelectionMode.SingleRange:
                                {
                                    //Set the start or end of the selection range
                                    if (shift)
                                    {
                                        this.Owner.UnHighlightDays();
                                        this.Owner.HoverEnd = selectedDate;
                                        this.Owner._hoverEndIndex = b.Index;
                                        this.Owner.HighlightDays();
                                    }
                                    else
                                    {
                                        this.Owner.UnHighlightDays();
                                        this.Owner._hoverStart = selectedDate;
                                        this.Owner._hoverStartIndex = b.Index;
                                    }
                                    return;
                                }
                            case CalendarSelectionMode.MultipleRange:
                                {
                                    if (shift)
                                    {
                                        if (!ctrl)
                                        {
                                            //clear the list, set the states to default
                                            foreach (DateTime item in this.Owner.SelectedDates)
                                            {
                                                this.Owner._removedItems.Add(item);
                                            }
                                            this.Owner.SelectedDates.ClearInternal();
                                        }
                                        this.Owner.HoverEnd = selectedDate;
                                        this.Owner._hoverEndIndex = b.Index;
                                        this.Owner.HighlightDays();
                                    }
                                    else
                                    {
                                        if (!ctrl)
                                        {
                                            //clear the list, set the states to default
                                            foreach (DateTime item in this.Owner.SelectedDates)
                                            {
                                                this.Owner._removedItems.Add(item);
                                            }
                                            this.Owner.SelectedDates.ClearInternal();
                                            this.Owner.UnHighlightDays();
                                        }
                                        this.Owner._hoverStart = selectedDate;
                                        this.Owner._hoverStartIndex = b.Index;
                                    }
                                    return;
                                }
                        }
                    }
                    else
                    {
                        //If a click occurs on a BlackOutDay we set the HoverStart to be null
                        this.Owner._hoverStart = null;
                    }
                }
            }
        }

        private void Cell_MouseEnter(object sender, MouseEventArgs e)
        {
            if (this.Owner != null)
            {
                CalendarDayButton b = sender as CalendarDayButton;
                if (_isMouseLeftButtonDown && b != null && b.IsEnabled && !b.IsDisabled)
                {
                    //Update the states of all buttons to be selected starting from
                    // _hoverStart to b
                    switch (this.Owner.SelectionMode)
                    {
                        case CalendarSelectionMode.SingleDate:
                            {
                                DateTime selectedDate = (DateTime)b.DataContext;
                                this.Owner.DatePickerDisplayDateFlag = true;
                                if (this.Owner.SelectedDates.Count == 0)
                                {
                                    this.Owner.SelectedDates.Add(selectedDate);
                                }
                                else
                                {
                                    this.Owner.SelectedDates[0] = selectedDate;
                                }
                                return;
                            }
                        case CalendarSelectionMode.SingleRange:
                        case CalendarSelectionMode.MultipleRange:
                            {
                                Debug.Assert(b.DataContext != null);
                                this.Owner.UnHighlightDays();
                                this.Owner._hoverEndIndex = b.Index;
                                this.Owner.HoverEnd = (DateTime)b.DataContext;
                                //Update the States of the buttons
                                this.Owner.HighlightDays();
                                return;
                            }
                    }
                }
            }
        }

        private void Cell_MouseLeave(object sender, MouseEventArgs e)
        {
            if(_isMouseLeftButtonDown)
            {
                CalendarDayButton b = sender as CalendarDayButton;
                //The button is in Pressed state. Change the state to normal.
                b.ReleaseMouseCapture();
                //null check is added for unit tests
                if (_downEventArg != null)
                {
                    b.SendMouseUpEvent(_downEventArg);
                }
                this._lastCalendarDayButton = b;
            }
        }

        private void Cell_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            if (this.Owner != null)
            {
                bool ctrl, shift;
                KeyboardHelper.GetMetaKeyState(out ctrl, out shift);
                CalendarDayButton b = sender as CalendarDayButton;
                if (b != null && !b.IsDisabled)
                {
                    this.Owner.OnDayButtonMouseUp(e);
                }
                _isMouseLeftButtonDown = false;
                if (b != null && b.DataContext != null)
                {
                    if (this.Owner.SelectionMode == CalendarSelectionMode.None || this.Owner.SelectionMode == CalendarSelectionMode.SingleDate)
                    {
                        this.Owner.OnDayClick((DateTime)b.DataContext);
                        return;
                    }
                    if (this.Owner._hoverStart.HasValue)
                    {
                        switch (this.Owner.SelectionMode)
                        {
                            case CalendarSelectionMode.SingleRange:
                                {
                                    //Update SelectedDates
                                    foreach (DateTime item in this.Owner.SelectedDates)
                                    {
                                        this.Owner._removedItems.Add(item);
                                    }
                                    this.Owner.SelectedDates.ClearInternal();
                                    AddSelection(b);
                                    return;
                                }
                            case CalendarSelectionMode.MultipleRange:
                                {
                                    //add the selection (either single day or SingleRange day)
                                    AddSelection(b);
                                    return;
                                }
                        }
                    }
                    else
                    {
                        //If the day is Disabled but a trailing day we should be able to switch months
                        if (b.IsInactive && b.IsDisabled)
                        {
                            this.Owner.OnDayClick((DateTime)b.DataContext);
                        }
                    }
                }
            }
        }

        private void HeaderButton_Click(object sender, RoutedEventArgs e)
        {
            if (this.Owner != null)
            {
                if (!this.Owner._hasFocus)
                {
                    this.Owner.Focus();
                }
                Button b = sender as Button;
                DateTime d;

                if (b.IsEnabled)
                {
                    if (this.Owner.DisplayMode == CalendarMode.Month)
                    {
                        if (this.Owner.DisplayDate != null)
                        {
                            d = this.Owner.DisplayDateInternal;
                            this.Owner.SelectedMonth = new DateTime(d.Year, d.Month, 1);
                        }
                        this.Owner.DisplayMode = CalendarMode.Year;
                    }
                    else
                    {
                        Debug.Assert(this.Owner.DisplayMode == CalendarMode.Year);

                        if (this.Owner.SelectedMonth != null)
                        {
                            d = this.Owner.SelectedMonth;
                            this.Owner.SelectedYear = new DateTime(d.Year, d.Month, 1);
                        }
                        this.Owner.DisplayMode = CalendarMode.Decade;
                    }
                }
            }
        }

        private void Month_CalendarButtonMouseDown(object sender, MouseButtonEventArgs e)
        {
            CalendarButton b = sender as CalendarButton;
            Debug.Assert(b != null);

            this._isMouseLeftButtonDownYearView = true;

            if (e != null)
            {
                _downEventArgYearView = e;
            }

            UpdateYearViewSelection(b);
        }

        private void Month_MouseEnter(object sender, MouseEventArgs e)
        {
            if (_isMouseLeftButtonDownYearView)
            {
                CalendarButton b = sender as CalendarButton;
                Debug.Assert(b != null);
                UpdateYearViewSelection(b);
            }
        }

        private void Month_MouseLeave(object sender, MouseEventArgs e)
        {
            if (_isMouseLeftButtonDownYearView)
            {
                CalendarButton b = sender as CalendarButton;
                //The button is in Pressed state. Change the state to normal.
                b.ReleaseMouseCapture();
                if (_downEventArgYearView != null)
                {
                    b.SendMouseUpEvent(_downEventArgYearView);
                }
                this._lastCalendarButton = b;
            }
        }

        private void MonthView_MouseLeave(object sender, MouseEventArgs e)
        {
            if (this._lastCalendarDayButton != null)
            {
                this._lastCalendarDayButton.CaptureMouse();
            }
        }

        private void NextButton_Click(object sender, RoutedEventArgs e)
        {
            if (this.Owner != null)
            {
                if (!this.Owner._hasFocus)
                {
                    this.Owner.Focus();
                }
                Button b = sender as Button;

                if (b.IsEnabled)
                {
                    this.Owner.OnNextClick();
                }
            }
        }

        private void PopulateGrids()
        {
            if (_monthView != null)
            {

                for (int i = 0; i < COLS; i++)
                {
                    if (_dayTitleTemplate != null)
                    {
                        FrameworkElement cell = (FrameworkElement)this._dayTitleTemplate.LoadContent();
                        cell.SetValue(Grid.RowProperty, 0);
                        cell.SetValue(Grid.ColumnProperty, i);
                        this._monthView.Children.Add(cell);
                    }
                }

                for (int i = 1; i < ROWS; i++)
                {
                    for (int j = 0; j < COLS; j++)
                    {
                        CalendarDayButton cell = new CalendarDayButton();

                        if (this.Owner != null)
                        {
                            cell.Owner = this.Owner;
                            if (this.Owner.CalendarDayButtonStyle != null)
                            {
                                cell.Style = this.Owner.CalendarDayButtonStyle;
                            }
                        }
                        cell.SetValue(Grid.RowProperty, i);
                        cell.SetValue(Grid.ColumnProperty, j);
                        cell.CalendarDayButtonMouseDown += new MouseButtonEventHandler(Cell_MouseLeftButtonDown);
                        cell.CalendarDayButtonMouseUp += new MouseButtonEventHandler(Cell_MouseLeftButtonUp);
                        cell.MouseEnter += new MouseEventHandler(Cell_MouseEnter);
                        cell.MouseLeave += new MouseEventHandler(Cell_MouseLeave);
                        cell.Click += new RoutedEventHandler(Cell_Click);
                        this._monthView.Children.Add(cell);
                    }
                }
            }

            if (_yearView != null)
            {
                CalendarButton month;
                int count = 0;
                for (int i = 0; i < YEAR_ROWS; i++)
                {
                    for (int j = 0; j < YEAR_COLS; j++)
                    {
                        month = new CalendarButton();

                        if (this.Owner != null)
                        {
                            month.Owner = this.Owner;
                            if (this.Owner.CalendarButtonStyle != null)
                            {
                                month.Style = this.Owner.CalendarButtonStyle;
                            }
                        }
                        month.SetValue(Grid.RowProperty, i);
                        month.SetValue(Grid.ColumnProperty, j);
                        month.CalendarButtonMouseDown += new MouseButtonEventHandler(Month_CalendarButtonMouseDown);
                        month.CalendarButtonMouseUp += new MouseButtonEventHandler(Month_CalendarButtonMouseUp);
                        month.MouseEnter += new MouseEventHandler(Month_MouseEnter);
                        month.MouseLeave += new MouseEventHandler(Month_MouseLeave);
                        this._yearView.Children.Add(month);
                        count++;
                    }
                }
            }
        }

        private void PreviousButton_Click(object sender, RoutedEventArgs e)
        {
            if (this.Owner != null)
            {
                if (!this.Owner._hasFocus)
                {
                    this.Owner.Focus();
                }

                Button b = sender as Button;
                if (b.IsEnabled)
                {
                    this.Owner.OnPreviousClick();
                }
            }
        }

        // How many days of the previous month need to be displayed
        private int PreviousMonthDays(DateTime firstOfMonth)
        {
            DayOfWeek day = _calendar.GetDayOfWeek(firstOfMonth);
            int i;

            if (this.Owner != null)
            {
                i = ((day - this.Owner.FirstDayOfWeek + NUMBER_OF_DAYS_IN_WEEK) % NUMBER_OF_DAYS_IN_WEEK);
            }
            else
            {
                i = ((day - DateTimeHelper.GetCurrentDateFormat().FirstDayOfWeek + NUMBER_OF_DAYS_IN_WEEK) % NUMBER_OF_DAYS_IN_WEEK);
            }

            if (i == 0)
            {
                return NUMBER_OF_DAYS_IN_WEEK;
            }
            else
            {
                return i;
            }
        }

        private void SetButtonState(CalendarDayButton childButton, DateTime dateToAdd)
        {
            if (this.Owner != null)
            {
                childButton.Opacity = 1;

                //If the day is outside the DisplayDateStart/End boundary, do not show it
                if (DateTimeHelper.CompareDays(dateToAdd, this.Owner.DisplayDateRangeStart) < 0 || DateTimeHelper.CompareDays(dateToAdd, this.Owner.DisplayDateRangeEnd) > 0)
                {
                    childButton.IsEnabled = false;
                    childButton.Opacity = 0;
                }
                else
                {
                    //SET IF THE DAY IS SELECTABLE OR NOT

                    if (this.Owner.BlackoutDates.Contains(dateToAdd))
                    {
                        childButton.IsDisabled = true;
                    }
                    else
                    {
                        childButton.IsDisabled = false;
                    }
                    childButton.IsEnabled = true;
                    //SET IF THE DAY IS INACTIVE OR NOT: set if the day is a trailing day or not

                    childButton.IsInactive = (DateTimeHelper.CompareYearMonth(dateToAdd, this.Owner.DisplayDateInternal) != 0);

                    //SET IF THE DAY IS TODAY OR NOT

                    childButton.IsToday = (this.Owner.IsTodayHighlighted && dateToAdd == DateTime.Today);

                    //SET IF THE DAY IS SELECTED OR NOT
                    childButton.IsSelected = false;
                    foreach (DateTime item in this.Owner.SelectedDates)
                    {
                        //Since we should be comparing the Date values not DateTime values, we can't use this.Owner.SelectedDates.Contains(dateToAdd) directly
                        childButton.IsSelected |= (DateTimeHelper.CompareDays(dateToAdd,item) == 0);
                    }

                    //SET THE FOCUS ELEMENT

                    if (this.Owner.LastSelectedDate != null)
                    {
                        if (DateTimeHelper.CompareDays(this.Owner.LastSelectedDate.Value, dateToAdd) == 0)
                        {
                            if (this.Owner._focusButton != null)
                            {
                                this.Owner._focusButton.IsCurrent = false;
                            }
                            this.Owner._focusButton = childButton;
                            if (this.Owner._hasFocus)
                            {
                                this.Owner._focusButton.IsCurrent = true;
                            }
                        }
                        else
                        {
                            childButton.IsCurrent = false;
                        }
                    }
                }
            }
        }

        private void SetCalendarDayButtons(DateTime firstDayOfMonth)
        {
            int lastMonthToDisplay = PreviousMonthDays(firstDayOfMonth);
            DateTime dateToAdd;

            if (DateTimeHelper.CompareYearMonth(firstDayOfMonth, DateTime.MinValue) > 0)
            {
                //DisplayDate is not equal to DateTime.MinValue
                //we can subtract days from the DisplayDate
                dateToAdd = _calendar.AddDays(firstDayOfMonth, -lastMonthToDisplay);
            }
            else
            {
                dateToAdd = firstDayOfMonth;
            }

            if (this.Owner != null && this.Owner.HoverEnd != null && this.Owner._hoverStart != null)
            {
                this.Owner._hoverEndIndex = null;
                this.Owner._hoverStartIndex = null;
            }

            int count = ROWS * COLS;

            for (int childIndex = COLS; childIndex < count; childIndex++)
            {
                CalendarDayButton childButton = _monthView.Children[childIndex] as CalendarDayButton;
                Debug.Assert(childButton != null);

                childButton.Index = childIndex;
                SetButtonState(childButton, dateToAdd);

                //Update the indexes of hoverStart and hoverEnd

                if (this.Owner != null && this.Owner.HoverEnd != null && this.Owner._hoverStart != null)
                {
                    if (DateTimeHelper.CompareDays(dateToAdd, this.Owner.HoverEnd.Value) == 0)
                    {
                        this.Owner._hoverEndIndex = childIndex;
                    }

                    if (DateTimeHelper.CompareDays(dateToAdd, this.Owner._hoverStart.Value) == 0)
                    {
                        this.Owner._hoverStartIndex = childIndex;
                    }

                }

                childButton.IsTabStop = false;
                childButton.Content = dateToAdd.Day.ToString(DateTimeHelper.GetCurrentDateFormat());
                childButton.DataContext = dateToAdd;

                if (DateTime.Compare((DateTime)DateTimeHelper.DiscardTime(DateTime.MaxValue), dateToAdd) > 0)
                {
                    //Since we are sure DisplayDate is not equal to DateTime.MaxValue, 
                    //it is safe to use AddDays 
                    dateToAdd = _calendar.AddDays(dateToAdd, 1);
                }
                else
                {
                    //DisplayDate is equal to the DateTime.MaxValue, so there are no trailing days
                    childIndex++;
                    for (int i = childIndex; i < count; i++)
                    {
                        childButton = _monthView.Children[i] as CalendarDayButton;
                        Debug.Assert(childButton != null);
                        //button needs a content to occupy the necessary space for the content presenter
                        childButton.Content = i.ToString(DateTimeHelper.GetCurrentDateFormat());
                        childButton.IsEnabled = false;
                        childButton.Opacity = 0;
                    }
                    return;
                }
            }

            //If the _hoverStart or _hoverEnd could not be found on the DisplayMonth set the values of the _hoverStartIndex or _hoverEndIndex to be the 
            // first or last day indexes on the current month
            if (this.Owner != null && this.Owner._hoverStart.HasValue && this.Owner._hoverEnd.HasValue)
            {
                if (!this.Owner._hoverEndIndex.HasValue)
                {
                    if (DateTimeHelper.CompareDays(this.Owner._hoverEnd.Value, this.Owner._hoverStart.Value) > 0)
                    {
                        this.Owner._hoverEndIndex = COLS * ROWS - 1;
                    }
                    else
                    {
                        this.Owner._hoverEndIndex = COLS;
                    }
                }

                if (!this.Owner._hoverStartIndex.HasValue)
                {
                    if (DateTimeHelper.CompareDays(this.Owner._hoverEnd.Value, this.Owner._hoverStart.Value) > 0)
                    {
                        this.Owner._hoverStartIndex = COLS;
                    }
                    else
                    {
                        this.Owner._hoverStartIndex = COLS * ROWS - 1;
                    }
                }
            }
        }

        private void SetDayTitles()
        {
            for (int childIndex = 0; childIndex < COLS; childIndex++)
            {
                FrameworkElement daytitle = _monthView.Children[childIndex] as FrameworkElement;
                if (daytitle != null)
                {
                    if (this.Owner != null)
                    {
                        daytitle.DataContext = DateTimeHelper.GetCurrentDateFormat().ShortestDayNames[(childIndex + (int)this.Owner.FirstDayOfWeek) % NUMBER_OF_DAYS_IN_WEEK];
                    }
                    else
                    {
                        daytitle.DataContext = DateTimeHelper.GetCurrentDateFormat().ShortestDayNames[(childIndex + (int)DateTimeHelper.GetCurrentDateFormat().FirstDayOfWeek) % NUMBER_OF_DAYS_IN_WEEK];
                    }
                }
            }
        }

        private void SetDecadeModeHeaderButton(int decade,int decadeEnd)
        {
            if (this._headerButton != null)
            {
                this._headerButton.Content = decade.ToString(CultureInfo.CurrentCulture) + "-" + decadeEnd.ToString(CultureInfo.CurrentCulture);
                this._headerButton.IsEnabled = false;
            }
        }

        private void SetDecadeModeNextButton(int decadeEnd)
        {
            if ( this.Owner != null && _nextButton != null)
            {
                _nextButton.IsEnabled = (this.Owner.DisplayDateRangeEnd.Year > decadeEnd);
            }
        }

        private void SetDecadeModePreviousButton(int decade)
        {
            if ( this.Owner != null && _previousButton != null)
            {
                _previousButton.IsEnabled = (decade > this.Owner.DisplayDateRangeStart.Year);
            }
        }

        private void SetMonthButtonsForYearMode()
        {
            int count = 0;
            foreach (object child in _yearView.Children)
            {
                CalendarButton childButton = child as CalendarButton;
                Debug.Assert(childButton != null);
                //There should be no time component. Time is 12:00 AM
                DateTime day = new DateTime(_currentMonth.Year, count + 1, 1);
                childButton.DataContext = day;

                childButton.Content = DateTimeHelper.GetCurrentDateFormat().AbbreviatedMonthNames[count];
                childButton.Visibility = Visibility.Visible;

                if (this.Owner != null)
                {
                    if (day.Year == _currentMonth.Year && day.Month == _currentMonth.Month && day.Day == _currentMonth.Day)
                    {
                        this.Owner.FocusCalendarButton = childButton;
                        childButton.IsFocusedOverride = this.Owner._hasFocus;
                    }
                    else
                    {
                        childButton.IsFocusedOverride = false;
                    }

                    Debug.Assert(this.Owner.DisplayDateInternal != null);
                    childButton.IsSelected = (DateTimeHelper.CompareYearMonth(day, this.Owner.DisplayDateInternal) == 0);

                    if (DateTimeHelper.CompareYearMonth(day, this.Owner.DisplayDateRangeStart) < 0 || DateTimeHelper.CompareYearMonth(day, this.Owner.DisplayDateRangeEnd) > 0)
                    {
                        childButton.IsEnabled = false;
                        childButton.Opacity = 0;
                    }
                    else
                    {
                        childButton.IsEnabled = true;
                        childButton.Opacity = 1;
                    }
                }

                childButton.IsInactive = false;
                count++;
            }

        }

        private void SetMonthModeHeaderButton()
        {
            if ( this._headerButton != null)
            {
                if (this.Owner != null)
                {
                    this._headerButton.Content = this.Owner.DisplayDateInternal.ToString("Y", DateTimeHelper.GetCurrentDateFormat());
                    this._headerButton.IsEnabled = true;
                }
                else
                {
                    this._headerButton.Content = DateTime.Today.ToString("Y", DateTimeHelper.GetCurrentDateFormat());
                }
            }
        }

        private void SetMonthModeNextButton(DateTime firstDayOfMonth)
        {
            if (this.Owner != null && _nextButton != null)
            {
                //DisplayDate is equal to DateTime.MaxValue
                if (DateTimeHelper.CompareYearMonth(firstDayOfMonth, DateTime.MaxValue) == 0)
                {
                    _nextButton.IsEnabled = false;
                }
                else
                {
                    //Since we are sure DisplayDate is not equal to DateTime.MaxValue, 
                    //it is safe to use AddMonths  
                    DateTime firstDayOfNextMonth = _calendar.AddMonths(firstDayOfMonth, 1);
                    _nextButton.IsEnabled = (DateTimeHelper.CompareDays(this.Owner.DisplayDateRangeEnd, firstDayOfNextMonth) > -1);
                }
            }
        }

        private void SetMonthModePreviousButton(DateTime firstDayOfMonth)
        {
            if (this.Owner != null && _previousButton != null)
            {
                _previousButton.IsEnabled = ( DateTimeHelper.CompareDays(this.Owner.DisplayDateRangeStart, firstDayOfMonth) < 0);
            }
        }

        private void SetYearButtons(int decade, int decadeEnd)
        {
            int year;
            int count = -1;
            foreach (object child in _yearView.Children)
            {
                CalendarButton childButton = child as CalendarButton;
                Debug.Assert(childButton != null);
                year = decade + count;

                if (year <= DateTime.MaxValue.Year && year >= DateTime.MinValue.Year)
                {
                    //There should be no time component. Time is 12:00 AM
                    DateTime day = new DateTime(year, 1, 1);
                    childButton.DataContext = day;
                    childButton.Content = year.ToString(DateTimeHelper.GetCurrentDateFormat());
                    childButton.Visibility = Visibility.Visible;

                    if (this.Owner != null)
                    {
                        if (year == this.Owner.SelectedYear.Year)
                        {
                            this.Owner.FocusCalendarButton = childButton;
                            childButton.IsFocusedOverride = this.Owner._hasFocus;
                        }
                        else
                        {
                            childButton.IsFocusedOverride = false;
                        }
                        childButton.IsSelected = (Owner.DisplayDate.Year == year);

                        if (year < this.Owner.DisplayDateRangeStart.Year || year > this.Owner.DisplayDateRangeEnd.Year)
                        {
                            childButton.IsEnabled = false;
                            childButton.Opacity = 0;
                        }
                        else
                        {
                            childButton.IsEnabled = true;
                            childButton.Opacity = 1;
                        }
                    }

                    //SET IF THE YEAR IS INACTIVE OR NOT: set if the year is a trailing year or not

                    childButton.IsInactive = (year < decade || year > decadeEnd);
                }
                else
                {
                    childButton.IsEnabled = false;
                    childButton.Opacity = 0;
                }

                count++;
            }

        }

        private void SetYearModeHeaderButton()
        {
            if (this._headerButton != null)
            {
                this._headerButton.IsEnabled = true;
                this._headerButton.Content = _currentMonth.Year.ToString(DateTimeHelper.GetCurrentDateFormat());
            }
        }

        private void SetYearModeNextButton()
        {
            if ( this.Owner != null && _nextButton != null)
            {
                _nextButton.IsEnabled = (this.Owner.DisplayDateRangeEnd.Year != _currentMonth.Year);
            }
        }

        private void SetYearModePreviousButton()
        {
            if ( this.Owner != null && _previousButton != null)
            {
                _previousButton.IsEnabled = (this.Owner.DisplayDateRangeStart.Year != _currentMonth.Year);
            }
        }

        private void YearView_MouseLeave(object sender, MouseEventArgs e)
        {
            if (this._lastCalendarButton != null)
            {
                this._lastCalendarButton.CaptureMouse();
            }
        }

        #endregion Private Methods
    }
}
