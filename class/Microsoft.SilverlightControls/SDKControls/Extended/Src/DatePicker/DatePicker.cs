// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Diagnostics;
using System.Globalization;
using System.Windows.Automation.Peers;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using System.Windows.Media;

namespace System.Windows.Controls
{
    /// <summary>
    /// Represents a control that allows the user to select a date.
    /// </summary>
    [TemplatePart(Name = DatePicker.ElementRoot, Type = typeof(FrameworkElement))]
    [TemplatePart(Name = DatePicker.ElementTextBox, Type = typeof(DatePickerTextBox))]
    [TemplatePart(Name = DatePicker.ElementButton, Type = typeof(Button))]
    [TemplatePart(Name = DatePicker.ElementPopup, Type = typeof(Popup))]
    [TemplateVisualState(Name = VisualStates.StateNormal, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = VisualStates.StateDisabled, GroupName = VisualStates.GroupCommon)]
    public partial class DatePicker : Control
    {
        #region Constants

        private const string ElementRoot = "Root";
        private const string ElementTextBox = "TextBox";
        private const string ElementButton = "Button";
        private const string ElementPopup = "Popup";

        #endregion Constants

        #region Data

        private Calendar _calendar;
        private string _defaultText;
        private Button _dropDownButton;
        private Canvas _outsideCanvas;
        private Canvas _outsidePopupCanvas;
        private Popup _popUp;
        private FrameworkElement _root;
        private bool _settingSelectedDate;
        private DatePickerTextBox _textBox;

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
        /// Occurs when text entered into the DatePicker cannot be parsed or the Date is not valid to be selected.
        /// </summary>
        public event EventHandler<DatePickerDateValidationErrorEventArgs> DateValidationError;

        /// <summary>
        /// Occurs when a date is selected.
        /// </summary>
        public event EventHandler<SelectionChangedEventArgs> SelectedDateChanged;

        #endregion PublicEvents

        /// <summary>
        /// Initializes a new instance of the DatePicker class. 
        /// </summary>
        public DatePicker()
        {
            InitializeCalendar();
            this.IsEnabledChanged += new DependencyPropertyChangedEventHandler(OnIsEnabledChanged);
            this.FirstDayOfWeek = DateTimeHelper.GetCurrentDateFormat().FirstDayOfWeek;
            this.IsTodayHighlighted = true;
            this.SelectedDateFormat = DatePickerFormat.Short;
            this._defaultText = string.Empty;
            this.SetValueNoCallback(DatePicker.TextProperty, string.Empty);
            this.DisplayDate = DateTime.Today;
            this.IsDropDownOpen = false;
            this.GotFocus += new RoutedEventHandler(DatePicker_GotFocus);
            this.LostFocus += new RoutedEventHandler(DatePicker_LostFocus);
            this.BlackoutDates = this._calendar.BlackoutDates;
            DefaultStyleKey = typeof(DatePicker);
        }

        #region Public properties

        #region BlackoutDates

        /// <summary>
        /// Gets the days that are not selectable.
        /// </summary>
        public CalendarBlackoutDatesCollection BlackoutDates
        {
            get;
            private set;
        }

        #endregion BlackoutDates

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
            new PropertyMetadata(OnCalendarStyleChanged));

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
                DatePicker datePicker = d as DatePicker;

