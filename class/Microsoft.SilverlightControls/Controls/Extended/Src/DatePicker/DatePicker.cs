// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System.Diagnostics;
using System.Globalization; 
using System.Windows.Controls.Primitives; 
using System.Windows.Input;
using System.Windows.Media; 
using System.Windows.Media.Animation;
using System.Windows.Controls;

namespace System.Windows.Controlsb1
{
    /// <summary>
    /// Represents a control that allows the user to select a date. 
    /// </summary> 
    [TemplatePart(Name = DatePicker.DATEPICKER_elementRootName, Type = typeof(Grid))]
    [TemplatePart(Name = DatePicker.DATEPICKER_elementTextBoxName, Type = typeof(WatermarkedTextBox))] 
    [TemplatePart(Name = DatePicker.DATEPICKER_elementButtonName, Type = typeof(Button))]
    [TemplatePart(Name = DatePicker.DATEPICKER_stateNormalName, Type = typeof(Storyboard))]
    [TemplatePart(Name = DatePicker.DATEPICKER_stateDisabledName, Type = typeof(Storyboard))] 

    public partial class DatePicker : Control
    { 
        #region Constants 

        private const string DATEPICKER_elementButtonName = "ButtonElement"; 
        private const string DATEPICKER_elementRootName = "RootElement";
        private const string DATEPICKER_elementTextBoxName = "TextBoxElement";
        private const string DATEPICKER_stateDisabledName = "Disabled State"; 
        private const string DATEPICKER_stateNormalName = "Normal State";

        #endregion Constants 
 
        #region Data
 
        private Calendar _calendar;
        private CalendarMode _calendarMode;
        private Storyboard _currentStoryboard; 
        private string _defaultText;
        private FrameworkElement _disabledVisual;
        private DateTime _displayDate; 
        private DateTime? _displayDateEnd; 
        private DateTime? _displayDateStart;
        private Button _dropDownButton; 
        private Popup _popUp;
        private Grid _rootGrid;
        private DateTime? _selectedDate; 
        private DateTime? _selectedDateEnd;
        private DateTime? _selectedDateStart;
        private Storyboard _stateDisabled; 
        private Storyboard _stateNormal; 
        private WatermarkedTextBox _textBox;
 
        #endregion Data

        #region Public Events 

        /// <summary>
        /// Occurs when the drop-down Calendar is closed. 
        /// </summary> 
        public event RoutedEventHandler CalendarClosed;
 
        /// <summary>
        /// Occurs when the drop-down Calendar is opened.
        /// </summary> 
        public event RoutedEventHandler CalendarOpened;

        /// <summary> 
        /// Occurs when a date is selected. 
        /// </summary>
        public event EventHandler<DatePickerDateChangedEventArgs> DateSelected; 

        /// <summary>
        /// Occurs when text entered into the DatePicker cannot be parsed. 
        /// </summary>
        public event EventHandler<DatePickerTextParseErrorEventArgs> TextParseError;
 
        #endregion PublicEvents 

        /// <summary> 
        /// Initializes a new instance of the DatePicker class.
        /// </summary>
        public DatePicker() 
        {
            InitializeCalendar();
            this.FirstDayOfWeek = DayOfWeek.Sunday; 
            this.IsTodayHighlighted = true; 
            this.AreDatesInPastSelectable = true;
            this.SelectedDateFormat = DatePickerFormat.Short; 
            this._defaultText = string.Empty;
            this.DisplayDate = DateTime.Today;
            this.IsDropDownOpen = false; 
            this.GotFocus += new RoutedEventHandler(DatePicker_GotFocus);
            this.LostFocus += new RoutedEventHandler(DatePicker_LostFocus);
            this._popUp = new Popup(); 
            this.IsEnabled = true; 
            Debug.Assert(_popUp != null);
            this._popUp.Child = this._calendar; 
        }

        #region Public properties 

        #region AreDatesInPastSelectable
 
        /// <summary> 
        /// Gets or sets a value that indicates whether dates before Today are valid selections.
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
            typeof(DatePicker),
            new PropertyMetadata (new PropertyChangedCallback(OnAreDatesInPastSelectableChanged))); 
 
        /// <summary>
        /// AreDatesInPastSelectableProperty property changed handler. 
        /// </summary>
        /// <param name="d">DatePicker that changed its AreDatesInPastSelectable.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param> 
        private static void OnAreDatesInPastSelectableChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DatePicker dp = d as DatePicker; 
            Debug.Assert(dp != null); 

