// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System;
using System.Diagnostics; 
using System.Windows.Automation.Peers;
using System.Windows.Controls.Primitives; 
using System.Windows.Input;
using System.Windows.Media.Animation; 
using System.Windows.Threading;
using System.Windows.Controls;

// use the cool stuff from SL Toolkit
using Microsoft.Windows.Controls;

namespace System.Windows.Controls.Primitives
{
    /// <summary>
    /// Control that raises its Click event repeatedly from the time it is 
    /// pressed until it is released. 
    /// </summary>

	[TemplateVisualState (Name = VisualStates.StateNormal, GroupName = VisualStates.GroupCommon)]
	[TemplateVisualState (Name = VisualStates.StateMouseOver, GroupName = VisualStates.GroupCommon)]
	[TemplateVisualState (Name = VisualStates.StatePressed, GroupName = VisualStates.GroupCommon)]
	[TemplateVisualState (Name = VisualStates.StateDisabled, GroupName = VisualStates.GroupCommon)]

	[TemplateVisualState (Name = VisualStates.StateFocused, GroupName = VisualStates.GroupFocus)]
	[TemplateVisualState (Name = VisualStates.StateUnfocused, GroupName = VisualStates.GroupFocus)]

	public sealed class RepeatButton : ButtonBase, IUpdateVisualState {

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
            DependencyProperty.Register( 
                "Delay", 
                typeof(int),
                typeof(RepeatButton), 
                new PropertyMetadata(OnDelayPropertyChanged));

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
                throw new ArgumentException(/*Resource.RepeatButton_DelayPropertyCannotBeNegative*/ "", DelayProperty.ToString()); 
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
            DependencyProperty.Register( 
                "RepeatButton.Interval",
                typeof(int),
                typeof(RepeatButton), 
                new PropertyMetadata(OnIntervalPropertyChanged));

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
                throw new ArgumentException(/*Resource.RepeatButton_IntervalMustBePositive*/ "", IntervalProperty.ToString());
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

// spouliot: I guess workaround is not needed anymore since SL2 final does not provide the HitTest method anymore
#if false
            if (IsPressed) 
            {
                // this is a workaround to check the HitTest to see
                // whether the mouse is still over the button, since 
                // Silverlight has a bug where MouseEnter/MouseLeave
                // states are not detected until the mouse has moved.
                foreach (UIElement element in HitTest(_mousePosition)) 
                { 
                    if (element == ElementRoot)
                        OnClick(); 
                }
            }
#endif
        } 

        #endregion Timer

	#region Visual state management
	/// <summary>
	/// Gets or sets the helper that provides all of the standard
	/// interaction functionality.
	/// </summary>
	private InteractionHelper Interaction { get; set; }

	/// <summary>
	/// Update the visual state of the control.
	/// </summary>
	/// <param name="useTransitions">
	/// A value indicating whether to automatically generate transitions to
	/// the new state, or instantly transition to the new state.
	/// </param>
	void IUpdateVisualState.UpdateVisualState (bool useTransitions)
	{
		UpdateVisualState (useTransitions);
	}
	#endregion

        #region Constructor 
        /// <summary>
        /// Initializes a new instance of the RepeatButton class. 
        /// </summary>
        public RepeatButton ()
        { 
            ClickMode = ClickMode.Press;
            Delay = 500;
            Interval = 33;
	    this.DefaultStyleKey = typeof (RepeatButton);
	    Interaction = new InteractionHelper (this);
	} 

        /// <summary> 
        /// Apply a template to the RepeatButton.
        /// </summary>
        public override void OnApplyTemplate() 
        {
            // get the parts
            ElementRoot = GetTemplateChild(ElementRootName) as FrameworkElement; 
            ElementFocusVisual = GetTemplateChild(ElementFocusVisualName) as FrameworkElement; 

            Interaction.OnApplyTemplateBase ();
        }
 
        #endregion Constructor

        #region Change State 

	/// <summary>
	/// Change to the correct visual state for the repeatbutton. 
	/// </summary> 
	internal void UpdateVisualState (bool useTransitions)
	{
		// all states are managed by the default InteractionHelper
		Interaction.UpdateVisualStateBase (useTransitions);

		if (ElementFocusVisual != null) {
			ElementFocusVisual.Visibility = (IsFocused && IsEnabled) ?
			    Visibility.Visible : Visibility.Collapsed;
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
		if (Interaction.AllowMouseLeftButtonDown (e)) {
			Interaction.OnMouseLeftButtonDownBase ();
			if (ClickMode != ClickMode.Hover) {
				StartTimer ();
			}
		}
        }
 
        /// <summary> 
        /// Overriding the handler for the MouseLeftButtonUp event.
        /// </summary> 
        /// <param name="e">MouseButtonEventArgs.</param>
        protected override void OnMouseLeftButtonUp(MouseButtonEventArgs e)
        {
		if (Interaction.AllowMouseLeftButtonUp (e)) {
			if (ClickMode != ClickMode.Hover) {
				StopTimer ();
			}
			Interaction.OnMouseLeftButtonUpBase ();
		}
        }

        /// <summary> 
        /// Overriding the handler for the MouseEnter event.
        /// </summary>
        /// <param name="e">Event arguments</param> 
        protected override void OnMouseEnter(MouseEventArgs e) 
        {
		if (Interaction.AllowMouseEnter (e)) {
			Interaction.OnMouseEnterBase ();

			if (IsPressed) {
				OnClick ();
				StartTimer ();
			}
		}
/* part of the workaround
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
 */
        } 

        /// <summary>
        /// Overriding the handler for the MouseLeave event. 
        /// </summary> 
        /// <param name="e">Event arguments</param>
        protected override void OnMouseLeave(MouseEventArgs e) 
        {
		if (Interaction.AllowMouseLeave (e)) {
			Interaction.OnMouseLeaveBase ();
			StopTimer();
		}
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
//            e.Handled = true;
/* part of the workaround
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
 */
        } 

	protected override void OnClick ()
	{
		base.OnClick ();
	}

        #endregion Mouse Handlers 

        #region KeyEvents
        /// <summary> 
        /// Responds to the KeyDown event. 
        /// </summary>
        /// <param name="e">The event data for the KeyDown event.</param> 
        protected override void OnKeyDown(KeyEventArgs e)
	{
		if (Interaction.AllowKeyDown (e)) {
		}
	}

        /// <summary> 
        /// Responds to the KeyUp event. 
        /// </summary>
        /// <param name="e">The event data for the KeyUp event.</param> 
        protected override void OnKeyUp(KeyEventArgs e)
	{
		if (Interaction.AllowKeyUp (e)) {
		}
	}

        #endregion KeyEvents

	protected override void OnLostFocus (RoutedEventArgs e)
	{
		if (Interaction.AllowLostFocus (e)) {
			Interaction.OnLostFocusBase ();
		}
	}

	protected override AutomationPeer OnCreateAutomationPeer ()
	{
		throw new NotImplementedException ();
	}

        #region Template Parts
        /// <summary> 
        /// Root of the RepeatButton template.
        /// </summary>
        internal FrameworkElement ElementRoot { get; set; } 
        internal const string ElementRootName = "RootElement"; 

        /// <summary> 
        /// Focus indicator of the RepeatButton template.
        /// </summary>
        internal FrameworkElement ElementFocusVisual { get; set; } 
        internal const string ElementFocusVisualName = "FocusVisualElement";

	#endregion Template Parts 
    }
}
