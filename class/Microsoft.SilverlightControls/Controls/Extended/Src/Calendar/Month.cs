// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System.Diagnostics;
using System.Globalization; 
using System.Windows.Input; 
using System.Windows.Media.Animation;
using System.Windows.Controls;
 

namespace System.Windows.Controlsb1
{ 
    [TemplatePart(Name = Month.CALENDAR_elementHeaderName, Type = typeof(Button))]
    [TemplatePart(Name = Month.CALENDAR_elementPreviousName, Type = typeof(Button))]
    [TemplatePart(Name = Month.CALENDAR_elementNextName, Type = typeof(Button))] 
    [TemplatePart(Name = Month.CALENDAR_stateNormal, Type = typeof(Storyboard))] 
    [TemplatePart(Name = Month.CALENDAR_stateDisabled, Type = typeof(Storyboard))]
 
    internal class Month : Control
    {
        #region Constants 
        private const string CALENDAR_elementHeaderName = "HeaderButtonElement";
        private const string CALENDAR_elementNextName = "NextButtonElement";
        private const string CALENDAR_elementPreviousName = "PreviousButtonElement"; 
        private const string CALENDAR_stateNormal = "Normal State"; 
        private const string CALENDAR_stateDisabled = "Disabled State";
        private const int COLS = 7; 
        private const int ROWS = 7;
        private const int YEAR_COLS = 4;
        private const int YEAR_ROWS = 3; 
        private const int NUMBER_OF_DAYS_IN_WEEK = 7;

        #endregion Constants 
 
        #region Data
 
        private Grid _backgroundGrid;
        private System.Globalization.Calendar _calendar = new GregorianCalendar();
        private DayButton _currentButton; 
        private DateTime _currentMonth;
        private Storyboard _currentStoryboard;
        private DataTemplate _dayTitleTemplate; 
        private Grid _disabledGrid; 
        private DayOfWeek _firstDayOfWeek;
        private Button _headerButton; 
        private Boolean _isTopLeftMostMonth = true;
        private Boolean _isTopRightMostMonth = true;
        // 
        private Grid _monthGrid;
        private Button _nextButton;
        private Calendar _owner; 
        private Button _previousButton; 
        private Grid _rootGrid;
        private Storyboard _stateDisabled; 
        private Storyboard _stateNormal;
        //
        private Grid _yearViewGrid; 

        #endregion Data
 
        public Month(Calendar owner) 
        {
            _owner = owner; 
        }

        #region Public Properties 

        public DayButton CurrentButton
        { 
            get { return this._currentButton; } 
            set { this._currentButton = value; }
        } 

        public Grid MonthView
        { 
            get { return _monthGrid;}
        }
 
        public Grid YearView 
        {
            get { return _yearViewGrid;} 
        }

        #endregion Public Properties 

        #region Protected Properties
        #endregion Protected Properties 
 
        #region Internal Properties
        #endregion Internal Properties 

        #region Private Properties
        #endregion Private Properties 

        #region Public Methods
 
