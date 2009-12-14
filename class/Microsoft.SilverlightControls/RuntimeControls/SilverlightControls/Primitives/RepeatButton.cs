// (c) Copyright Microsoft Corporation. 
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved. 


using System; 
using System.Windows; 
using System.Diagnostics;
using System.Windows.Controls.Primitives; 
using System.Windows.Input;
using System.Windows.Media.Animation;
using System.Windows.Threading; 
using System.Windows.Automation.Peers;
using System.Windows.Media; 
 
namespace System.Windows.Controls.Primitives
{ 
    /// <summary>
    /// Control that raises its Click event repeatedly from the time it is
    /// pressed until it is released. 
    /// </summary>
    [TemplateVisualState(Name = RepeatButton.StateNormal, GroupName = RepeatButton.GroupCommon)]
    [TemplateVisualState(Name = RepeatButton.StateMouseOver, GroupName = RepeatButton.GroupCommon)] 
    [TemplateVisualState(Name = RepeatButton.StatePressed, GroupName = RepeatButton.GroupCommon)] 
    [TemplateVisualState(Name = RepeatButton.StateDisabled, GroupName = RepeatButton.GroupCommon)]
    [TemplateVisualState(Name = RepeatButton.StateUnfocused, GroupName = RepeatButton.GroupFocus)] 
    [TemplateVisualState(Name = RepeatButton.StateFocused, GroupName = RepeatButton.GroupFocus)]
    public sealed partial class RepeatButton : ButtonBase
    { 
        #region Delay
        /// <summary>
        /// Gets or sets the amount of time, in milliseconds, the RepeatButton 
        /// waits while it is pressed before it starts repeating. The value must 
        /// be non-negative.
        /// </summary> 
        public int Delay
        {
            get { return (int)GetValue(DelayProperty); } 
            set { SetValue(DelayProperty, value); }
        }
 
        /// <summary> 
        /// Identifies the Delay dependency property.
        /// </summary> 
        public static readonly DependencyProperty DelayProperty =
            DependencyProperty.RegisterCore(
                "Delay", 
                typeof(int),
                typeof(RepeatButton),
                new PropertyMetadata(500, OnDelayPropertyChanged)); 
 
        /// <summary>
        /// DelayProperty property changed handler. 
        /// </summary>
        /// <param name="d">RepeatButton that changed its Delay.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param> 
        private static void OnDelayPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            RepeatButton repeatButton = d as RepeatButton; 
            Debug.Assert(repeatButton != null); 

            int delay = (int)e.NewValue; 
            if (delay < 0)
            {
                throw new ArgumentException(); 
            }
        }
        #endregion Delay 
 
        #region Interval
        /// <summary> 
        /// Gets or sets the amount of time, in milliseconds, between repeats
        /// once repeating starts. The value must be non-negative.
        /// </summary> 
        public int Interval
        {
            get { return (int)GetValue(IntervalProperty); } 
            set { SetValue(IntervalProperty, value); } 
        }
 
        /// <summary>
        /// Identifies the Interval dependency property.
        /// </summary> 
        public static readonly DependencyProperty IntervalProperty =
            DependencyProperty.RegisterCore(
                "Interval", 
                typeof(int), 
                typeof(RepeatButton),
                new PropertyMetadata(33, OnIntervalPropertyChanged)); 