            dp._calendar.AreDatesInPastSelectable = dp.AreDatesInPastSelectable; 
        }

        #endregion AreDatesInPastSelectable 

        #region CalendarStyle
 
        /// <summary> 
        /// Gets or sets the style that is used when rendering the calendar.
        /// </summary> 
        public Style CalendarStyle
        {
            get { return (Style)GetValue(CalendarStyleProperty); } 
            set { SetValue(CalendarStyleProperty, value); }
        }
 
        /// <summary> 
        /// Identifies the CalendarStyle dependency property.
        /// </summary> 
        public static readonly DependencyProperty CalendarStyleProperty =
            DependencyProperty.Register(
            "CalendarStyle", 
            typeof(Style),
            typeof(DatePicker),
            new PropertyMetadata (new PropertyChangedCallback(OnCalendarStyleChanged))); 
 
        /// <summary>
        /// CalendarStyleProperty property changed handler. 
        /// </summary>
        /// <param name="d">DatePicker that changed its CalendarStyle.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param> 
        private static void OnCalendarStyleChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            Style newStyle = e.NewValue as Style; 
            if (newStyle != null) 
            {
                DatePicker dp = d as DatePicker; 

                if (dp != null)
                { 
                    Style oldStyle = e.OldValue as Style;

                    // Set the style for the calendar if it has not already been set 
 
                    if (dp._calendar != null)
                    { 
                            //

                            if ( dp._calendar.Style == null || dp._calendar.Style == oldStyle) 
                            {
                                dp._calendar.Style = newStyle;
                            } 
                    } 
                }
            } 
        }

        #endregion CalendarStyle 

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
            typeof(DatePicker),
            new PropertyMetadata (new PropertyChangedCallback(OnDisplayDateChanged)));
 
        /// <summary> 
        /// DisplayDateProperty property changed handler.
        /// </summary> 
        /// <param name="d">DatePicker that changed its DisplayDate.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnDisplayDateChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) 
        {
            DatePicker dp = d as DatePicker;
            Debug.Assert(dp != null); 
 

            dp._displayDate = DiscardDayTime((DateTime)e.NewValue); 

            if (dp._calendar.DisplayDate != dp._displayDate)
            { 
                dp._calendar.DisplayDate = dp._displayDate;
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
            typeof(DatePicker),
            new PropertyMetadata (new PropertyChangedCallback(OnDisplayDateEndChanged)));
 
        /// <summary>
        /// DisplayDateEndProperty property changed handler.
        /// </summary> 
        /// <param name="d">DatePicker that changed its DisplayDateEnd.</param> 
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnDisplayDateEndChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) 
        {
            DatePicker dp = d as DatePicker;
            Debug.Assert(dp != null); 

            dp._displayDateEnd = DiscardTime((DateTime?)e.NewValue);
            dp._calendar.DisplayDateEnd = dp._displayDateEnd; 
        } 
        #endregion DisplayDateEnd
 
        #region DisplayDateStart

        /// <summary> 
        /// Gets or sets the first date to be displayed.
        /// </summary>
        public DateTime? DisplayDateStart 
        { 
            get{ return (DateTime?)GetValue(DisplayDateStartProperty);}
            set{ SetValue(DisplayDateStartProperty, value);} 
        }

        /// <summary> 
        /// Identifies the DisplayDateStart dependency property.
        /// </summary>
        public static readonly DependencyProperty DisplayDateStartProperty = 
            DependencyProperty.Register( 
            "DisplayDateStart",
            typeof(DateTime?), 
            typeof(DatePicker),
            new PropertyMetadata (new PropertyChangedCallback(OnDisplayDateStartChanged)));
 
        /// <summary>
        /// DisplayDateStartProperty property changed handler.
        /// </summary> 
        /// <param name="d">DatePicker that changed its DisplayDateStart.</param> 
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnDisplayDateStartChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) 
        {
            DatePicker dp = d as DatePicker;
            Debug.Assert(dp != null); 

            dp._displayDateStart = DiscardTime((DateTime?)e.NewValue);
            dp._calendar.DisplayDateStart = dp._displayDateStart; 
 
        }
 
        #endregion DisplayDateStart

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
            typeof(DatePicker), 
            new PropertyMetadata (new PropertyChangedCallback(OnFirstDayOfWeekChanged)));

        /// <summary> 
        /// FirstDayOfWeekProperty property changed handler. 
        /// </summary>
        /// <param name="d">DatePicker that changed its FirstDayOfWeek.</param> 
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnFirstDayOfWeekChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        { 
            DatePicker dp = d as DatePicker;
            Debug.Assert(dp != null);
 
            dp._calendar.FirstDayOfWeek = dp.FirstDayOfWeek; 

        } 

        #endregion FirstDayOfWeek
 
        #region IsDropDownOpen

        /// <summary> 
        /// Gets or sets a value that indicates whether the drop-down Calendar is open or closed. 
        /// </summary>
        public bool IsDropDownOpen 
        {
            get{ return (bool)GetValue(IsDropDownOpenProperty);}
            set{ SetValue(IsDropDownOpenProperty, value);} 
        }

        /// <summary> 
        /// Identifies the IsDropDownOpen dependency property. 
        /// </summary>
        public static readonly DependencyProperty IsDropDownOpenProperty = 
            DependencyProperty.Register(
            "IsDropDownOpen",
            typeof(bool), 
            typeof(DatePicker),
            new PropertyMetadata (new PropertyChangedCallback(OnIsDropDownOpenChanged)));
 
        /// <summary> 
        /// IsDropDownOpenProperty property changed handler.
        /// </summary> 
        /// <param name="d">DatePicker that changed its IsDropDownOpen.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnIsDropDownOpenChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) 
        {
            DatePicker dp = d as DatePicker;
            Debug.Assert(dp != null); 
            bool newValue = (bool)e.NewValue; 
            bool oldValue = (bool)e.OldValue;
 

            if ( dp._popUp != null && dp._popUp.Child != null)
            { 
                if (newValue != oldValue)
                {
                    if (dp._calendar.DisplayMode == CalendarMode.Year) 
                    { 
                        dp._calendar.DisplayMode = CalendarMode.Month;
                    } 

                    dp._popUp.IsOpen = newValue;
                    dp._calendar.ResetStates(); 

                    if (newValue)
                    { 
                        //
                     //
                        dp.OnCalendarOpened(new RoutedEventArgs()); 
                    }
                    else
                    { 
                        Debug.Assert(!newValue);
                        dp.OnCalendarClosed(new RoutedEventArgs());
                    } 
 
                }
            } 
        }

        #endregion IsDropDownOpen 

        #region IsEnabled
 
        /// <summary> 
        /// Gets or sets a value that indicates whether this element is enabled in the user interface (UI).
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
            typeof(DatePicker),
            new PropertyMetadata (new PropertyChangedCallback(OnIsEnabledPropertyChanged))); 
 
        /// <summary>
        /// IsEnabledProperty property changed handler. 
        /// </summary>
        /// <param name="d">DatePicker that changed its IsEnabled.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param> 
        private static void OnIsEnabledPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DatePicker dp = d as DatePicker; 
            Debug.Assert(dp != null); 

            dp.UpdateDisabledVisual((bool)e.NewValue); 
        }

        #endregion IsEnabled 

        #region IsTodayHighlighted
 
        /// <summary> 
        /// Gets or sets a value that indicates whether the current date will be highlighted.
        /// </summary> 
        public bool IsTodayHighlighted
        {
            get{ return (bool)GetValue(IsTodayHighlightedProperty);} 
            set{ SetValue(IsTodayHighlightedProperty, value);}
        }
 
        /// <summary> 
        /// Identifies the IsTodayHighlighted dependency property.
        /// </summary> 
        public static readonly DependencyProperty IsTodayHighlightedProperty =
            DependencyProperty.Register(
            "IsTodayHighlighted", 
            typeof(bool),
            typeof(DatePicker),
            new PropertyMetadata (new PropertyChangedCallback(OnIsTodayHighlightedChanged))); 
 
        /// <summary>
        /// IsTodayHighlightedProperty property changed handler. 
        /// </summary>
        /// <param name="d">DatePicker that changed its IsTodayHighlighted.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param> 
        private static void OnIsTodayHighlightedChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DatePicker dp = d as DatePicker; 
            Debug.Assert(dp != null); 

            dp._calendar.IsTodayHighlighted = dp.IsTodayHighlighted; 

        }
 
        #endregion IsTodayHighlighted

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
            typeof(DatePicker), 
            new PropertyMetadata (new PropertyChangedCallback(OnSelectedDateChanged))); 

        /// <summary> 
        /// SelectedDateProperty property changed handler.
        /// </summary>
        /// <param name="d">DatePicker that changed its SelectedDate.</param> 
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnSelectedDateChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        { 
            DateTime? addedDate; 
            DateTime? removedDate;
            DatePicker dp = d as DatePicker; 
            Debug.Assert(dp != null);

            addedDate = (DateTime?)e.NewValue; 
            removedDate = (DateTime?)e.OldValue;

            dp._selectedDate = DiscardTime(addedDate); 
 
            if (dp._selectedDate != dp._calendar.SelectedDate)
            { 
                dp._calendar.SelectedDate = dp._selectedDate;
            }
 
            //
            if (DateTime.Compare(addedDate.GetValueOrDefault(DateTime.MinValue), removedDate.GetValueOrDefault(DateTime.MaxValue)) != 0)
            { 
                if (dp.SelectedDate != null) 
                {
                    DateTime day = (DateTime)dp.SelectedDate; 

                    string s = dp.DateTimeToString(day);
 
                    if (s != null && dp._textBox != null)
                    {
                        dp.SetValue(TextProperty, s); 
                    } 

                    if (day.Month != dp.DisplayDate.Month || day.Year != dp.DisplayDate.Year) 
                    {
                        dp.DisplayDate = day;
                    } 
                }
                dp.OnDateSelected(new DatePickerDateChangedEventArgs(removedDate, addedDate));
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
            typeof(DatePicker),
            new PropertyMetadata (new PropertyChangedCallback(OnSelectableDateEndChanged))); 

        /// <summary>
        /// SelectableDateEndProperty property changed handler. 
        /// </summary> 
        /// <param name="d">DatePicker that changed its SelectableDateEnd.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param> 
        private static void OnSelectableDateEndChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DatePicker dp = d as DatePicker; 
            Debug.Assert(dp != null);

 
            dp._selectedDateEnd = DiscardTime((DateTime?)e.NewValue); 
            dp._calendar.SelectableDateEnd = dp._selectedDateEnd;
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
            typeof(DatePicker),
            new PropertyMetadata (new PropertyChangedCallback(OnSelectableDateStartChanged)));
 
        /// <summary> 
        /// SelectableDateStartProperty property changed handler.
        /// </summary> 
        /// <param name="d">DatePicker that changed its SelectableDateStart.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnSelectableDateStartChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) 
        {
            DatePicker dp = d as DatePicker;
            Debug.Assert(dp != null); 
 

            dp._selectedDateStart = DiscardTime((DateTime?)e.NewValue); 
            dp._calendar.SelectableDateStart = dp._selectedDateStart;
        }
 

        #endregion SelectableDateStart
 
        #region SelectedDateFormat 

        /// <summary> 
        /// Gets or sets the format that is used to display the selected date.
        /// </summary>
        public DatePickerFormat SelectedDateFormat 
        {
            get { return (DatePickerFormat)GetValue(SelectedDateFormatProperty); }
            set { SetValue(SelectedDateFormatProperty, value); } 
        } 

        /// <summary> 
        /// Identifies the SelectedDateFormat dependency property.
        /// </summary>
        public static readonly DependencyProperty SelectedDateFormatProperty = 
            DependencyProperty.Register(
            "SelectedDateFormat",
            typeof(DatePickerFormat), 
            typeof(DatePicker), 
            new PropertyMetadata (new PropertyChangedCallback(OnSelectedDateFormatChanged)));
 
        /// <summary>
        /// SelectedDateFormatProperty property changed handler.
        /// </summary> 
        /// <param name="d">DatePicker that changed its SelectedDateFormat.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnSelectedDateFormatChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) 
        { 
            DatePicker dp = d as DatePicker;
            Debug.Assert(dp != null); 

            if (IsValidSelectedDateFormat((DatePickerFormat)e.NewValue))
            { 
                if (dp._textBox != null)
                {
                    //Update WaterMarkedTextBox.Text 
                    if (string.IsNullOrEmpty(dp._textBox.Text)) 
                    {
                        dp.SetWaterMarkText(); 
                    }
                    else
                    { 
                        DateTime? date = dp.ParseText(dp._textBox.Text, (DatePickerFormat)e.OldValue);

                        if (date != null) 
                        { 
                            string s = dp.DateTimeToString((DateTime)date);
                            dp._textBox.Text = s; 
                        }
                    }
                } 
            }
            else
            { 
                throw new ArgumentOutOfRangeException("d", Resource.DatePicker_OnSelectedDateFormatChanged_InvalidValue); 
            }
 
        }

        #endregion SelectedDateFormat 

        #region Text
 
        /// <summary> 
        /// Gets or sets the text that is displayed by the DatePicker.
        /// </summary> 
        public string Text
        {
            get{ return (string)GetValue(TextProperty);} 
            set{ SetValue(TextProperty, value);}
        }
 
        /// <summary> 
        /// Identifies the Text dependency property.
        /// </summary> 
        public static readonly DependencyProperty TextProperty =
            DependencyProperty.Register(
            "TextProperty", 
            typeof(string),
            typeof(DatePicker),
            new PropertyMetadata (new PropertyChangedCallback(OnTextChanged))); 
 
        /// <summary>
        /// TextProperty property changed handler. 
        /// </summary>
        /// <param name="d">DatePicker that changed its Text.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param> 
        private static void OnTextChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DatePicker dp = d as DatePicker; 
            Debug.Assert(dp != null); 

            if (dp._textBox != null) 
            {
                dp._textBox.Text = (string)e.NewValue;
            } 
            else
            {
                dp._defaultText = (string)e.NewValue; 
            } 
        }
 
        #endregion Text

        #region Tooltip 

        /// <summary>
        /// Gets or sets the tool-tip object that is displayed for this element 
        /// in the user interface (UI). 
        /// </summary>
        public object ToolTip 
        {
            get { return ToolTipService.GetToolTip(this); }
            set { ToolTipService.SetToolTip(this, value); } 
        }

        #endregion ToolTip 
 
        #endregion Public Properties
 
        #region Protected properties
        #endregion Protected Properties
 
        #region Internal Properties

        // 
 
        internal Popup DropDown
        { 
            get
            {
               return this._popUp ; 
            }
            private set
            { 
                this._popUp = value; 
            }
        } 

        internal Button DropDownButton
        { 
            get
            {
                return this._dropDownButton; 
            } 
            private set
            { 
                if (_dropDownButton != null)
                {
                    this._dropDownButton = value; 
                }
            }
        } 
 
        internal WatermarkedTextBox TB
        { 
            get
            {
                return this._textBox; 
            }
            private set
            { 
                if (this._textBox != null) 
                {
                    this._textBox = value; 
                }
            }
        } 

        #endregion Internal Properties
 
        #region Private Properties 
        #endregion Private Properties
 
        #region Public Methods

        /// <summary> 
        /// Gets the value of any one of the DatePicker object's dependency properties.
        /// </summary>
        /// <param name="dp">The dependency property to get the value of.</param> 
        /// <returns>The value of the specified dependency property.</returns> 
        public override object GetValue(DependencyProperty dp)
        { 

            if ( dp == DatePicker.DisplayDateProperty)
            { 
                return this._displayDate;
            }
 
            if ( dp == DatePicker.DisplayDateStartProperty) 
            {
                return this._displayDateStart; 
            }

            if ( dp == DatePicker.DisplayDateEndProperty) 
            {
                return this._displayDateEnd;
            } 
 
            if ( dp == DatePicker.SelectedDateProperty)
            { 
                return this._selectedDate;
            }
 
            if ( dp == DatePicker.SelectableDateStartProperty)
            {
                return this._selectedDateStart; 
            } 

            if ( dp == DatePicker.SelectableDateEndProperty) 
            {
                return this._selectedDateEnd;
            } 

            if ( dp == DatePicker.TextProperty)
            { 
                if (this._textBox != null) 
                {
                    return this._textBox.Text; 
                }
                else
                { 
                    return this._defaultText;
                }
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
        /// Builds the visual tree for the DatePicker control when a new template is applied.
        /// </summary> 
        public override void OnApplyTemplate() 
        {
            base.OnApplyTemplate(); 

            _dropDownButton = GetTemplateChild(DATEPICKER_elementButtonName) as Button;
 
            if (_dropDownButton != null)
            {
                _dropDownButton.MouseLeftButtonDown += new MouseButtonEventHandler(DropDownButton_MouseLeftButtonDown); 
                _dropDownButton.KeyDown += new KeyEventHandler(DropDownButton_KeyDown); 
                _dropDownButton.IsTabStop = false;
            } 

            _rootGrid = GetTemplateChild(DATEPICKER_elementRootName) as Grid;
 
            if (_rootGrid != null)
            {
                _stateDisabled = _rootGrid.Resources[DATEPICKER_stateDisabledName] as Storyboard; 
                _stateNormal = _rootGrid.Resources[DATEPICKER_stateNormalName] as Storyboard; 
                _rootGrid.SizeChanged += new SizeChangedEventHandler(RootGrid_SizeChanged);
            } 

            _textBox = GetTemplateChild(DATEPICKER_elementTextBoxName) as WatermarkedTextBox;
            _disabledVisual = GetTemplateChild("DisabledVisual") as FrameworkElement; 


            UpdateDisabledVisual(this.IsEnabled); 
 
            this._calendarMode = CalendarMode.Month;
            this.MouseLeftButtonDown += new MouseButtonEventHandler(DatePicker_MouseLeftButtonDown); 
            this.IsTabStop = true;
            SetWaterMarkText();
 
            if (_textBox != null)
            {
                _textBox.KeyDown += new KeyEventHandler(TextBox_KeyDown); 
                _textBox.IsTabStop = true; 

                if (this.SelectedDate == null) 
                {

                    if (!string.IsNullOrEmpty(this._defaultText)) 
                    {
                        _textBox.Text = this._defaultText;
                    } 
                } 
                else
                { 
                    _textBox.Text = this.DateTimeToString((DateTime)this.SelectedDate);
                }
            } 
        }

 
        /// <summary> 
        /// Raises the TextParseError event.
        /// </summary> 
        /// <param name="e">A DatePickerTextParseErrorEventArgs that contains the event data.</param>
        protected virtual void OnTextParseError(DatePickerTextParseErrorEventArgs e)
        { 
            EventHandler<DatePickerTextParseErrorEventArgs> handler = this.TextParseError;
            if (handler != null)
            { 
                handler(this, e); 
            }
        } 

        #endregion Protected Methods
 
        #region Internal Methods
        #endregion Internal Methods
 
        #region Private Methods 

        private void Calendar_DateSelected(object sender, CalendarDateChangedEventArgs e) 
        {
            if (e.AddedDate != this.SelectedDate)
            { 
                this.SelectedDate = (DateTime?)e.AddedDate;
            }
        } 
 
        private void Calendar_DisplayDateChanged(object sender, CalendarDateChangedEventArgs e)
        { 
            if ( e.AddedDate != this.DisplayDate)
            {
                SetValue(DisplayDateProperty,(DateTime)e.AddedDate); 
            }
        }
 
        private void Calendar_DisplayModeChanged(object sender, CalendarModeChangedEventArgs e) 
        {
            if (e != null) 
            {
                this._calendarMode = e.NewMode;
            } 
        }

        private void Calendar_KeyDown(object sender, KeyEventArgs e) 
        { 
            Calendar c = sender as Calendar;
            Debug.Assert(c != null); 

            if (!e.Handled &&  ( e.Key == Key.Enter || e.Key == Key.Space) && c.DisplayMode == CalendarMode.Month)
            { 
                this.IsDropDownOpen = false;
            }
        } 
 
        private void Calendar_LostFocus(object sender, RoutedEventArgs e)
        { 
            if (this.IsDropDownOpen && this._calendarMode != CalendarMode.Year && !(e.Source is DayButton))
            {
                 this.IsDropDownOpen = false; 
            }
        }
 
        private void Calendar_MouseLeftButtonUp(object sender, MouseButtonEventArgs e) 
        {
            FrameworkElement fe = e.Source as FrameworkElement; 

            if (fe != null && fe.DataContext is DateTime && ((DateTime)fe.DataContext).Hour == 0)
            { 
                this.IsDropDownOpen = false;
                _calendar.ReleaseMouseCapture();
            } 
        } 

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
 
        //

        internal string DateTimeToString(DateTime d) 
        { 
            DateTimeFormatInfo dtfi = CultureInfo.CurrentCulture.DateTimeFormat;
 
            if ( this.SelectedDateFormat == DatePickerFormat.Short)
            {
                return string.Format(CultureInfo.CurrentCulture, d.ToString(dtfi.ShortDatePattern, dtfi)); 
            }
            else
            { 
                return string.Format(CultureInfo.CurrentCulture, d.ToString(dtfi.LongDatePattern, dtfi)); 
            }
        } 

        private void DatePicker_LostFocus(object sender, RoutedEventArgs e)
        { 
            DatePicker dp = sender as DatePicker;
            Debug.Assert(dp != null);
            Debug.Assert(e.Source != null); 
 
            if (dp.IsDropDownOpen && dp._dropDownButton != null && !(e.Source.Equals(dp._dropDownButton)) && !(e.Source.Equals(dp._calendar)))
            { 
                dp.IsDropDownOpen = false;
            }
        } 

        private void DatePicker_GotFocus(object sender, RoutedEventArgs e)
        { 
            DatePicker dp = sender as DatePicker; 
            Debug.Assert(dp != null);
 
            if (this.IsEnabled && this._textBox != null)
            {
                this._textBox.Focus(); 
            }
        }
 
        private void DatePicker_MouseLeftButtonDown(object sender, MouseButtonEventArgs e) 
        {
            if (_popUp.IsOpen && _dropDownButton != null && !_dropDownButton.CaptureMouse()) 
            {
                this.IsDropDownOpen = false;
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
 
        private void DropDownButton_KeyDown(object sender, KeyEventArgs e)
        {
            if (!e.Handled && e.Key == Key.Enter) 
            { 
                HandlePopUp();
            } 
        }

        // 

        internal void DropDownButton_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        { 
                HandlePopUp(); 
        }
 
        private void HandlePopUp()
        {
            if (this.IsDropDownOpen) 
            {
                this.IsDropDownOpen = false;
                _calendar.ReleaseMouseCapture(); 
            } 
            else
            { 
                Debug.Assert(!this.IsDropDownOpen);
                _calendar.CaptureMouse();
                ProcessTextBox(); 
            }
        }
 
        private void InitializeCalendar() 
        {
            _calendar = new Calendar(); 

            _calendar.MouseLeftButtonUp += new MouseButtonEventHandler(Calendar_MouseLeftButtonUp);
 
            _calendar.DisplayModeChanged += new EventHandler<CalendarModeChangedEventArgs>(Calendar_DisplayModeChanged);
            _calendar.DisplayDateChanged += new EventHandler<CalendarDateChangedEventArgs>(Calendar_DisplayDateChanged);
            _calendar.DateSelected += new EventHandler<CalendarDateChangedEventArgs>(Calendar_DateSelected); 
            _calendar.LostFocus += new RoutedEventHandler(Calendar_LostFocus); 
            _calendar.KeyDown += new KeyEventHandler(Calendar_KeyDown);
 

            _calendar.IsTabStop = true;
        } 

        private static bool IsValidSelectedDateFormat(DatePickerFormat value)
        { 
            DatePickerFormat format = (DatePickerFormat)value; 

            return format == DatePickerFormat.Long 
                || format == DatePickerFormat.Short;
        }
 
        private void OnCalendarClosed(RoutedEventArgs e)
        {
            RoutedEventHandler handler = this.CalendarClosed; 
            if (null != handler) 
            {
                handler(this, e); 
            }
        }
 
        private void OnCalendarOpened(RoutedEventArgs e)
        {
            RoutedEventHandler handler = this.CalendarOpened; 
            if (null != handler) 
            {
                handler(this, e); 
            }
        }
 
        private void OnDateSelected(DatePickerDateChangedEventArgs e)
        {
            EventHandler<DatePickerDateChangedEventArgs> handler = this.DateSelected; 
            if (null != handler) 
            {
                handler(this, e); 
            }
        }
 

        // iT SHOULD RETURN NULL IF THE STRING IS NOT VALID, RETURN THE DATETIME VALUE IF IT IS VALID
 
        /// <summary> 
        /// Input text is parsed in the correct format and changed into a DateTime object.
        /// If the text can not be parsed TextParseError Event is thrown. 
        /// </summary>
        private DateTime? ParseText(string text, DatePickerFormat f)
        { 
            DateTime newSelectedDate;
            DateTimeFormatInfo dtfi = CultureInfo.CurrentCulture.DateTimeFormat;
 
            if (f == DatePickerFormat.Short) 
            {
                try 
                {
                    newSelectedDate = DateTime.ParseExact(text,dtfi.ShortDatePattern, CultureInfo.CurrentCulture);
                    return newSelectedDate; 
                }
                catch(Exception ex)
                { 
                    DatePickerTextParseErrorEventArgs textParseError = new DatePickerTextParseErrorEventArgs(ex, text); 
                    OnTextParseError(textParseError);
 
                    if (textParseError.ThrowException)
                    {
                        throw textParseError.Exception; 
                    }
                }
            } 
            else 
            {
                Debug.Assert(f == DatePickerFormat.Long); 
                try
                {
                    newSelectedDate = DateTime.ParseExact(text, dtfi.LongDatePattern, CultureInfo.CurrentCulture); 
                    return newSelectedDate;
                }
                catch (Exception ex) 
                { 
                    DatePickerTextParseErrorEventArgs textParseError = new DatePickerTextParseErrorEventArgs(ex, text);
                    OnTextParseError(textParseError); 

                    if (textParseError.ThrowException)
                    { 
                        throw textParseError.Exception;
                    }
                } 
            } 
            return null;
        } 

        private bool ProcessDatePickerKey(KeyEventArgs e)
        { 
            switch (e.Key)
            {
                case Key.Enter: 
                    SetSelectedDate(); 
                    return true;
            } 
            return false;
        }
 
        private void ProcessTextBox()
        {
            SetSelectedDate(); 
            this.IsDropDownOpen = true; 
            //
            this._calendar.IsTabStop = true; 
            _calendar.Focus();
        }
 
        private void RootGrid_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            FrameworkElement page = (Application.Current != null) ? 
                        Application.Current.RootVisual as FrameworkElement : 
                        null;
 
            if (page != null && this.Parent != null && this._rootGrid != null)
            {
                GeneralTransform gt = this.TransformToVisual(page); 

                if (this._rootGrid != null)
                { 
                    Point p = gt.Transform(new Point(0, this._rootGrid.RowDefinitions[0].ActualHeight)); 
                    _popUp.VerticalOffset = p.Y;
                    _popUp.HorizontalOffset = p.X; 
                }
            }
        } 

        private void SetSelectedDate()
        { 
            if (this._textBox != null) 
            {
                if (!string.IsNullOrEmpty(this._textBox.Text)) 
                {
                    string s = this._textBox.Text;
                    DateTime? d = SetTextBoxValue(s); 
                    this.SelectedDate = d;
                }
            } 
        } 

        private DateTime? SetTextBoxValue(string s) 
        {
            if (string.IsNullOrEmpty(s))
            { 
                SetValue(TextProperty, s);
                return this.SelectedDate;
            } 
            else 
            {
                DateTime? d = ParseText(s, this.SelectedDateFormat); 

                if (d != null)
                { 
                    SetValue(TextProperty, s);
                    return d;
                } 
                else 
                {
                    ////If parse error: 
                    //TextBox should have the latest valid selecteddate value:
                    if (this.SelectedDate != null)
                    { 
                        string newtext = this.DateTimeToString((DateTime)this.SelectedDate);
                        SetValue(TextProperty,newtext);
                        if (this._textBox != null) 
                        { 
                            this._textBox.Text = newtext;
                        } 
                        return this.SelectedDate;
                    }
                    else 
                    {
                        SetWaterMarkText();
                        return null; 
                    } 
                }
            } 
        }

        private void SetWaterMarkText() 
        {
            if (this._textBox != null)
            { 
                DateTimeFormatInfo dtfi = CultureInfo.CurrentCulture.DateTimeFormat; 
                this._textBox.Text = string.Empty;
                if (this.SelectedDateFormat == DatePickerFormat.Long) 
                {
                    this._textBox.Watermark = string.Format(CultureInfo.CurrentCulture, Resource.DatePicker_WatermarkText, dtfi.LongDatePattern.ToString());
                } 
                else
                {
                    Debug.Assert(this.SelectedDateFormat == DatePickerFormat.Short); 
                    this._textBox.Watermark = string.Format(CultureInfo.CurrentCulture, Resource.DatePicker_WatermarkText, dtfi.ShortDatePattern.ToString()); 
                }
            } 
        }

        private void TextBox_KeyDown(object sender, KeyEventArgs e) 
        {
            if (!e.Handled)
            { 
                e.Handled = ProcessDatePickerKey(e); 
            }
        } 

        private void UpdateDisabledVisual(bool isEnabled)
        { 
            if (_disabledVisual != null)
            {
                if (isEnabled) 
                { 
                    _disabledVisual.Visibility = Visibility.Collapsed;
                    ChangeVisualState(_stateNormal); 
                }
                else
                { 
                    Debug.Assert((bool)isEnabled == false);
                    _disabledVisual.Visibility = Visibility.Visible;
                    ChangeVisualState(_stateDisabled); 
                } 
            }
        } 

        #endregion Private Methods
 
    }
}