        public void UpdateMonthMode() 
        {
            Debug.Assert(this._owner.DisplayDate != null); 
            _currentMonth = (DateTime)this._owner.DisplayDate;

 
            _firstDayOfWeek = this._owner.FirstDayOfWeek;

            if ( _currentMonth != null) 
            { 
                int year = _calendar.GetYear(_currentMonth);
                int month = _calendar.GetMonth(_currentMonth); 
                DateTime firstDayOfMonth = new DateTime(year, month, 1);

                if (this._headerButton != null) 
                {
                    this._headerButton.Content = string.Format(CultureInfo.CurrentCulture, Resource.Calendar_MonthViewHeaderText,
                         CultureInfo.CurrentCulture.DateTimeFormat.MonthNames[month - 1], 
                         year.ToString(CultureInfo.CurrentCulture)); 

                    this._headerButton.IsEnabled = true; 
                }

                int lastMonthToDisplay = PreviousMonthDays(firstDayOfMonth); 
                DateTime dateToAdd = _calendar.AddDays(firstDayOfMonth, -lastMonthToDisplay);
                DateTime firstDayOfNextMonth = _calendar.AddMonths(firstDayOfMonth, 1);
 
                // check to see if Prev/Next Buttons will be displayed 
                if (_previousButton != null)
                { 
                    if (DateTime.Compare(_owner.DisplayDateStart.GetValueOrDefault(DateTime.MinValue), firstDayOfMonth) > -1)
                    {
                        _previousButton.IsEnabled = false; 
                    }
                    else
                    { 
                        _previousButton.IsEnabled = true; 
                    }
                } 

                if (_nextButton != null)
                { 
                    if (DateTime.Compare(_owner.DisplayDateEnd.GetValueOrDefault(DateTime.MaxValue), firstDayOfNextMonth) < 0)
                    {
                        _nextButton.IsEnabled = false; 
                    } 
                    else
                    { 
                        _nextButton.IsEnabled = true;
                    }
                } 

                int count = 0;
 
                if (_monthGrid != null) 
                {
 
                    Debug.Assert(_monthGrid.Children.Count == COLS * ROWS);
                    //Set the day titles
                    foreach (object child in _monthGrid.Children) 
                    {
                        if (count < (COLS))
                        { 
                            //this assumes that the day titles are always text blocks 
                            TextBlock daytitle = child as TextBlock;
                            Debug.Assert(daytitle != null); 
                            daytitle.Text = CultureInfo.CurrentCulture.DateTimeFormat.AbbreviatedDayNames[(count + (int)_firstDayOfWeek) % NUMBER_OF_DAYS_IN_WEEK];
                        }
                        else 
                        {
                            DayButton mybutton = child as DayButton;
                            Debug.Assert(mybutton != null); 
 
                            SetButtonState(mybutton, dateToAdd);
 
                            mybutton.IsTabStop = false;

                            if (_currentButton == null) 
                            {
                                if (_owner.SelectedDate == null && mybutton.IsToday)
                                { 
                                    mybutton.IsTabStop = true; 
                                    _currentButton = mybutton;
                                    mybutton.IsCurrent = true; 
                                }
                                else
                                { 
                                    if (mybutton.IsSelected)
                                    {
                                        mybutton.IsTabStop = true; 
                                        _currentButton = mybutton; 
                                        mybutton.IsCurrent = true;
                                    } 
                                }
                            }
 
                            mybutton.Content = String.Format(CultureInfo.CurrentCulture, "{0,2}", dateToAdd.Day);
                            mybutton.DataContext = dateToAdd;
                            dateToAdd = _calendar.AddDays(dateToAdd, 1); 
                        } 
                        count++;
                    } 
                }
            }
        } 

        public void UpdateSelectedDate(DateTime? addedDate, DateTime? removedDate)
        { 
            int count = 1; 

            if (addedDate != removedDate && _monthGrid != null) 
            {

                foreach (object child in _monthGrid.Children) 
                {
                    if (count > COLS)
                    { 
                        DayButton mybutton = child as DayButton; 
                        Debug.Assert(mybutton != null);
                        DateTime current = (DateTime)mybutton.DataContext; 
                        if (current != null)
                        {
                            if (addedDate != null) 
                            {
                                if (DateTime.Compare(current, (DateTime)addedDate) == 0)
                                { 
                                    if (_currentButton != null) 
                                    {
                                        _currentButton.IsCurrent = false; 
                                        _currentButton.IsTabStop = false;
                                    }
                                    mybutton.IsSelected = true; 
                                    mybutton.IsTabStop = true;
                                    mybutton.IsCurrent = true;
 
                                    _currentButton = mybutton; 
                                }
                            } 

                            if (removedDate != null)
                            { 
                                if (DateTime.Compare(current, (DateTime)removedDate) == 0)
                                {
                                    mybutton.IsSelected = false; 
                                    mybutton.IsTabStop = false; 
                                    mybutton.IsCurrent = false;
                                } 
                            }
                        }
                    } 
                    count++;
                }
            } 
        } 