                if (datePicker != null)
                {
                    Style oldStyle = e.OldValue as Style;

                    // Set the style for the calendar if it has not already been set

                    if (datePicker._calendar != null)
                    {
                        // 

                        if (datePicker._calendar.Style == null || datePicker._calendar.Style == oldStyle)
                        {
                            datePicker._calendar.Style = newStyle;
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
            typeof(DatePicker),
            new PropertyMetadata(OnDisplayDateChanged));

        /// <summary>
        /// DisplayDateProperty property changed handler.
        /// </summary>
        /// <param name="d">DatePicker that changed its DisplayDate.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnDisplayDateChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DatePicker datePicker = d as DatePicker;
            Debug.Assert(datePicker != null);

            if ( DateTimeHelper.CompareYearMonth(datePicker._calendar.DisplayDate,(DateTime)e.NewValue) != 0)
            {
                datePicker._calendar.DisplayDate = datePicker.DisplayDate;

                if (DateTime.Compare(datePicker._calendar.DisplayDate, datePicker.DisplayDate) != 0)
                {
                    datePicker.DisplayDate = datePicker._calendar.DisplayDate;
                }
            }
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
            typeof(DatePicker),
            new PropertyMetadata(OnDisplayDateEndChanged));

        /// <summary>
        /// DisplayDateEndProperty property changed handler.
        /// </summary>
        /// <param name="d">DatePicker that changed its DisplayDateEnd.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnDisplayDateEndChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DatePicker datePicker = d as DatePicker;
            Debug.Assert(datePicker != null);

            datePicker._calendar.DisplayDateEnd = datePicker.DisplayDateEnd;

            if ( datePicker._calendar.DisplayDateEnd.HasValue && datePicker.DisplayDateEnd.HasValue && DateTime.Compare(datePicker._calendar.DisplayDateEnd.Value, datePicker.DisplayDateEnd.Value) != 0)
            {
                datePicker.DisplayDateEnd = datePicker._calendar.DisplayDateEnd;
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
            typeof(DatePicker),
            new PropertyMetadata(OnDisplayDateStartChanged));

        /// <summary>
        /// DisplayDateStartProperty property changed handler.
        /// </summary>
        /// <param name="d">DatePicker that changed its DisplayDateStart.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnDisplayDateStartChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DatePicker datePicker = d as DatePicker;
            Debug.Assert(datePicker != null);

            datePicker._calendar.DisplayDateStart = datePicker.DisplayDateStart;

            if (datePicker._calendar.DisplayDateStart.HasValue && datePicker.DisplayDateStart.HasValue && DateTime.Compare(datePicker._calendar.DisplayDateStart.Value, datePicker.DisplayDateStart.Value) != 0)
            {
                datePicker.DisplayDateStart = datePicker._calendar.DisplayDateStart;
            }
        }

