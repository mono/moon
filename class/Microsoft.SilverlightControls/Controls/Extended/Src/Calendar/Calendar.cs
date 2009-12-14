// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System.Diagnostics;
using System.Globalization; 
using System.Windows.Input; 
using System.Windows.Media;
using System.Windows.Controls;
 
namespace System.Windows.Controlsb1
{
    /// <summary> 
    /// Represents a control that enables a user to select a date by using a visual calendar display.
    /// </summary>
    [TemplatePart(Name = Calendar.CALENDAR_elementRootName, Type = typeof(FrameworkElement))] 
 
    public partial class Calendar : Control
    { 
        #region Constants

        private const string CALENDAR_elementRootName = "RootElement"; 
        private const int COLS = 7;
        private const int ROWS = 7;
        private const int YEAR_ROWS = 3; 
        private const int YEAR_COLS = 4; 

        #endregion Constants 

        #region Data
        private DateTime _displayDate; 
        private DateTime? _displayDateEnd;
        private DateTime? _displayDateStart;
        internal Canvas _months; 
        private ControlTemplate _monthTemplate; 
        private DateTime? _selectedDate;
        private DateTime? _selectedDateEnd; 
        private DateTime? _selectedDateStart;
        private DateTime _selectedMonth;
 
        #endregion Data

        #region Public Events 
        /// <summary> 
        /// Occurs when a date is selected.
        /// </summary> 
        public event EventHandler<CalendarDateChangedEventArgs> DateSelected;

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
            this.IsTabStop = true;
            this.DisplayDate = new DateTime(DateTime.Today.Year, DateTime.Today.Month, 1, 0, 0, 0, 0); 
            this.GotFocus += new RoutedEventHandler(Calendar_GotFocus);
            this.LostFocus += new RoutedEventHandler(Calendar_LostFocus);
            this.FirstDayOfWeek = DayOfWeek.Sunday; 
            this.IsTodayHighlighted = true; 
            this.AreDatesInPastSelectable = true;
            this.IsEnabled = true; 
            this.MouseLeftButtonDown += new MouseButtonEventHandler(Calendar_MouseLeftButtonDown);
        }
 

        #region Public Properties
 
        #region AreDatesInPastSelectable 

        /// <summary> 
        /// Gets or sets a value indicating whether dates before Today are valid selections.
        /// </summary>
        public bool AreDatesInPastSelectable 
        {
            get{ return (bool)GetValue(AreDatesInPastSelectableProperty);}
            set{ SetValue(AreDatesInPastSelectableProperty, value);} 
        } 

        /// <summary> 
        /// Identifies the AreDatesInPastSelectable dependency property.
        /// </summary>
        public static readonly DependencyProperty AreDatesInPastSelectableProperty = 
            DependencyProperty.Register(
            "AreDatesInPastSelectable",
            typeof(bool), 
            typeof(Calendar), 
            new PropertyMetadata (new PropertyChangedCallback(OnAreDatesInPastSelectableChanged)));
 
        /// <summary>
        /// AreDatesInPastSelectableProperty property changed handler.
        /// </summary> 
        /// <param name="d">Calendar that changed its AreDatesInPastSelectable.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnAreDatesInPastSelectableChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) 
        { 
            Calendar c = d as Calendar;
            Debug.Assert(c != null); 
            bool b = (bool)e.NewValue;

            if (c.SelectableDateStart == null && c.SelectableDateEnd == null) 
            {
                if (c.SelectedDate != null)
                { 
                    if (DateTime.Compare(DateTime.Today, (DateTime)c.SelectedDate) > 0 && b == false) 
                    {
                        throw new ArgumentException(Resource.Calendar_OnAreDatesInPastSelectableChanged_InvalidValue); 
                    }
                }
 
                c.UpdateMonths();
            }
            // If SelectableDateStart/End are set, do nothing. 
        } 
        #endregion AreDatesInPastSelectable
 
        #region DayStyle

        /// <summary> 
        /// Gets or sets the style for displaying a day.
        /// </summary>
        public Style DayStyle 
        { 
            get { return (Style)GetValue(DayStyleProperty); }
            set { SetValue(DayStyleProperty, value); } 
        }

        /// <summary> 
        /// Identifies the DayStyle dependency property.
        /// </summary>
        public static readonly DependencyProperty DayStyleProperty = 
            DependencyProperty.Register( 
            "DayStyle",
            typeof(Style), 
            typeof(Calendar),
            new PropertyMetadata (new PropertyChangedCallback(OnDayStyleChanged)));
 