        public void UpdateYearMode() 
        {
            Debug.Assert(this._owner.SelectedMonth != null);
            _currentMonth = (DateTime)_owner.SelectedMonth; 

            _firstDayOfWeek = _owner.FirstDayOfWeek;
 
             if (_currentMonth != null) 
            {
                if (this._headerButton != null) 
                {
                    this._headerButton.Content = _currentMonth.Year.ToString(CultureInfo.CurrentCulture);
                    this._headerButton.IsEnabled = false; 
                }

                int count = 0; 
                int year = _calendar.GetYear(_currentMonth); 

                // check to see if Prev/Next Buttons will be displayed 


                if (_previousButton != null) 
                {
                    if (_owner.DisplayDateStart.GetValueOrDefault(DateTime.MinValue).Year == year)
                    { 
                        _previousButton.IsEnabled = false; 
                    }
                    else 
                    {
                        _previousButton.IsEnabled = true;
                    } 
                }

                if (_nextButton != null) 
                { 
                    if (_owner.DisplayDateEnd.GetValueOrDefault(DateTime.MaxValue).Year == year)
                    { 
                        _nextButton.IsEnabled = false;
                    }
                    else 
                    {
                        _nextButton.IsEnabled = true;
                    } 
                } 

                if (_yearViewGrid != null) 
                {
                    foreach (object child in _yearViewGrid.Children)
                    { 
                        MonthButton mybutton = child as MonthButton;
                        Debug.Assert(mybutton != null);
                        DateTime day = new DateTime(year, count + 1, 1, 1, 1, 1); 
                        mybutton.DataContext = day; 
                        mybutton.Content = CultureInfo.CurrentCulture.DateTimeFormat.AbbreviatedMonthNames[count];
 
                        mybutton.Visibility = Visibility.Visible;

                        if (day.Year == _currentMonth.Year && day.Month == _currentMonth.Month && day.Day == _currentMonth.Day) 
                        {
                            mybutton.IsFocusedOverride = true;
                        } 
                        else 
                        {
                            mybutton.IsFocusedOverride = false; 
                        }

                        if (_owner.SelectedDate != null) 
                        {
                            if (CompareYearMonth(day, (DateTime)_owner.SelectedDate) == 0)
                            { 
                                mybutton.IsSelected = true; 
                            }
                            else 
                            {
                                mybutton.IsSelected = false;
                            } 
                        }

                        if (CompareYearMonth(day, _owner.DisplayDateStart.GetValueOrDefault(DateTime.MinValue)) < 0 || CompareYearMonth(day, _owner.DisplayDateEnd.GetValueOrDefault(DateTime.MaxValue)) > 0) 
                        { 
                            mybutton.IsEnabled = false;
                            mybutton.Opacity = 0; 
                        }
                        else
                        { 
                            mybutton.IsEnabled = true;
                            mybutton.Opacity = 1;
                        } 
 
                        count++;
                    } 
                }
            }
        } 

        #endregion Public Methods
 
        #region Protected methods 

        public override void OnApplyTemplate() 
        {
            base.OnApplyTemplate();
 
            _rootGrid = GetTemplateChild("RootGrid") as Grid;

            if (_rootGrid != null) 
            { 
                _rootGrid.SizeChanged += new SizeChangedEventHandler(RootGrid_SizeChanged);
            } 

            _monthGrid = GetTemplateChild("MonthGrid") as Grid;
            _yearViewGrid = GetTemplateChild("YearViewGrid") as Grid; 
            _previousButton = GetTemplateChild(CALENDAR_elementPreviousName) as Button;

            if (this._previousButton != null) 
            { 
                if (_isTopLeftMostMonth)
                { 
                    this._previousButton.Visibility = Visibility.Visible;
                    this._previousButton.MouseLeftButtonUp += new MouseButtonEventHandler(PreviousButton_MouseLeftButtonUp);
                    this._previousButton.IsTabStop = false; 
                }
            }
 
            _nextButton = GetTemplateChild(CALENDAR_elementNextName) as Button; 

            if (this._nextButton != null) 
            {
                if (_isTopRightMostMonth)
                { 
                    this._nextButton.Visibility = Visibility.Visible;
                    this._nextButton.MouseLeftButtonUp += new MouseButtonEventHandler(NextButton_MouseLeftButtonUp);
                    this._nextButton.IsTabStop = false; 
 
                }
            } 

            _headerButton = GetTemplateChild(CALENDAR_elementHeaderName) as Button;
 
            if (this._headerButton != null)
            {
                this._headerButton.MouseLeftButtonUp += new MouseButtonEventHandler(HeaderButton_MouseLeftButtonUp); 
                this._headerButton.IsTabStop = false; 
            }
 
            _backgroundGrid = GetTemplateChild("BackgroundGrid") as Grid;

            if (_backgroundGrid != null) 
            {
                _stateDisabled = _backgroundGrid.Resources[CALENDAR_stateDisabled] as Storyboard;
                _stateNormal = _backgroundGrid.Resources[CALENDAR_stateNormal] as Storyboard; 
            } 

            _dayTitleTemplate = GetTemplateChild("DayTitleTemplate") as DataTemplate; 
            _disabledGrid = GetTemplateChild("DisabledGrid") as Grid;

            //this part should be called after the stateDisabled/stateNormal storyboards are loaded 
            if (this._disabledGrid != null)
            {
                UpdateDisabledGrid(_owner.IsEnabled); 
            } 

            PopulateGrids(); 

            if (_owner.DisplayMode == CalendarMode.Month)
            { 
                _monthGrid.Visibility = Visibility.Visible;
                UpdateMonthMode();
            } 
            else 
            {
                Debug.Assert(_owner.DisplayMode == CalendarMode.Year); 
                _yearViewGrid.Visibility = Visibility.Visible;
                _owner.SelectedMonth = _owner.DisplayDate;
                UpdateYearMode(); 
            }
        }
 