        #endregion DisplayDateStart

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
            typeof(DatePicker),
            new PropertyMetadata(OnFirstDayOfWeekChanged));

        /// <summary>
        /// FirstDayOfWeekProperty property changed handler.
        /// </summary>
        /// <param name="d">DatePicker that changed its FirstDayOfWeek.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnFirstDayOfWeekChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DatePicker datePicker = d as DatePicker;
            Debug.Assert(datePicker != null);

            datePicker._calendar.FirstDayOfWeek = datePicker.FirstDayOfWeek;

        }

        #endregion FirstDayOfWeek

        #region IsDropDownOpen

        /// <summary>
        /// Gets or sets a value that indicates whether the drop-down Calendar is open or closed.
        /// </summary>
        public bool IsDropDownOpen
        {
            get { return (bool)GetValue(IsDropDownOpenProperty); }
            set { SetValue(IsDropDownOpenProperty, value); }
        }

        /// <summary>
        /// Identifies the IsDropDownOpen dependency property.
        /// </summary>
        public static readonly DependencyProperty IsDropDownOpenProperty =
            DependencyProperty.Register(
            "IsDropDownOpen",
            typeof(bool),
            typeof(DatePicker),
            new PropertyMetadata(OnIsDropDownOpenChanged));

        /// <summary>
        /// IsDropDownOpenProperty property changed handler.
        /// </summary>
        /// <param name="d">DatePicker that changed its IsDropDownOpen.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnIsDropDownOpenChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            
            DatePicker datePicker = d as DatePicker;
            Debug.Assert(datePicker != null);
            bool newValue = (bool)e.NewValue;
            bool oldValue = (bool)e.OldValue;

            if (datePicker._popUp != null && datePicker._popUp.Child != null)
            {
                if (newValue != oldValue)
                {
                    if (datePicker._calendar.DisplayMode != CalendarMode.Month)
                    {
                        datePicker._calendar.DisplayMode = CalendarMode.Month;
                    }

                    if (newValue)
                    {
                        datePicker.OpenDropDown();
                    }
                    else
                    {
                        Debug.Assert(!newValue);
                        datePicker._popUp.IsOpen = false;
                        datePicker.OnCalendarClosed(new RoutedEventArgs());
                    }

                }
            }
        }

        #endregion IsDropDownOpen

        #region IsEnabled

        /// <summary>
        /// Called when the IsEnabled property changes.
        /// </summary>
        /// <param name="sender">Sender object</param>
        /// <param name="e">Property changed args</param>
        private void OnIsEnabledChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            UpdateDisabledVisual();
        }

        #endregion IsEnabled

        #region IsTodayHighlighted

        /// <summary>
        /// Gets or sets a value that indicates whether the current date will be highlighted.
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
            typeof(DatePicker),
            new PropertyMetadata(OnIsTodayHighlightedChanged));

        /// <summary>
        /// IsTodayHighlightedProperty property changed handler.
        /// </summary>
        /// <param name="d">DatePicker that changed its IsTodayHighlighted.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnIsTodayHighlightedChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DatePicker datePicker = d as DatePicker;
            Debug.Assert(datePicker != null);

            datePicker._calendar.IsTodayHighlighted = datePicker.IsTodayHighlighted;

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
            typeof(DatePicker),
            new PropertyMetadata(OnSelectedDateChanged));

        /// <summary>
        /// SelectedDateProperty property changed handler.
        /// </summary>
        /// <param name="d">DatePicker that changed its SelectedDate.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnSelectedDateChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            Collection<DateTime> addedItems = new Collection<DateTime>();
            Collection<DateTime> removedItems = new Collection<DateTime>();
            DateTime? addedDate;
            DateTime? removedDate;
            DatePicker datePicker = d as DatePicker;
            Debug.Assert(datePicker != null);

            addedDate = (DateTime?)e.NewValue;
            removedDate = (DateTime?)e.OldValue;

            if (addedDate.HasValue)
            {
                addedItems.Add(addedDate.Value);
            }

            if (removedDate.HasValue)
            {
                removedItems.Add(removedDate.Value);
            }

            if (addedDate != datePicker._calendar.SelectedDate)
            {
                datePicker._calendar.SelectedDate = addedDate;
            }

            if (datePicker.SelectedDate != null)
            {
                DateTime day = (DateTime)datePicker.SelectedDate;
                
                //When the SelectedDateProperty change is done from OnTextPropertyChanged method, two-way binding breaks
                //if BeginInvoke is not used:
                datePicker.Dispatcher.BeginInvoke(delegate
                {
                    datePicker._settingSelectedDate = true;
                    datePicker.Text = datePicker.DateTimeToString(day);
                    datePicker._settingSelectedDate = false;
                    datePicker.OnDateSelected(new SelectionChangedEventArgs(removedItems, addedItems));
                });

                //When DatePickerDisplayDateFlag is TRUE, the SelectedDate change is coming from the Calendar UI itself,
                //so, we shouldn't change the DisplayDate since it will automatically be changed by the Calendar
                if ((day.Month != datePicker.DisplayDate.Month || day.Year != datePicker.DisplayDate.Year) && !datePicker._calendar.DatePickerDisplayDateFlag)
                {
                    datePicker.DisplayDate = day;
                }
                datePicker._calendar.DatePickerDisplayDateFlag = false;
            }
            else
            {
                datePicker._settingSelectedDate = true;
                datePicker.SetWaterMarkText();
                datePicker._settingSelectedDate = false;
                datePicker.OnDateSelected(new SelectionChangedEventArgs(removedItems, addedItems));
            }
            
        }

        #endregion SelectedDate

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
            new PropertyMetadata(OnSelectedDateFormatChanged));

        /// <summary>
        /// SelectedDateFormatProperty property changed handler.
        /// </summary>
        /// <param name="d">DatePicker that changed its SelectedDateFormat.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnSelectedDateFormatChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DatePicker datePicker = d as DatePicker;
            Debug.Assert(datePicker != null);

            if (IsValidSelectedDateFormat((DatePickerFormat)e.NewValue))
            {
                if (datePicker._textBox != null)
                {
                    //Update DatePickerTextBox.Text
                    if (string.IsNullOrEmpty(datePicker._textBox.Text))
                    {
                        datePicker.SetWaterMarkText();
                    }
                    else
                    {
                        DateTime? date = datePicker.ParseText(datePicker._textBox.Text);

                        if (date != null)
                        {
                            string s = datePicker.DateTimeToString((DateTime)date);
                            datePicker.Text = s;
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

        #region SelectionBackground

        /// <summary>
        /// Gets or sets the SelectedBackground of the DatePickerTextBox.
        /// </summary>
        public Brush SelectionBackground
        {
            get { return (Brush)GetValue(SelectionBackgroundProperty); }
            set { SetValue(SelectionBackgroundProperty, value); }
        }

        /// <summary>
        /// Identifies the SelectionBackground dependency property.
        /// </summary>
        public static readonly DependencyProperty SelectionBackgroundProperty =
            DependencyProperty.Register(
            "SelectionBackground",
            typeof(Brush),
            typeof(DatePicker),
            null);

        #endregion SelectionBackground

        #region Text

        /// <summary>
        /// Gets or sets the text that is displayed by the DatePicker.
        /// </summary>
        public string Text
        {
            get { return (string)GetValue(TextProperty); }
            set { SetValue(TextProperty, value); }
        }

        /// <summary>
        /// Identifies the Text dependency property.
        /// </summary>
        public static readonly DependencyProperty TextProperty =
            DependencyProperty.Register(
            "Text",
            typeof(string),
            typeof(DatePicker),
            new PropertyMetadata(OnTextChanged));

        /// <summary>
        /// TextProperty property changed handler.
        /// </summary>
        /// <param name="d">DatePicker that changed its Text.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnTextChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DatePicker datePicker = d as DatePicker;
            Debug.Assert(datePicker != null);

            if (!datePicker.IsHandlerSuspended(DatePicker.TextProperty))
            {
                string newValue = e.NewValue as String;

                if (newValue != null)
                {
                    if (datePicker._textBox != null)
                    {
                        datePicker._textBox.Text = newValue;
                    }
                    else
                    {
                        datePicker._defaultText = newValue;
                    }
                    if (!datePicker._settingSelectedDate)
                    {
                        datePicker.SetSelectedDate();
                    }
                }
                else
                {
                    if (!datePicker._settingSelectedDate)
                    {
                        datePicker.SetValueNoCallback(DatePicker.SelectedDateProperty, null);
                    }
                }
            }
        }

        #endregion Text

        #endregion Public Properties

        #region Protected properties

        #endregion Protected Properties

        #region Internal Properties

        #endregion Internal Properties

        #region Private Properties
        #endregion Private Properties

        #region Public Methods

        /// <summary>
        /// Builds the visual tree for the DatePicker control when a new template is applied.
        /// </summary>
        public override void OnApplyTemplate()
        {
            base.OnApplyTemplate();

            if (_popUp != null)
            {
                _popUp.Child = null;
            }

            _popUp = GetTemplateChild(ElementPopup) as Popup;

            if (_popUp != null)
            {
                if (this._outsideCanvas == null)
                {
                    _outsideCanvas = new Canvas();
                    _outsidePopupCanvas = new Canvas();
                    _outsidePopupCanvas.Background = new SolidColorBrush(Colors.Transparent);
                    _outsideCanvas.Children.Add(this._outsidePopupCanvas);
                    _outsideCanvas.Children.Add(this._calendar);
                    _outsidePopupCanvas.MouseLeftButtonDown += new MouseButtonEventHandler(OutsidePopupCanvas_MouseLeftButtonDown);
                }
                
                _popUp.Child = this._outsideCanvas;
                _root = GetTemplateChild(ElementRoot) as FrameworkElement;

                if (this.IsDropDownOpen)
                {
                    OpenDropDown();
                }
            }

            if (_dropDownButton != null)
            {
                _dropDownButton.Click -= new RoutedEventHandler(DropDownButton_Click);
            }

            _dropDownButton = GetTemplateChild(ElementButton) as Button;
            if (_dropDownButton != null)
            {
                _dropDownButton.Click += new RoutedEventHandler(DropDownButton_Click);
                _dropDownButton.IsTabStop = false;

                //If the user does not provide a Content value in template, we provide a helper text that can be used in Accessibility
                //this text is not shown on the UI, just used for Accessibility purposes
                if (_dropDownButton.Content == null)
                {
                    _dropDownButton.Content = Resource.DatePicker_DropDownButtonName;
                }
            }

            if (_textBox != null)
            {
                _textBox.KeyDown -= new KeyEventHandler(TextBox_KeyDown);
                _textBox.TextChanged -= new TextChangedEventHandler(TextBox_TextChanged);
                _textBox.GotFocus -= new RoutedEventHandler(TextBox_GotFocus);
            }

            _textBox = GetTemplateChild(ElementTextBox) as DatePickerTextBox;

            UpdateDisabledVisual();
            if (this.SelectedDate == null)
            {
                SetWaterMarkText();
            }

            if (_textBox != null)
            {
                _textBox.KeyDown += new KeyEventHandler(TextBox_KeyDown);
                _textBox.TextChanged += new TextChangedEventHandler(TextBox_TextChanged);
                _textBox.GotFocus += new RoutedEventHandler(TextBox_GotFocus);

                if (this.SelectedDate == null)
                {

                    if (!string.IsNullOrEmpty(this._defaultText))
                    {
                        _textBox.Text = this._defaultText;
                        SetSelectedDate();
                    }
                }
                else
                {
                    _textBox.Text = this.DateTimeToString((DateTime)this.SelectedDate);
                }
            }
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
        /// Creates the automation peer for this DatePicker Control.
        /// </summary>
        /// <returns></returns>
        protected override AutomationPeer OnCreateAutomationPeer()
        {
            return new DatePickerAutomationPeer(this);
        }

        /// <summary>
        /// Raises the DateValidationError event.
        /// </summary>
        /// <param name="e">A DatePickerDateValidationErrorEventArgs that contains the event data.</param>
        protected virtual void OnDateValidationError(DatePickerDateValidationErrorEventArgs e)
        {
            EventHandler<DatePickerDateValidationErrorEventArgs> handler = this.DateValidationError;
            if (handler != null)
            {
                handler(this, e);
            }
        }

        #endregion Protected Methods

        #region Internal Methods
        #endregion Internal Methods

        #region Private Methods

        private void Calendar_DayButtonMouseUp(object sender, MouseButtonEventArgs e)
        {
            this.Focus();
            this.IsDropDownOpen = false;
            _calendar.ReleaseMouseCapture();      
        }

        private void Calendar_DisplayDateChanged(object sender, CalendarDateChangedEventArgs e)
        {
            if (e.AddedDate != this.DisplayDate)
            {
                SetValue(DisplayDateProperty, (DateTime)e.AddedDate);
            }
        }

        private void Calendar_KeyDown(object sender, KeyEventArgs e)
        {
            Calendar c = sender as Calendar;
            Debug.Assert(c != null);

            if (!e.Handled && (e.Key == Key.Enter || e.Key == Key.Space || e.Key == Key.Escape) && c.DisplayMode == CalendarMode.Month)
            {
                this.Focus();
                this.IsDropDownOpen = false;
            }
        }

        private void Calendar_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            e.Handled = true;
        }

        private void Calendar_SelectedDatesChanged(object sender, SelectionChangedEventArgs e)
        {
            Debug.Assert(e.AddedItems.Count < 2);

            if (e.AddedItems.Count > 0 && this.SelectedDate.HasValue && DateTime.Compare((DateTime)e.AddedItems[0], this.SelectedDate.Value) != 0)
            {
                this.SelectedDate = (DateTime?)e.AddedItems[0];
            }
            else
            {
                if (e.AddedItems.Count == 0)
                {
                    this.SelectedDate = null;
                    return;
                }

                if (!this.SelectedDate.HasValue)
                {
                    if (e.AddedItems.Count > 0)
                    {
                        this.SelectedDate = (DateTime?)e.AddedItems[0];
                    }
                }
            }
        }

        private void Calendar_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            SetPopUpPosition();
        }

        private string DateTimeToString(DateTime d)
        {
            DateTimeFormatInfo dtfi = DateTimeHelper.GetCurrentDateFormat();

            switch (this.SelectedDateFormat)
            {
                case DatePickerFormat.Short:
                    {
                        return string.Format(CultureInfo.CurrentCulture, d.ToString(dtfi.ShortDatePattern, dtfi));
                    }
                case DatePickerFormat.Long:
                    {
                        return string.Format(CultureInfo.CurrentCulture, d.ToString(dtfi.LongDatePattern, dtfi));
                    }
            }      
            return null;
        }

        private void DatePicker_LostFocus(object sender, RoutedEventArgs e)
        {
            DatePicker datePicker = sender as DatePicker;
            Debug.Assert(datePicker != null);
            Debug.Assert(e.OriginalSource != null);

            if (datePicker.IsDropDownOpen && datePicker._dropDownButton != null && !(e.OriginalSource.Equals(datePicker._textBox)) && !(e.OriginalSource.Equals(datePicker._dropDownButton)) && !(e.OriginalSource.Equals(datePicker._calendar)))
            {
                datePicker.IsDropDownOpen = false;
            }
            SetSelectedDate();
        }

        private void DatePicker_GotFocus(object sender, RoutedEventArgs e)
        {
            DatePicker datePicker = sender as DatePicker;
            Debug.Assert(datePicker != null);
            if (this.IsEnabled && this._textBox != null)
            {
                this._textBox.Focus();
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

        private void DropDownButton_Click(object sender, RoutedEventArgs e)
        {
            HandlePopUp();
        }

        private void HandlePopUp()
        {
            if (this.IsDropDownOpen)
            {
                this.Focus();
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
            _calendar.DayButtonMouseUp += new MouseButtonEventHandler(Calendar_DayButtonMouseUp);
            _calendar.DisplayDateChanged += new EventHandler<CalendarDateChangedEventArgs>(Calendar_DisplayDateChanged);
            _calendar.SelectedDatesChanged += new EventHandler<SelectionChangedEventArgs>(Calendar_SelectedDatesChanged);
            _calendar.MouseLeftButtonDown += new MouseButtonEventHandler(Calendar_MouseLeftButtonDown);
            _calendar.KeyDown += new KeyEventHandler(Calendar_KeyDown);
            _calendar.SelectionMode = CalendarSelectionMode.SingleDate;
            _calendar.SizeChanged += new SizeChangedEventHandler(Calendar_SizeChanged);
            _calendar.IsTabStop = true;
        }

        /// <summary>
        /// Sets the matrix to its inverse
        /// </summary>
        /// <param name="matrix">Matrix to be inverted.</param>
        /// <returns>
        /// bool - true if the Matrix is invertible, false otherwise.
        /// </returns>
        private static bool InvertMatrix(ref Matrix matrix)
        {
            double determinant = matrix.M11 * matrix.M22 - matrix.M12 * matrix.M21;

            if (determinant == 0.0)
            {
                return false;
            }

            Matrix matCopy = matrix;
            matrix.M11 = matCopy.M22 / determinant;
            matrix.M12 = -1 * matCopy.M12 / determinant;
            matrix.M21 = -1 * matCopy.M21 / determinant;
            matrix.M22 = matCopy.M11 / determinant;
            matrix.OffsetX = (matCopy.OffsetY * matCopy.M21 - matCopy.OffsetX * matCopy.M22) / determinant;
            matrix.OffsetY = (matCopy.OffsetX * matCopy.M12 - matCopy.OffsetY * matCopy.M11) / determinant;

            return true;
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

        private void OnDateSelected(SelectionChangedEventArgs e)
        {
            EventHandler<SelectionChangedEventArgs> handler = this.SelectedDateChanged;
            if (null != handler)
            {
                handler(this, e);
            }
        }

        private void OpenDropDown()
        {
            OpenPopUp();
            this._calendar.Focus();
            this._calendar.ResetStates();
            this.OnCalendarOpened(new RoutedEventArgs());
        }

        private void OpenPopUp()
        {
            FrameworkElement page = (Application.Current != null) ?
                       Application.Current.RootVisual as FrameworkElement :
                       null;

            if (page != null)
            {            
                 this._popUp.IsOpen = true;
            }
        }

        private void OutsidePopupCanvas_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            this.IsDropDownOpen = false;
        }

        

        /// <summary>
        /// Input text is parsed in the correct format and changed into a DateTime object.
        /// If the text can not be parsed TextParseError Event is thrown.
        /// </summary>
        private DateTime? ParseText(string text)
        {
            DateTime newSelectedDate;

            //TryParse is not used in order to be able to pass the exception to the TextParseError event
            try
            {
                newSelectedDate = DateTime.Parse(text, DateTimeHelper.GetCurrentDateFormat());

                if (Calendar.IsValidDateSelection(this._calendar, newSelectedDate))
                {
                    return newSelectedDate;
                }
                else
                {
                    DatePickerDateValidationErrorEventArgs dateValidationError = new DatePickerDateValidationErrorEventArgs(new ArgumentOutOfRangeException("text",Resource.Calendar_OnSelectedDateChanged_InvalidValue), text);
                    OnDateValidationError(dateValidationError);

                    if (dateValidationError.ThrowException)
                    {
                        throw dateValidationError.Exception;
                    }
                }
            }
            catch (FormatException ex)
            {
                DatePickerDateValidationErrorEventArgs textParseError = new DatePickerDateValidationErrorEventArgs(ex, text);
                OnDateValidationError(textParseError);

                if (textParseError.ThrowException)
                {
                    throw textParseError.Exception;
                }
            }
            return null;
        }

        private bool ProcessDatePickerKey(KeyEventArgs e)
        {
            switch (e.Key)
            {
                case Key.Enter:
                    {
                        SetSelectedDate();
                        return true;
                    }
                case Key.Down:
                    {
                        // 
                        if ((Keyboard.Modifiers & ModifierKeys.Control) == ModifierKeys.Control)
                        {
                            HandlePopUp();
                        }
                        return true;
                    }
            }
            return false;
        }

        private void ProcessTextBox()
        {
            SetSelectedDate();
            this.IsDropDownOpen = true;
            _calendar.Focus();
        }

        private void SetPopUpPosition()
        {
            if (this._calendar != null && Application.Current != null && Application.Current.Host != null && Application.Current.Host.Content != null)
            {
                double pageHeight = Application.Current.Host.Content.ActualHeight;
                double pageWidth = Application.Current.Host.Content.ActualWidth;
                double calendarHeight = this._calendar.ActualHeight;
                double dpHeight = this.ActualHeight;
           
                if ( this._root != null)
                {
                    MatrixTransform mt = this._root.TransformToVisual(null) as MatrixTransform;

                    if ( mt != null)
                    {
                        double dpX = mt.Matrix.OffsetX;
                        double dpY = mt.Matrix.OffsetY;

                        double calendarX = dpX;
                        double calendarY = dpY + dpHeight;

                        //if the page height is less then the total height of the PopUp + DatePicker
                        //or if we can fit the PopUp inside the page, we want to place the PopUp to the bottom
                        if ( pageHeight < calendarY + calendarHeight)
                        {
                            calendarY = dpY - calendarHeight;
                        }
                        this._popUp.HorizontalOffset = 0;
                        this._popUp.VerticalOffset = 0;
                        this._outsidePopupCanvas.Width = pageWidth;
                        this._outsidePopupCanvas.Height = pageHeight;
                        this._calendar.HorizontalAlignment = HorizontalAlignment.Left;
                        this._calendar.VerticalAlignment = VerticalAlignment.Top;
                        Canvas.SetLeft(this._calendar, calendarX - dpX);
                        Canvas.SetTop(this._calendar, calendarY - dpY);

                        //transform the invisible canvas to plugin coordinate space origin
                        Matrix transformToRootMatrix = mt.Matrix;
                        if (InvertMatrix(ref transformToRootMatrix))
                        {
                            mt.Matrix = transformToRootMatrix;
                            this._outsidePopupCanvas.RenderTransform = mt;
                        }
                    }
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

                    if (this.SelectedDate != null)
                    {
                        //If the string value of the SelectedDate and the TextBox string value are equal,
                        //we do not parse the string again
                        //if we do an extra parse, we lose data in M/d/yy format
                        // ex: SelectedDate = DateTime(1008,12,19) but when "12/19/08" is parsed it is interpreted as DateTime(2008,12,19)

                        string selectedDate = DateTimeToString(this.SelectedDate.Value);

                        if (selectedDate == s)
                        {
                            return;
                        }
                    }
                    DateTime? d = SetTextBoxValue(s);
                    if (!this.SelectedDate.Equals(d))
                    {
                        this.SelectedDate = d;
                    }
                }
                else
                {
                    if (this.SelectedDate != null)
                    {
                        this.SelectedDate = null;
                    }
                }
            }
            else
            {
                DateTime? d = SetTextBoxValue(_defaultText);
                if (!this.SelectedDate.Equals(d))
                {
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
                DateTime? d = ParseText(s);

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
                        SetValue(TextProperty, newtext);
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
                DateTimeFormatInfo dtfi = DateTimeHelper.GetCurrentDateFormat();
                this.Text = string.Empty;
                this._defaultText = string.Empty;

                switch (this.SelectedDateFormat)
                {
                    case DatePickerFormat.Long:
                        {
                            this._textBox.Watermark = string.Format(CultureInfo.CurrentCulture, Resource.DatePicker_WatermarkText, dtfi.LongDatePattern.ToString());
                            break;
                        }
                    case DatePickerFormat.Short:
                        {
                            this._textBox.Watermark = string.Format(CultureInfo.CurrentCulture, Resource.DatePicker_WatermarkText, dtfi.ShortDatePattern.ToString());
                            break;
                        }
                }
            }
        }

        private void TextBox_GotFocus(object sender, RoutedEventArgs e)
        {
            this.IsDropDownOpen = false;
        }

        private void TextBox_KeyDown(object sender, KeyEventArgs e)
        {
            if (!e.Handled)
            {
                e.Handled = ProcessDatePickerKey(e);
            }
        }

        private void TextBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            this.SetValueNoCallback(DatePicker.TextProperty, this._textBox.Text);
        }

        private void UpdateDisabledVisual()
        {
            if (!IsEnabled)
            {
                VisualStates.GoToState(this, true, VisualStates.StateDisabled, VisualStates.StateNormal);
            }
            else
            {
                VisualStates.GoToState(this, true, VisualStates.StateNormal);
            }

        }

        #endregion Private Methods

    }
}