        /// <summary>
        /// DayStyleProperty property changed handler.
        /// </summary> 
        /// <param name="d">Calendar that changed its DayStyle.</param> 
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnDayStyleChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) 
        {
            Style newStyle = e.NewValue as Style;
            if (newStyle != null) 
            {
                Calendar c = d as Calendar;
 
                if (c != null) 
                {
                    Style oldStyle = e.OldValue as Style; 

                    // Set the style for the days if it has not already been set
                    Month monthControl = c.MonthControl; 
                    int count = ROWS * COLS;
                    if (monthControl != null)
                    { 
                        if (monthControl.MonthView != null) 
                        {
                            for (int childIndex = ROWS; childIndex < count; childIndex++) 
                            {
                                EnsureDayStyle(monthControl.MonthView.Children[childIndex] as DayButton, oldStyle, newStyle);
                            } 
                        }
                    }
                } 
            } 
        }
 
        #endregion DayStyle

        #region DisplayDate 

        /// <summary>
        /// Gets or sets the date to display. 
        /// </summary> 
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
            new PropertyMetadata (new PropertyChangedCallback(OnDisplayDateChanged)));

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

            c._displayDate = DiscardDayTime(addedDate);
 
            if ((Month.CompareYearMonth(c._displayDate, c.DisplayDateStart.GetValueOrDefault(DateTime.MinValue)) > -1 &&
                  Month.CompareYearMonth(c._displayDate, c.DisplayDateEnd.GetValueOrDefault(DateTime.MaxValue)) < 1))
            { 
                c.UpdateMonths(); 
                c.OnDisplayDate(new CalendarDateChangedEventArgs(removedDate, addedDate));
            } 
            else
            {
                throw new ArgumentOutOfRangeException("d", Resource.Calendar_OnDisplayDateChanged_InvalidValue); 
            }
        }
 
        #endregion DisplayDate 

        #region DisplayDateEnd 

        /// <summary>
        /// Gets or sets the last date to be displayed. 
        /// </summary>
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
            new PropertyMetadata (new PropertyChangedCallback(OnDisplayDateEndChanged)));

        /// <summary> 
        /// DisplayDateEndProperty property changed handler.
        /// </summary>
        /// <param name="d">Calendar that changed its DisplayDateEnd.</param> 
        /// <param name="e">DependencyPropertyChangedEventArgs.</param> 
        private static void OnDisplayDateEndChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        { 
            Calendar c = d as Calendar;
            Debug.Assert(c != null);
 
            c._displayDateEnd = DiscardTime((DateTime?)e.NewValue);

            if (IsValidDisplayDateEndProperty(c, c._displayDateEnd)) 
            { 
                if (e.NewValue == null || c.DisplayMode == CalendarMode.Year)
                { 
                    c.UpdateMonths();
                }
                else 
                {
                    if (c.DisplayDate != null)
                    { 
                        DateTime newRange = (DateTime)e.NewValue; 
                        // if new DisplayDateEnd is on the DisplayDate
 
                        int i = Month.CompareYearMonth(newRange, (DateTime)c.DisplayDate);

                        if (i > -2 && i < 2) 
                        {
                            c.UpdateMonths();
                        } 
                    } 
                }
            } 
            else
            {
                throw new ArgumentOutOfRangeException("d", Resource.Calendar_OnDisplayDateEndChanged_InvalidValue); 
            }
        }
 
        #endregion DisplayDateEnd 

        #region DisplayDateStart 

        /// <summary>
        /// Gets or sets the first date to be displayed. 
        /// </summary>
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
            new PropertyMetadata (new PropertyChangedCallback(OnDisplayDateStartChanged)));

        /// <summary> 
        /// DisplayDateStartProperty property changed handler.
        /// </summary>
        /// <param name="d">Calendar that changed its DisplayDateStart.</param> 
        /// <param name="e">DependencyPropertyChangedEventArgs.</param> 
        private static void OnDisplayDateStartChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        { 
            Calendar c = d as Calendar;
            Debug.Assert(c != null);
 
            c._displayDateStart = DiscardTime((DateTime?)e.NewValue);

            if (IsValidDisplayDateStartProperty(c, c._displayDateStart)) 
            { 
                if (e.NewValue == null || c.DisplayMode == CalendarMode.Year)
                { 
                    c.UpdateMonths();
                }
                else 
                {
                    if (c.DisplayDate != null)
                    { 
                        DateTime newRange = (DateTime)e.NewValue; 
                        // if new DisplayDateStart is on the DisplayDate
 
                        int i = Month.CompareYearMonth(newRange, (DateTime)c.DisplayDate);

                        if (i > -2 && i < 2) 
                        {
                            c.UpdateMonths();
                        } 
                    } 
                }
            } 
            else
            {
                throw new ArgumentOutOfRangeException("d", Resource.Calendar_OnDisplayDateStartChanged_InvalidValue); 
            }

        } 
 
        #endregion DisplayDateStart
 
        #region DisplayMode

        /// <summary> 
        /// Gets or sets a value indicating whether the calendar is displayed in months or years.
        /// </summary>
        public CalendarMode DisplayMode 
        { 
            get{ return (CalendarMode)GetValue(DisplayModeProperty);}
            set{ SetValue(DisplayModeProperty, value);} 
        }

        /// <summary> 
        /// Identifies the DisplayMode dependency property.
        /// </summary>
        public static readonly DependencyProperty DisplayModeProperty = 
            DependencyProperty.Register( 
            "DisplayMode",
            typeof(CalendarMode), 
            typeof(Calendar),
            new PropertyMetadata (new PropertyChangedCallback(OnDisplayModePropertyChanged)));
 
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
            Month monthControl = c.MonthControl;
 
            if (monthControl != null) 
            {
                if (mode == CalendarMode.Month) 
                {
                    c.OnMonthClick();
                    c.OnDisplayModeChanged(new CalendarModeChangedEventArgs(CalendarMode.Year, CalendarMode.Month)); 
                }
                else
                { 
                    Debug.Assert(mode == CalendarMode.Year); 
                    c.OnHeaderClick();
                    c.OnDisplayModeChanged(new CalendarModeChangedEventArgs(CalendarMode.Month, CalendarMode.Year)); 
                }
            }
        } 

        #endregion DisplayMode
 
        #region FirstDayOfWeek 

        /// <summary> 
        /// Gets or sets the day that is considered the beginning of the week.
        /// </summary>
        public DayOfWeek FirstDayOfWeek 
        {
            get{ return (DayOfWeek)GetValue(FirstDayOfWeekProperty);}
            set{ SetValue(FirstDayOfWeekProperty, value);} 
        } 

        /// <summary> 
        /// Identifies the FirstDayOfWeek dependency property.
        /// </summary>
        public static readonly DependencyProperty FirstDayOfWeekProperty = 
            DependencyProperty.Register(
            "FirstDayOfWeek",
            typeof(DayOfWeek), 
            typeof(Calendar), 
            new PropertyMetadata (new PropertyChangedCallback(OnFirstDayOfWeekChanged)));
 
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
        /// Gets or sets a value indicating whether this calendar is enabled in the user interface (UI).
        /// </summary> 
        public bool IsEnabled
        {
            get { return (bool)GetValue(IsEnabledProperty); } 
            set { SetValue(IsEnabledProperty, value); }
        }
 
        /// <summary> 
        /// Identifies the IsEnabled dependency property.
        /// </summary> 
        public static readonly DependencyProperty IsEnabledProperty =
            DependencyProperty.Register(
            "IsEnabled", 
            typeof(bool),
            typeof(Calendar),
            new PropertyMetadata (new PropertyChangedCallback(OnIsEnabledPropertyChanged))); 
 
        /// <summary>
        /// IsEnabledProperty property changed handler. 
        /// </summary>
        /// <param name="d">Calendar that changed its IsEnabled.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param> 
        private static void OnIsEnabledPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            Calendar c = d as Calendar; 
            Debug.Assert(c != null); 
            Month monthControl = c.MonthControl;
 
            if (monthControl != null)
            {
                monthControl.UpdateDisabledGrid((bool)e.NewValue); 
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
            new PropertyMetadata (new PropertyChangedCallback(OnIsTodayHighlightedChanged)));
 
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
                int i = Month.CompareYearMonth((DateTime)c.DisplayDate, DateTime.Today); 

                if (i > -2 && i < 2) 
                {
                    c.UpdateMonths();
                } 
            }
        }
 
        #endregion IsTodayHighlighted 

        #region MonthStyle 

        /// <summary>
        /// Gets or sets the style for displaying a month. 
        /// </summary>
        public Style MonthStyle
        { 
            get { return (Style)GetValue(MonthStyleProperty); } 
            set { SetValue(MonthStyleProperty, value); }
        } 

        /// <summary>
        /// Identifies the MonthStyle dependency property. 
        /// </summary>
        public static readonly DependencyProperty MonthStyleProperty =
            DependencyProperty.Register( 
            "MonthStyle", 
            typeof(Style),
            typeof(Calendar), 
            new PropertyMetadata (new PropertyChangedCallback(OnMonthStyleChanged)));

        /// <summary> 
        /// MonthStyleProperty property changed handler.
        /// </summary>
        /// <param name="d">Calendar that changed its MonthStyle.</param> 
        /// <param name="e">DependencyPropertyChangedEventArgs.</param> 
        private static void OnMonthStyleChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        { 
            Style newStyle = e.NewValue as Style;
            if (newStyle != null)
            { 
                Calendar c = d as Calendar;

                if (c != null) 
                { 
                    Style oldStyle = e.OldValue as Style;
 
                    // Set the style for the month button if it has not already been set
                    Month monthControl = c.MonthControl;
                    int count = YEAR_COLS * YEAR_ROWS; 
                    if (monthControl != null)
                    {
                        if (monthControl.YearView != null) 
                        { 
                            for (int childIndex = 0; childIndex < count; childIndex++)
                            { 
                                EnsureMonthStyle(monthControl.YearView.Children[childIndex] as MonthButton, oldStyle, newStyle);
                            }
                        } 
                    }

                } 
            } 
        }
 
        #endregion MonthStyle

        #region SelectedDate 

        /// <summary>
        /// Gets or sets the currently selected date. 
        /// </summary> 
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
            new PropertyMetadata (new PropertyChangedCallback(OnSelectedDateChanged)));

        /// <summary> 
        /// SelectedDateProperty property changed handler. 
        /// </summary>
        /// <param name="d">Calendar that changed its SelectedDate.</param> 
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnSelectedDateChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        { 
            Calendar c = d as Calendar;
            Debug.Assert(c != null);
            DateTime? removedDate, addedDate; 
 
            addedDate = (DateTime?)e.NewValue;
            removedDate = (DateTime?)e.OldValue; 

            c._selectedDate = DiscardTime(addedDate);
 
            if (IsValidDate(c, c._selectedDate))
            {
                c.UpdateSelectedDate(addedDate, removedDate); 
                c.OnDateSelected(new CalendarDateChangedEventArgs(removedDate, addedDate)); 
            }
            else 
            {
                throw new ArgumentOutOfRangeException("d", Resource.Calendar_OnSelectedDateChanged_InvalidValue);
            } 
        }

        #endregion SelectedDate 
 
        #region SelectableDateEnd
 
        /// <summary>
        /// Gets or sets the last date that can be selected.
        /// </summary> 
        public DateTime? SelectableDateEnd
        {
            get { return (DateTime?)GetValue(SelectableDateEndProperty); } 
            set { SetValue(SelectableDateEndProperty, value); } 
        }
 
        /// <summary>
        /// Identifies the SelectableDateEnd dependency property.
        /// </summary> 
        public static readonly DependencyProperty SelectableDateEndProperty =
            DependencyProperty.Register(
            "SelectableDateEnd",
            typeof(DateTime?),
            typeof(Calendar),
            new PropertyMetadata(new PropertyChangedCallback(OnSelectableDateEndChanged)));

        /// <summary>
        /// SelectableDateEndProperty property changed handler. 
        /// </summary>
        /// <param name="d">Calendar that changed its SelectableDateEnd.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param> 
        private static void OnSelectableDateEndChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) 
        {
            Calendar c = d as Calendar; 
            Debug.Assert(c != null);

 
            c._selectedDateEnd = DiscardTime((DateTime?)e.NewValue);

            if (IsValidSelectableDateEnd(c, c._selectedDateEnd)) 
            { 
                c.UpdateMonths();
            } 
            else
            {
                throw new ArgumentOutOfRangeException("d", Resource.Calendar_OnSelectableDateEndChanged_InvalidValue); 
            }
        }
 
        #endregion SelectableDateEnd 

        #region SelectableDateStart 

        /// <summary>
        /// Gets or sets the first date that can be selected. 
        /// </summary>
        public DateTime? SelectableDateStart
        { 
            get { return (DateTime?)GetValue(SelectableDateStartProperty); } 
            set { SetValue(SelectableDateStartProperty, value); }
        } 

        /// <summary>
        /// Identifies the SelectableDateStart dependency property. 
        /// </summary>
        public static readonly DependencyProperty SelectableDateStartProperty =
            DependencyProperty.Register( 
            "SelectableDateStart", 
            typeof(DateTime?),
            typeof(Calendar), 
            new PropertyMetadata (new PropertyChangedCallback(OnSelectableDateStartChanged)));

        /// <summary> 
        /// SelectableDateStartProperty property changed handler.
        /// </summary>
        /// <param name="d">Calendar that changed its SelectableDateStart.</param> 
        /// <param name="e">DependencyPropertyChangedEventArgs.</param> 
        private static void OnSelectableDateStartChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        { 
            Calendar c = d as Calendar;
            Debug.Assert(c != null);
 
            c._selectedDateStart = DiscardTime((DateTime?)e.NewValue);

            if (IsValidSelectableDateStart(c, c._selectedDateStart)) 
            { 
                c.UpdateMonths();
            } 
            else
            {
                throw new ArgumentOutOfRangeException("d", Resource.Calendar_OnSelectableDateStartChanged_InvalidValue); 
            }
        }
 
        #endregion SelectableDateStart 

        #endregion Public Properties 

        #region Protected Properties
        #endregion Protected Properties 

        #region Internal Properties
 
        internal DateTime SelectedMonth 
        {
            get 
            {
                return this._selectedMonth;
            } 
            set
            {
                int i = Month.CompareYearMonth(value, this.DisplayDateStart.GetValueOrDefault(DateTime.MinValue)); 
                int j = Month.CompareYearMonth(value, this.DisplayDateEnd.GetValueOrDefault(DateTime.MaxValue)); 

                if (i > -1 && j < 1) 
                {
                    this._selectedMonth = value;
                } 
                else
                {
                    if (i < 0) 
                    { 
                        this._selectedMonth = DiscardDayTime(this.DisplayDateStart.GetValueOrDefault(DateTime.MinValue));
                    } 
                    else
                    {
                        Debug.Assert(j > 0); 
                        this._selectedMonth = DiscardDayTime(this.DisplayDateEnd.GetValueOrDefault(DateTime.MaxValue));
                    }
                } 
 
            }
        } 

        #endregion Internal Properties
 
        #region Private Properties

        private Month MonthControl 
        { 
            get
            { 
                if (this._months != null && this._months.Children.Count > 0)
                {
                    return this._months.Children[0] as Month; 
                }
                return null;
            } 
        } 

        #endregion Private Properties 

        #region Public Methods
 
        /// <summary>
        /// Gets the value of any of the Calendar object's dependency properties.
        /// </summary> 
        /// <param name="dp">The dependency property to get the value of.</param> 
        /// <returns>The value of the given dependency property.</returns>
        public override object GetValue(DependencyProperty dp) 
        {
            if (dp == Calendar.DisplayDateProperty)
            { 
                return this._displayDate;
            }
 
            if (dp == Calendar.DisplayDateStartProperty) 
            {
                return this._displayDateStart; 
            }

            if (dp == Calendar.DisplayDateEndProperty) 
            {
                return this._displayDateEnd;
            } 
 
            if (dp == Calendar.SelectedDateProperty)
            { 
                return this._selectedDate;
            }
 
            if (dp == Calendar.SelectableDateStartProperty)
            {
                return this._selectedDateStart; 
            } 

            if (dp == Calendar.SelectableDateEndProperty) 
            {
                return this._selectedDateEnd;
            } 

            return base.GetValue(dp);
        } 
 
        /// <summary>
        /// Provides a text representation of the selected date. 
        /// </summary>
        /// <returns>A text representation of the selected date, or an empty string if SelectedDate is a null reference.</returns>
        public override string ToString() 
        {
            if (this.SelectedDate != null)
            { 
                return ((DateTime)this.SelectedDate).ToString(); 
            }
            else 
            {
                return string.Empty;
            } 
        }

        #endregion Public Methods 
 
        #region Protected Methods
 
        /// <summary>
        /// Invoked whenever application code or an internal process,
        /// such as a rebuilding layout pass, calls the ApplyTemplate method. 
        /// </summary>
        public override void OnApplyTemplate()
        { 
            base.OnApplyTemplate(); 

            _months = GetTemplateChild(CALENDAR_elementRootName) as Canvas; 

            if (_months != null)
            { 
                _monthTemplate = _months.Resources["MonthTemplate"] as ControlTemplate;

                Month month = new Month(this); 
 
                if (_monthTemplate != null)
                { 
                    month.Template = _monthTemplate;
                }
                // 
                _months.Children.Add(month);
            }
 
 
            this.SizeChanged += new SizeChangedEventHandler(Calendar_SizeChanged);
            this.KeyDown += new KeyEventHandler(Calendar_KeyDown); 

        }
 
        #endregion Protected Methods

        #region Internal Methods 
 
        internal void OnDayClick(DateTime selectedDate)
        { 
            Debug.Assert(this.DisplayMode == CalendarMode.Month);
            this.SelectedDate = selectedDate;
 
            int i = Month.CompareYearMonth(selectedDate, (DateTime)this.DisplayDate);
            if (i > 0)
            { 
                OnNextClick(); 
            }
            else if (i < 0) 
            {
                OnPreviousClick();
            } 
        }

        internal void OnHeaderClick() 
        { 
            if (this.DisplayMode == CalendarMode.Year)
            { 
                Month monthControl = this.MonthControl;
                if (monthControl != null)
                { 
                    double w = monthControl.MonthView.ActualWidth;
                    double h = monthControl.MonthView.ActualHeight;
                    monthControl.MonthView.Visibility = Visibility.Collapsed; 
                    monthControl.YearView.Width = w; 
                    monthControl.YearView.Height = h;
                    monthControl.YearView.Visibility = Visibility.Visible; 
                    this.UpdateMonths();
                }
            } 
        }

        internal void OnMonthClick() 
        { 
            Month monthControl = this.MonthControl;
            if (monthControl != null) 
            {
                double w = monthControl.YearView.ActualWidth;
                double h = monthControl.YearView.ActualHeight; 
                monthControl.YearView.Visibility = Visibility.Collapsed;
                monthControl.MonthView.Width = w;
                monthControl.MonthView.Height = h; 
                monthControl.MonthView.Visibility = Visibility.Visible; 
                this.UpdateMonths();
            } 
        }

        internal void OnMonthSizeChanged(double height, double width) 
        {
            if (_months != null)
            { 
                _months.Height = height; 
                _months.Width = width;
            } 

            if (this.Height == 0.0 && this.Width == 0.0)
            { 
                this.Width = width;
                this.Height = height;
            } 
        } 

        internal void OnNextClick() 
        {
            if (this.DisplayMode == CalendarMode.Month && this.DisplayDate != null)
            { 
                /*

 
 

*/ 
                this.DisplayDate = ((DateTime)this.DisplayDate).AddMonths(1);
            }
            else 
            {
                Debug.Assert(this.DisplayMode == CalendarMode.Year);
                /* 
 

 

*/
 
                this.SelectedMonth = new DateTime((this.SelectedMonth.AddYears(1)).Year, 1, 1);
                UpdateMonths();
            } 
        } 

        internal void OnPreviousClick() 
        {
            if (this.DisplayMode == CalendarMode.Month && this.DisplayDate != null)
            { 
                /*

 
 

*/ 
                this.DisplayDate = ((DateTime)this.DisplayDate).AddMonths(-1);
            }
            else 
            {
                Debug.Assert(this.DisplayMode == CalendarMode.Year);
                /* 
 

 

*/
                this.SelectedMonth = new DateTime((this.SelectedMonth.AddYears(-1)).Year, 1, 1); 
                UpdateMonths();

            } 
        } 

        internal void OnSelectedMonthChanged(DateTime selectedMonth) 
        {
                Debug.Assert(this.DisplayMode == CalendarMode.Year);
 
                /*

 
 

 


*/ 

                this.SelectedMonth = selectedMonth;
                UpdateMonths(); 
        } 

        internal void ResetStates() 
        {
            DayButton d;
            Month monthControl = this.MonthControl; 
            int count = ROWS * COLS;
            if (monthControl != null)
            { 
                if (monthControl.MonthView != null) 
                {
                    for (int childIndex = ROWS; childIndex < count; childIndex++) 
                    {
                        d = monthControl.MonthView.Children[childIndex] as DayButton;
                        d.IsMouseOverOverride = false; 
                    }
                }
            } 
        } 

        #endregion Internal Methods 

        #region Private Methods
 
        private void Calendar_GotFocus(object sender, RoutedEventArgs e)
        {
            Calendar c = sender as Calendar; 
            Debug.Assert(c != null); 

            Month monthControl = c.MonthControl; 

            if (monthControl != null && this.IsEnabled)
            { 
                if (c.DisplayMode == CalendarMode.Month)
                {
                    if (monthControl.CurrentButton != null) 
                    { 
                        ((DayButton)monthControl.CurrentButton).IsCurrent = true;
                    } 
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
 
        private void Calendar_LostFocus(object sender, RoutedEventArgs e)
        { 
            Calendar c = sender as Calendar;
            Debug.Assert(c != null);
 
            Month monthControl = c.MonthControl;

            if (monthControl != null && monthControl.CurrentButton != null) 
            { 
                ((DayButton)monthControl.CurrentButton).IsCurrent = false;
            } 
        }

        private void Calendar_MouseLeftButtonDown(object sender, MouseButtonEventArgs e) 
        {
            if (!e.Handled)
            { 
                this.Focus(); 
            }
        } 

        private void Calendar_SizeChanged(object sender, SizeChangedEventArgs e)
        { 
            Debug.Assert(sender is Calendar);

            RectangleGeometry rg = new RectangleGeometry(); 
            rg.Rect = new Rect(0, 0, e.NewSize.Width, e.NewSize.Height); 

            if (_months != null) 
            {
                _months.Clip = rg;
            } 
        }

        private static DateTime DiscardDayTime(DateTime d) 
        { 
            int year = d.Year;
            int month = d.Month; 
            DateTime newD = new DateTime(year, month, 1, 0, 0, 0);
            return newD;
        } 

        private static DateTime? DiscardTime(DateTime? d)
        { 
            if (d == null) 
            {
                return null; 
            }
            else
            { 
                DateTime discarded = (DateTime)d;
                int year = discarded.Year;
                int month = discarded.Month; 
                int day = discarded.Day; 
                DateTime newD = new DateTime(year, month, day, 0, 0, 0);
                return newD; 
            }
        }
 
        private static void EnsureDayStyle(DayButton day, Style oldCalendarDayStyle, Style newCalendarDayStyle)
        {
            Debug.Assert(day != null); 
 
            if (newCalendarDayStyle != null)
            { 
                //

                if (day != null && (day.Style == null || day.Style == oldCalendarDayStyle)) 
                {
                    day.Style = newCalendarDayStyle;
                } 
            } 
        }
 
        private static void EnsureMonthStyle(MonthButton month, Style oldCalendarMonthStyle, Style newCalendarMonthStyle)
        {
            Debug.Assert(month != null); 

            if (newCalendarMonthStyle != null)
            { 
                // 

                if (month != null && (month.Style == null || month.Style == oldCalendarMonthStyle)) 
                {
                    month.Style = newCalendarMonthStyle;
                } 
            }
        }
 
        private static bool IsValidDate(Calendar cal, object value) 
        {
            if (value == null) 
            {
                return true;
            } 
            else
            {
 
                if (cal.SelectableDateEnd != null || cal.SelectableDateStart != null) 
                {
                    return (DateTime.Compare((DateTime)value, cal.DisplayDateStart.GetValueOrDefault(DateTime.MinValue)) > -1 && 
                           DateTime.Compare((DateTime)value, cal.DisplayDateEnd.GetValueOrDefault(DateTime.MaxValue)) < 1 &&
                           DateTime.Compare((DateTime)value, cal.SelectableDateEnd.GetValueOrDefault(DateTime.MaxValue)) < 1 &&
                           DateTime.Compare((DateTime)value, cal.SelectableDateStart.GetValueOrDefault(DateTime.MinValue)) > -1); 

                }
                else 
                { 
                    if (cal.AreDatesInPastSelectable)
                    { 
                        return (DateTime.Compare((DateTime)value, cal.DisplayDateStart.GetValueOrDefault(DateTime.MinValue)) > -1 &&
                               DateTime.Compare((DateTime)value, cal.DisplayDateEnd.GetValueOrDefault(DateTime.MaxValue)) < 1);
                    } 
                    else
                    {
                        return (DateTime.Compare((DateTime)value, cal.DisplayDateStart.GetValueOrDefault(DateTime.MinValue)) > -1 && 
                                    DateTime.Compare((DateTime)value, cal.DisplayDateEnd.GetValueOrDefault(DateTime.MaxValue)) < 1 && 
                                    DateTime.Compare((DateTime)value, DateTime.Today) > -1);
                    } 
                }
            }
        } 

        private static bool IsValidDisplayDateEndProperty(Calendar cal, object value)
        { 
            if (value == null) 
            {
                return true; 
            }
            else
            { 
                if (cal.DisplayDate != null)
                {
                    if (Month.CompareYearMonth((DateTime)value, (DateTime)cal.DisplayDate) < 0) 
                    { 
                        return false;
                    } 
                }

                return (DateTime.Compare((DateTime)value, cal.DisplayDateStart.GetValueOrDefault(DateTime.MinValue)) > -1 && 
                        DateTime.Compare((DateTime)value, cal.SelectedDate.GetValueOrDefault(DateTime.MinValue)) > -1 &&
                        DateTime.Compare((DateTime)value, cal.SelectableDateStart.GetValueOrDefault(DateTime.MinValue)) > -1 &&
                        DateTime.Compare((DateTime)value, cal.SelectableDateEnd.GetValueOrDefault(DateTime.MinValue)) > -1); 
 
            }
        } 

        private static bool IsValidDisplayDateStartProperty(Calendar cal, object value)
        { 
            if (value == null)
            {
                return true; 
            } 
            else
            { 
                if (cal.DisplayDate != null)
                {
                    if (Month.CompareYearMonth((DateTime)value, cal.DisplayDate) > 0) 
                    {
                        return false;
                    } 
                } 

                return (DateTime.Compare((DateTime)value, cal.DisplayDateEnd.GetValueOrDefault(DateTime.MaxValue)) < 1 && 
                        DateTime.Compare((DateTime)value, cal.SelectedDate.GetValueOrDefault(DateTime.MaxValue)) < 1 &&
                        DateTime.Compare((DateTime)value, cal.SelectableDateStart.GetValueOrDefault(DateTime.MaxValue)) < 1 &&
                        DateTime.Compare((DateTime)value, cal.SelectableDateEnd.GetValueOrDefault(DateTime.MaxValue)) < 1); 

            }
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

        private static bool IsValidSelectableDateEnd(Calendar cal, object value) 
        {
            if (value == null)
            { 
                return true; 
            }
            else 
            {
                if (cal.SelectedDate != null)
                { 
                    if (DateTime.Compare((DateTime)value, (DateTime)cal.SelectedDate) < 0)
                    {
                        return false; 
                    } 
                }
 
                return (DateTime.Compare((DateTime)value, cal.DisplayDateStart.GetValueOrDefault(DateTime.MinValue)) > -1 &&
                        DateTime.Compare((DateTime)value, cal.DisplayDateEnd.GetValueOrDefault(DateTime.MaxValue)) < 1 &&
                        DateTime.Compare((DateTime)value, cal.SelectableDateStart.GetValueOrDefault(DateTime.MinValue)) > -1); 
            }

        } 
 
        private static bool IsValidSelectableDateStart(Calendar cal, object value)
        { 
            if (value == null)
            {
                return true; 
            }
            else
            { 
                if (cal.SelectedDate != null) 
                {
                    if (DateTime.Compare((DateTime)value, (DateTime)cal.SelectedDate) > 0) 
                    {
                        return false;
                    } 
                }

                return (DateTime.Compare((DateTime)value, cal.DisplayDateStart.GetValueOrDefault(DateTime.MinValue)) > -1 && 
                           DateTime.Compare((DateTime)value, cal.DisplayDateEnd.GetValueOrDefault(DateTime.MaxValue)) < 1 && 
                           DateTime.Compare((DateTime)value, cal.SelectableDateEnd.GetValueOrDefault(DateTime.MaxValue)) < 1);
            } 

        }
 
        private void OnDateSelected(CalendarDateChangedEventArgs e)
        {
            EventHandler<CalendarDateChangedEventArgs> handler = this.DateSelected; 
            if (null != handler) 
            {
                handler(this, e); 
            }
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

        private bool ProcessCalendarKey(KeyEventArgs e)
        { 
            if (this.DisplayMode == CalendarMode.Month) 
            {
                if (this.SelectedDate != null && this.DisplayDate != null) 
                {
                    if (Month.CompareYearMonth((DateTime)this.SelectedDate, (DateTime)this.DisplayDate) != 0)
                    { 
                        return true;
                    }
                } 
            } 
            switch (e.Key)
            { 
                case Key.Up:
                    ProcessUpKey();
                    return true; 

                case Key.Down:
                    ProcessDownKey(); 
                    return true; 

                case Key.Left: 
                    ProcessLeftKey();
                    return true;
 
                case Key.Right:
                    ProcessRightKey();
                    return true; 
 
                case Key.PageDown:
                    ProcessPageDownKey(); 
                    return true;

                case Key.PageUp: 
                    ProcessPageUpKey();
                    return true;
 
                case Key.Home: 
                    ProcessHomeKey();
                    return true; 

                case Key.End:
                    ProcessEndKey(); 
                    return true;

                //
                //
                case Key.Enter:
                    return ProcessEnterKey(); 
            }
            return false;
        } 

        private void ProcessDownKey()
        { 
            System.Globalization.Calendar _cal = new GregorianCalendar(); 
            if (this.DisplayMode == CalendarMode.Month)
            { 
                DateTime selectedDate = _cal.AddDays(this.SelectedDate.GetValueOrDefault(DateTime.Today), 7);
                if (IsValidDate(this, selectedDate))
                { 
                    OnDayClick(selectedDate);
                }
            } 
            else 
            {
                Debug.Assert(this.DisplayMode == CalendarMode.Year); 
                DateTime selectedMonth = _cal.AddMonths(this._selectedMonth, 4);
                OnSelectedMonthChanged(selectedMonth);
            } 
        }

        private void ProcessEndKey() 
        { 
            System.Globalization.Calendar _cal = new GregorianCalendar();
            if (this.DisplayMode == CalendarMode.Month) 
            {
                if (this.DisplayDate != null)
                { 
                    DateTime selectedDate = new DateTime(((DateTime)this.DisplayDate).Year, ((DateTime)this.DisplayDate).Month, 1);
                    selectedDate = _cal.AddMonths(selectedDate, 1);
                    selectedDate = _cal.AddDays(selectedDate, -1); 
                    if (IsValidDate(this, selectedDate)) 
                    {
                        OnDayClick(selectedDate); 
                    }
                }
            } 
            else
            {
                Debug.Assert(this.DisplayMode == CalendarMode.Year); 
                DateTime selectedMonth = new DateTime(this._selectedMonth.Year, 12, 1); 
                OnSelectedMonthChanged(selectedMonth);
            } 
        }

        private bool ProcessEnterKey() 
        {
            if (this.DisplayMode == CalendarMode.Year)
            { 
                Month monthControl = this.MonthControl; 
                if (monthControl != null)
                { 
                    monthControl.Month_EnterKey(this._selectedMonth);
                }
                return true; 
            }
            else
            { 
                return false; 
            }
        } 

        private void ProcessHomeKey()
        { 
            if (this.DisplayMode == CalendarMode.Month)
            {
                if (this.DisplayDate != null) 
                { 
                    DateTime selectedDate = new DateTime(((DateTime)this.DisplayDate).Year, ((DateTime)this.DisplayDate).Month, 1);
                    if (IsValidDate(this, selectedDate)) 
                    {
                        OnDayClick(selectedDate);
                    } 
                }
            }
            else 
            { 
                Debug.Assert(this.DisplayMode == CalendarMode.Year);
                DateTime selectedMonth = new DateTime(this._selectedMonth.Year, 1, 1); 
                OnSelectedMonthChanged(selectedMonth);
            }
        } 

        private void ProcessLeftKey()
        { 
            System.Globalization.Calendar _cal = new GregorianCalendar(); 
            if (this.DisplayMode == CalendarMode.Month)
            { 
                DateTime selectedDate = _cal.AddDays(this.SelectedDate.GetValueOrDefault(DateTime.Today), -1);
                if (IsValidDate(this, selectedDate))
                { 
                    OnDayClick(selectedDate);
                }
            } 
            else 
            {
                Debug.Assert(this.DisplayMode == CalendarMode.Year); 
                DateTime selectedMonth = _cal.AddMonths(this._selectedMonth, -1);
                OnSelectedMonthChanged(selectedMonth);
            } 
        }

        private void ProcessPageDownKey() 
        { 
            System.Globalization.Calendar _cal = new GregorianCalendar();
            if (this.DisplayMode == CalendarMode.Month) 
            {
                DateTime selectedDate = _cal.AddMonths(this.SelectedDate.GetValueOrDefault(DateTime.Today), 1);
                if (IsValidDate(this,selectedDate)) 
                {
                    OnDayClick(selectedDate);
                } 
            } 
            else
            { 
                Debug.Assert(this.DisplayMode == CalendarMode.Year);
                DateTime selectedMonth = _cal.AddYears(this._selectedMonth, 1);
                OnSelectedMonthChanged(selectedMonth); 
            }
        }
 
        private void ProcessPageUpKey() 
        {
            System.Globalization.Calendar _cal = new GregorianCalendar(); 
            if (this.DisplayMode == CalendarMode.Month)
            {
                DateTime selectedDate = _cal.AddMonths(this.SelectedDate.GetValueOrDefault(DateTime.Today), -1); 
                if (IsValidDate(this,selectedDate))
                {
                    OnDayClick(selectedDate); 
                } 
            }
            else 
            {
                Debug.Assert(this.DisplayMode == CalendarMode.Year);
                DateTime selectedMonth = _cal.AddYears(this._selectedMonth, -1); 
                OnSelectedMonthChanged(selectedMonth);
            }
        } 
 
        private void ProcessRightKey()
        { 
            System.Globalization.Calendar _cal = new GregorianCalendar();
            if (this.DisplayMode == CalendarMode.Month)
            { 
                DateTime selectedDate = _cal.AddDays(this.SelectedDate.GetValueOrDefault(DateTime.Today), 1);
                if (IsValidDate(this, selectedDate))
                { 
                    OnDayClick(selectedDate); 
                }
            } 
            else
            {
                Debug.Assert(this.DisplayMode == CalendarMode.Year); 
                DateTime selectedMonth = _cal.AddMonths(this._selectedMonth, 1);
                OnSelectedMonthChanged(selectedMonth);
            } 
        } 

        private void ProcessUpKey() 
        {
            System.Globalization.Calendar _cal = new GregorianCalendar();
            if (this.DisplayMode == CalendarMode.Month) 
            {
                DateTime selectedDate = _cal.AddDays(this.SelectedDate.GetValueOrDefault(DateTime.Today), -7);
                if (IsValidDate(this, selectedDate)) 
                { 
                    OnDayClick(selectedDate);
                } 
            }
            else
            { 
                Debug.Assert(this.DisplayMode == CalendarMode.Year);
                DateTime selectedMonth = _cal.AddMonths(this._selectedMonth, -4);
                OnSelectedMonthChanged(selectedMonth); 
            } 
        }
 
        private void UpdateMonths()
        {
            Month monthControl = this.MonthControl; 
            if (monthControl != null)
            {
                if (this.DisplayMode == CalendarMode.Month) 
                { 
                    monthControl.UpdateMonthMode();
                } 
                else
                {
                    Debug.Assert(this.DisplayMode == CalendarMode.Year); 
                    monthControl.UpdateYearMode();
                }
            } 
        } 

        private void UpdateSelectedDate(DateTime? addedDate, DateTime? removedDate) 
        {
            Month monthControl = this.MonthControl;
            if (monthControl != null) 
            {
                monthControl.UpdateSelectedDate(addedDate, removedDate);
            } 
        } 

        #endregion Private Methods 
    }
}