        #endregion Protected Methods 

        #region Internal Methods 

        /*
 


 
 

 


 


 
 

*/ 

        internal static int CompareYearMonth(DateTime dt1, DateTime dt2)
        { 
            return (dt1.Year - dt2.Year) * 12 + (dt1.Month - dt2.Month);
        }
 
        internal void Month_EnterKey(DateTime newmonth) 
        {
            _owner.DisplayDate = newmonth; 
            _owner.DisplayMode = CalendarMode.Month;
        }
 
        internal void UpdateDisabledGrid(bool isEnabled)
        {
            if (_disabledGrid != null) 
            { 
                if (isEnabled)
                { 
                    _disabledGrid.Visibility = Visibility.Collapsed;
                    ChangeVisualState(_stateNormal);
                } 
                else
                {
                    Debug.Assert((bool)isEnabled == false); 
                    _disabledGrid.Visibility = Visibility.Visible; 
                    ChangeVisualState(_stateDisabled);
                } 
            }
        }
 
        #endregion Internal Methods

        #region Private Methods 
        /* 

 


 


 
 

 


 
*/

        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1031:DoNotCatchGeneralExceptionTypes", Justification = "An invalid Storyboard or playing a Storyboard at the wrong time throws System.Exception.")] 
        private void ChangeVisualState(Storyboard storyboard) 
        {
            if ((storyboard != null) && (_currentStoryboard != storyboard)) 
            {
                try
                { 
                    storyboard.Begin();

                    if (_currentStoryboard != null) 
                    { 
                        _currentStoryboard.Stop();
                    } 

                    _currentStoryboard = storyboard;
                } 
                catch (Exception)
                {
                } 
            } 
        }
 
        private void Cell_Click(object sender, RoutedEventArgs e)
        {
            _owner.Focus(); 
            DayButton b = sender as DayButton;
            if (b != null && b.IsEnabled)
            { 
                DateTime selectedDate = (DateTime)b.DataContext; 
                _owner.OnDayClick(selectedDate);
            } 
        }

        private void HeaderButton_MouseLeftButtonUp(object sender, MouseButtonEventArgs e) 
        {
            Button b = sender as Button;
 
            if (b.IsEnabled) 
            {
                if (_owner.DisplayDate != null && _owner.DisplayMode == CalendarMode.Month) 
                {
                    DateTime d = (DateTime)_owner.DisplayDate;
                    _owner.SelectedMonth = new DateTime(d.Year, d.Month, 1); 
                }
                _owner.DisplayMode = CalendarMode.Year;
            } 
        } 

        private void Month_MouseLeftButtonUp(object sender, MouseButtonEventArgs e) 
        {
            DateTime newmonth = (DateTime)((MonthButton)sender).DataContext;
            _owner.DisplayDate = newmonth; 
            _owner.DisplayMode = CalendarMode.Month;
        }
 
        private void NextButton_MouseLeftButtonUp(object sender, MouseButtonEventArgs e) 
        {
                Button b = sender as Button; 

                if (b.IsEnabled)
                { 
                    _owner.OnNextClick();
                }
        } 
 
        private void PopulateGrids()
        { 
            if (_monthGrid != null)
            {
 
                for (int i = 0; i < COLS; i++)
                {
                    if (_dayTitleTemplate != null) 
                    { 
                        FrameworkElement cell = (FrameworkElement)this._dayTitleTemplate.LoadContent();
                        cell.SetValue(Grid.RowProperty, 0); 
                        cell.SetValue(Grid.ColumnProperty, i);
                        this._monthGrid.Children.Add(cell);
                    } 
                }

                for (int i = 1; i < ROWS; i++) 
                { 
                    for (int j = 0; j < COLS; j++)
                    { 
                        DayButton cell = new DayButton();
                        cell.SetValue(Grid.RowProperty, i);
                        cell.SetValue(Grid.ColumnProperty, j); 
                        ((DayButton)cell).Click += new RoutedEventHandler(Cell_Click);
                        if (_owner.DayStyle != null)
                        { 
                            cell.Style = _owner.DayStyle; 
                        }
                        this._monthGrid.Children.Add(cell); 
                    }
                }
            } 

            if (_yearViewGrid != null)
            { 
                MonthButton month; 
                int count = 0;
                for (int i = 0; i < YEAR_ROWS; i++) 
                {
                    for (int j = 0; j < YEAR_COLS; j++)
                    { 
                        month = new MonthButton();
                        month.SetValue(Grid.RowProperty, i);
                        month.SetValue(Grid.ColumnProperty, j); 
                        month.MouseLeftButtonUp += new MouseButtonEventHandler(Month_MouseLeftButtonUp); 

                        if (_owner.MonthStyle != null) 
                        {
                            month.Style = _owner.MonthStyle;
                        } 
                        this._yearViewGrid.Children.Add(month);
                        count++;
                    } 
                } 
            }
        } 