        /// <summary>
        /// IntervalProperty property changed handler. 
        /// </summary>
        /// <param name="d">RepeatButton that changed its Interval.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param> 
        private static void OnIntervalPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) 
        {
            RepeatButton repeatButton = d as RepeatButton; 
            Debug.Assert(repeatButton != null);

            int interval = (int)e.NewValue; 
            if (interval <= 0)
            {
                throw new ArgumentException(); 
            } 
        }
        #endregion Interval 

        #region Timer
        /// <summary> 
        /// Timer listening for the button to repeat.
        /// </summary>
        private DispatcherTimer _timer; 
 
        /// <summary>
        /// Start the timer. 
        /// </summary>
        private void StartTimer()
        { 
            if (_timer == null)
            {
                _timer = new DispatcherTimer(); 
                _timer.Tick += new EventHandler(OnTimeout); 
            }
            else if (_timer.IsEnabled) 
            {
                return;
            } 
            _timer.Interval = TimeSpan.FromMilliseconds(Delay);
            _timer.Start();
        } 
 
        /// <summary>
        /// Stop the timer. 
        /// </summary>
        private void StopTimer()
        { 
            if (_timer != null)
            {
                _timer.Stop(); 
            } 
        }
 
        /// <summary>
        /// Handler timer ticks.
        /// </summary> 
        /// <param name="sender">Timer.</param>
        /// <param name="e">EventArgs.</param>
        private void OnTimeout(object sender, EventArgs e) 
        { 
            int interval = Interval;
            if (_timer.Interval.Milliseconds != interval) 
            {
                _timer.Interval = TimeSpan.FromMilliseconds(interval);
            } 

            if (IsPressed)
            { 
                if (_keyboardCausingRepeat) 
                {
                    // if the timeout occurred when the space key was down 
                    // trigger the click directly without running a hit test.
                    OnClick();
                } 
                else
                {
                    // this is a workaround to check the HitTest to see 
                    // whether the mouse is still over the button, since 
                    // Silverlight has a bug where MouseEnter/MouseLeave
                    // states are not detected until the mouse has moved. 
                    foreach (UIElement element in VisualTreeHelper.FindElementsInHostCoordinates(_mousePosition, this))
                    {
                        if (element == _elementRoot) 
                            OnClick();
                    }
                } 
            } 
        }
 
        /// <summary>
        /// Starts or stops the repeating timer based on whether the
        /// mouse or keyboard is being used to cause the repeats. 
        /// </summary>
        private void UpdateRepeatState()
        { 
              if (_mouseCausingRepeat || _keyboardCausingRepeat) 
            {
                StartTimer(); 
            }
            else
            { 
                StopTimer();
            }
        } 
 
        /// <summary>
        /// Called when the IsEnabled property changes. 
        /// </summary>
        /// <param name="e">Property changed args</param>
        private void OnIsEnabledChanged(object sender, DependencyPropertyChangedEventArgs e) 
        {
            // kill the repeat state timer
            _keyboardCausingRepeat = false; 
            _mouseCausingRepeat = false; 
            UpdateRepeatState();
        } 

        /// <summary>
        /// Responds to the LostFocus event. 
        /// </summary>
        /// <param name="e">The event data for the LostFocus event.</param>
        protected override void OnLostFocus(RoutedEventArgs e) 
        { 
            base.OnLostFocus(e);
 
            if (ClickMode != ClickMode.Hover)
            {
                // kill the repeat state timer 
                _keyboardCausingRepeat = false;
                _mouseCausingRepeat = false;
                UpdateRepeatState(); 
            } 
        }
        #endregion Timer 

        #region Template Parts
        /// <summary> 
        /// Root template part of the Button.
        /// </summary>
        /// <remarks>This field is marked internal for unit testing.</remarks> 
        internal FrameworkElement _elementRoot; 
        internal const string ElementRoot = "Root";
        #endregion Template Parts 

        #region Constructor
        /// <summary> 
        /// Initializes a new instance of the RepeatButton class.
        /// </summary>
        public RepeatButton() 
        { 
            DefaultStyleKey = typeof(RepeatButton);
            ClickMode = ClickMode.Press; 
            IsEnabledChanged += OnIsEnabledChanged;
        }
 
        /// <summary>
        /// Apply a template to the RepeatButton.
        /// </summary> 
        public override void OnApplyTemplate() 
        {
            // get the parts 
            _elementRoot = GetTemplateChild(ElementRoot) as FrameworkElement;

            // Sync the logical and visual states of the control 
            UpdateVisualState(false);
        }
 
        #endregion Constructor 

        #region Change State 

        /// <summary>
        /// Change to the correct visual state for the repeatbutton. 
        /// </summary>
        /// <param name="useTransitions">
        /// true to use transitions when updating the visual state, false to 
        /// snap directly to the new visual state. 
        /// </param>
        internal override void ChangeVisualState(bool useTransitions) 
        {
            if (!IsEnabled)
            { 
                GoToState(useTransitions, StateDisabled);
            }
            else if (IsPressed) 
            { 
                GoToState(useTransitions, StatePressed);
            } 
            else if (IsMouseOver)
            {
                GoToState(useTransitions, StateMouseOver); 
            }
            else
            { 
                GoToState(useTransitions, StateNormal); 
            }
 
            if (IsFocused && IsEnabled)
            {
                GoToState(useTransitions, StateFocused); 
            }
            else
            { 
                GoToState(useTransitions, StateUnfocused); 
            }
        } 
        #endregion Change State

        #region Mouse Handlers 
        /// <summary>
        /// Overriding the handler for the MouseLeftButtonDown event.
        /// </summary> 
        /// <param name="e">MouseButtonEventArgs.</param> 
        protected override void OnMouseLeftButtonDown(MouseButtonEventArgs e)
        { 
            if (e == null)
            {
                throw new ArgumentNullException("e"); 
            }
            else if (e.Handled)
            { 
                return; 
            }
 
            base.OnMouseLeftButtonDown(e);

            if (ClickMode != ClickMode.Hover) 
            {
                _mouseCausingRepeat = true;
                UpdateRepeatState(); 
            } 
        }
 
        /// <summary>
        /// Overriding the handler for the MouseLeftButtonUp event.
        /// </summary> 
        /// <param name="e">MouseButtonEventArgs.</param>
        protected override void OnMouseLeftButtonUp(MouseButtonEventArgs e)
        { 
            if (e == null) 
            {
                throw new ArgumentNullException("e"); 
            }
            else if (e.Handled)
            { 
                return;
            }
 
            base.OnMouseLeftButtonUp(e); 

            if (ClickMode != ClickMode.Hover) 
            {
                _mouseCausingRepeat = false;
                UpdateRepeatState(); 
            }
            UpdateVisualState();
        } 
 
        /// <summary>
        /// Overriding the handler for the MouseEnter event. 
        /// </summary>
        /// <param name="e">Event arguments</param>
        protected override void OnMouseEnter(MouseEventArgs e) 
        {
            base.OnMouseEnter(e);
            if (ClickMode == ClickMode.Hover) 
            { 
                _mouseCausingRepeat = true;
                UpdateRepeatState(); 
            }
            UpdateVisualState();
 
            // this code will cache the mouse position relative to the top level UIElement
            // in the Silverlight page.
            object parent = this; 
            while (true) 
            {
                FrameworkElement element = parent as FrameworkElement; 
                if (element == null)
                {
                    break; 
                }
                parent = element.Parent;
            } 
            _mousePosition = e.GetPosition(parent as UIElement); 
        }
 
        /// <summary>
        /// Overriding the handler for the MouseLeave event.
        /// </summary> 
        /// <param name="e">Event arguments</param>
        protected override void OnMouseLeave(MouseEventArgs e)
        { 
            base.OnMouseLeave(e); 

            if (ClickMode == ClickMode.Hover) 
            {
                _mouseCausingRepeat = false;
                UpdateRepeatState(); 
            }

            UpdateVisualState(); 
        } 

        /// <summary> 
        /// Overriding the handler for the MouseMove event.
        /// </summary>
        /// <param name="e"></param> 
        /// <remarks>
        /// The reason I am overriding this function without calling
        /// base.OnMouseMove is that the ButtonBase class sometimes 
        /// sets IsPressed to false based on mouse position. This 
        /// interferes with the RepeatButton functionality.
        /// </remarks> 
        protected override void OnMouseMove(MouseEventArgs e)
        {
            if (e == null) 
            {
                throw new ArgumentNullException("e");
            } 
 
            // this code will cache the mouse position relative to the top level UIElement
            // in the Silverlight page. 
            object parent = this;
            while (true)
            { 
                FrameworkElement element = parent as FrameworkElement;
                if (element == null)
                { 
                    break; 
                }
                parent = element.Parent; 
            }
            _mousePosition = e.GetPosition(parent as UIElement);
        } 

	// Match signature from SL2 final
	protected override void OnClick ()
	{
		base.OnClick ();
	}

        #region Keyboard Handlers
        /// <summary> 
        /// Overriding the OnKeyDown handler to start the repeat timer. 
        /// </summary>
        /// <param name="e">KeyEventArgs</param> 
        protected override void OnKeyDown(KeyEventArgs e)
        {
            if ((e.Key == Key.Space) && (ClickMode != ClickMode.Hover)) 
            {
                _keyboardCausingRepeat = true;
                UpdateRepeatState(); 
            } 

            base.OnKeyDown(e); 
        }

        /// <summary> 
        /// Overriding the OnKeyUp handler to stop the repeat timer.
        /// </summary>
        /// <param name="e">KeyEventArgs</param> 
        protected override void OnKeyUp(KeyEventArgs e) 
        {
            base.OnKeyUp(e); 

            if ((e.Key == Key.Space) && (ClickMode != ClickMode.Hover))
            { 
                _keyboardCausingRepeat = false;
                UpdateRepeatState();
            } 
            UpdateVisualState(); 
        }
        #endregion 

        #endregion Mouse Handlers
 
        #region Private Members
        // True if keyboard was used to initiate the repeat button, false otherwise.
        private bool _keyboardCausingRepeat; 
 
        // True if mouse was used to initiate the repeat button, false otherwise.
        private bool _mouseCausingRepeat; 
        #endregion
    }
} 