        private void PreviousButton_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        { 
                Button b = sender as Button;
                if (b.IsEnabled)
                { 
                    _owner.OnPreviousClick(); 
                }
        } 

        // How many days of the previous month need to be displayed
        private int PreviousMonthDays(DateTime firstOfMonth) 
        {
            DayOfWeek day = _calendar.GetDayOfWeek(firstOfMonth);
            int i = ((day - _firstDayOfWeek + NUMBER_OF_DAYS_IN_WEEK) % NUMBER_OF_DAYS_IN_WEEK); 
            if (i == 0) 
            {
                return NUMBER_OF_DAYS_IN_WEEK; 
            }
            else
            { 
                return i;
            }
        } 
 
        private void RootGrid_SizeChanged(object sender, SizeChangedEventArgs e)
        { 
            double w = e.NewSize.Width;
            double h = e.NewSize.Height;
            this.Width = w; 
            this.Height = h;
            _owner.OnMonthSizeChanged(h, w);
        } 
 
        private void SetButtonState(DayButton mybutton, DateTime dateToAdd)
        { 
            mybutton.Opacity = 1;

            if (DateTime.Compare(dateToAdd, _owner.DisplayDateStart.GetValueOrDefault(DateTime.MinValue)) < 0 || DateTime.Compare(dateToAdd, _owner.DisplayDateEnd.GetValueOrDefault(DateTime.MaxValue)) > 0) 
            {
                mybutton.IsEnabled = false;
                mybutton.Opacity = 0; 
            } 
            else
            { 
                //SET IF THE DAY IS SELECTABLE OR NOT

                if (_owner.SelectableDateEnd != null || _owner.SelectableDateStart != null) 
                {
                    if (DateTime.Compare(dateToAdd, _owner.SelectableDateEnd.GetValueOrDefault(DateTime.MaxValue)) < 1 &&
                         DateTime.Compare(dateToAdd, _owner.SelectableDateStart.GetValueOrDefault(DateTime.MinValue)) > -1) 
                    { 
                        mybutton.IsDisabled = false;
                        mybutton.IsEnabled = true; 
                    }
                    else
                    { 
                        mybutton.IsDisabled = true;
                        mybutton.IsEnabled = false;
                    } 
                } 
                else
                { 
                    if (_owner.AreDatesInPastSelectable)
                    {
                        mybutton.IsDisabled = false; 
                        mybutton.IsEnabled = true;
                    }
                    else 
                    { 
                        if (DateTime.Compare(dateToAdd, DateTime.Today) > -1)
                        { 
                            mybutton.IsDisabled = false;
                            mybutton.IsEnabled = true;
                        } 
                        else
                        {
                            mybutton.IsDisabled = true; 
                            mybutton.IsEnabled = false; 
                        }
                    } 
                }

                //SET IF THE DAY IS INACTIVE OR NOT 

                if (CompareYearMonth(dateToAdd, (DateTime)_owner.DisplayDate) != 0)
                { 
                    mybutton.IsInactive = true; 
                }
                else 
                {
                    mybutton.IsInactive = false;
                } 

                //SET IF THE DAY IS TODAY OR NOT
 
                if (_owner.IsTodayHighlighted && dateToAdd == DateTime.Today) 
                {
                    mybutton.IsToday = true; 
                }
                else
                { 
                    mybutton.IsToday = false;
                }
 
                //SET IF THE DAY IS SELECTED OR NOT 

                if (DateTime.Compare(dateToAdd, _owner.SelectedDate.GetValueOrDefault(DateTime.MinValue)) == 0) 
                {
                    mybutton.IsSelected = true;
                    mybutton.IsTabStop = true; 
                    mybutton.IsCurrent = true;
                    _currentButton = mybutton;
                } 
                else 
                {
                    mybutton.IsSelected = false; 
                    mybutton.IsTabStop = false;
                    mybutton.IsCurrent = false;
                } 
            }
        }
 
        #endregion Private Methods 
    }
} 
